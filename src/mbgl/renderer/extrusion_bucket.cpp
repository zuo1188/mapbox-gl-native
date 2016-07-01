#include <mbgl/renderer/extrusion_bucket.hpp>
#include <mbgl/style/layers/extrusion_layer.hpp>
#include <mbgl/renderer/painter.hpp>
#include <mbgl/shader/extrusion_shader.hpp>
#include <mbgl/shader/extrusion_pattern_shader.hpp>
#include <mbgl/shader/extrusion_texture_shader.hpp>
//#include <mbgl/shader/outline_shader.hpp>
//#include <mbgl/shader/outlinepattern_shader.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/platform/log.hpp>

#include <mapbox/earcut.hpp>

#include <cassert>

namespace mapbox {
    namespace util {
        template <> struct nth<0, mbgl::GeometryCoordinate> {
            static int64_t get(const mbgl::GeometryCoordinate& t) { return t.x; };
        };

        template <> struct nth<1, mbgl::GeometryCoordinate> {
            static int64_t get(const mbgl::GeometryCoordinate& t) { return t.y; };
        };
    } // namespace util
} // namespace mapbox

namespace mbgl {

using namespace style;

struct GeometryTooLongException : std::exception {};

ExtrusionBucket::ExtrusionBucket() {
}

ExtrusionBucket::~ExtrusionBucket() = default;

void ExtrusionBucket::addGeometry(const GeometryCollection& geometry) {     // TODO change this to also take properties from extrusion_layer_impl.cpp so that we can include height properties
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tesselation.
        limitHoles(polygon, 500);

        std::size_t totalVertices = 0;

        for (const auto& ring : polygon) {
            totalVertices += ring.size();
            if (totalVertices > 65535)
                throw GeometryTooLongException();
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);

        std::vector<GLsizei> flatIndices;

        if (triangleGroups.empty() || triangleGroups.back()->vertex_length + totalVertices > 65535) {
            triangleGroups.emplace_back(std::make_unique<TriangleGroup>());
        }

        TriangleGroup& triangleGroup = *triangleGroups.back();

        for (const auto& ring : polygon) {
            std::size_t nVertices = ring.size();

            if (nVertices == 0)
                continue;

            if (lineGroups.empty() || lineGroups.back()->vertex_length + nVertices > 65535)
                lineGroups.emplace_back(std::make_unique<LineGroup>());

            LineGroup& lineGroup = *lineGroups.back();
            GLsizei lineIndex = lineGroup.vertex_length;    // HI fIndex
            flatIndices.emplace_back(lineGroup.vertex_length);
            lineIndex += 1;

            vertexBuffer.add(ring[0].x, ring[0].y, 0, 10, 0, 0, 1, 1, 1, 0);
            lineElementsBuffer.add(lineGroup.vertex_length + nVertices - 1, lineGroup.vertex_length);

            float edgeDistance = 0;
            
            for (uint32_t i = 1; i < nVertices; i++) {
                auto& p1 = ring[i - 1];
                auto& p2 = ring[i];

                flatIndices.emplace_back(lineIndex);

//                    vertexBuffer.add(p2.x, p2.y, 0, /* stand-in height */ 10, 0, 0, 1, 1, 1, 0);
//                    lineElementsBuffer.add(lineIndex - 1, lineIndex);

                // TODO may have to ungroup these??
                Point<double> perp = util::unit(util::perp(convertPoint<double>(p2) - convertPoint<double>(p1)));

                vertexBuffer.add(p1.x, p1.y, 0, 10, perp.x, perp.y, 0, 0, 0, edgeDistance);
                vertexBuffer.add(p1.x, p1.y, 0, 10, perp.x, perp.y, 0, 0, 1, edgeDistance);

                edgeDistance += util::dist<float>(convertPoint<double>(p2), convertPoint<double>(p1));

                vertexBuffer.add(p2.x, p2.y, 0, 10, perp.x, perp.y, 0, 0, 0, edgeDistance);
                vertexBuffer.add(p2.x, p2.y, 0, 10, perp.x, perp.y, 0, 0, 1, edgeDistance);
                // TODO there's extra data stuffed in here, but it will be in more of this format once DDS lands...

                lineElementsBuffer.add(lineIndex + 0, lineIndex + 1);
                lineElementsBuffer.add(lineIndex + 2, lineIndex + 3);
                lineElementsBuffer.add(lineIndex + 0, lineIndex + 2);
                lineElementsBuffer.add(lineIndex + 1, lineIndex + 3);

                triangleElementsBuffer.add(lineIndex + 0, lineIndex + 1, lineIndex + 2);
                triangleElementsBuffer.add(lineIndex + 1, lineIndex + 2, lineIndex + 3);

                lineIndex += 4;
            }

            lineGroup.elements_length += nVertices * 4;
            triangleGroup.elements_length += nVertices * 2;
        }

        std::size_t nIndices = indices.size();
        assert(nIndices % 3 == 0);

//            if (triangleGroups.empty() || triangleGroups.back()->vertex_length + totalVertices > 65535) {
//                triangleGroups.emplace_back(std::make_unique<TriangleGroup>());
//            }
//
//            TriangleGroup& triangleGroup = *triangleGroups.back();
        GLsizei triangleIndex = triangleGroup.vertex_length;

        for (uint32_t i = 0; i < nIndices; i += 3) {
            triangleElementsBuffer.add(triangleIndex + indices[i],
                                       triangleIndex + indices[i + 1],
                                       triangleIndex + indices[i + 2]);
        }

        // TODO this is all definitely wrong ^ re triangles -- may need to separate out the for loops as in gljs

        triangleGroup.vertex_length += totalVertices;
        triangleGroup.elements_length += nIndices / 3;
    }
}

void ExtrusionBucket::upload(gl::TexturePool&, gl::ObjectStore& store) {
    vertexBuffer.upload(store);
    triangleElementsBuffer.upload(store);
    lineElementsBuffer.upload(store);

    // From now on, we're going to render during the opaque and translucent pass.
    uploaded = true;
}

void ExtrusionBucket::render(Painter& painter,
                        const Layer& layer,
                        const UnwrappedTileID& tileID,
                        const mat4& matrix) {
    painter.renderExtrusion(*this, *layer.as<ExtrusionLayer>(), tileID, matrix);
}

bool ExtrusionBucket::hasData() const {
    return !triangleGroups.empty() || !lineGroups.empty();
}

bool ExtrusionBucket::needsClipping() const {
    return true;
}

void ExtrusionBucket::drawElements(ExtrusionShader& shader, gl::ObjectStore& store) {
    GLbyte* vertex_index = BUFFER_OFFSET(0);
    GLbyte* elements_index = BUFFER_OFFSET(0);
    for (auto& group : triangleGroups) {
        assert(group);
        group->array[0].bind(shader, vertexBuffer, triangleElementsBuffer, vertex_index, store);
        MBGL_CHECK_ERROR(glDrawElements(GL_TRIANGLES, group->elements_length * 3, GL_UNSIGNED_SHORT, elements_index));
        vertex_index += group->vertex_length * vertexBuffer.itemSize;
        elements_index += group->elements_length * triangleElementsBuffer.itemSize;
    }
}

void ExtrusionBucket::drawElements(ExtrusionPatternShader& shader, gl::ObjectStore& store) {
    GLbyte* vertex_index = BUFFER_OFFSET(0);
    GLbyte* elements_index = BUFFER_OFFSET(0);
    for (auto& group : triangleGroups) {
        assert(group);
        // TODO this index - ??
        group->array[1].bind(shader, vertexBuffer, triangleElementsBuffer, vertex_index, store);
        MBGL_CHECK_ERROR(glDrawElements(GL_TRIANGLES, group->elements_length * 3, GL_UNSIGNED_SHORT, elements_index));
        vertex_index += group->vertex_length * vertexBuffer.itemSize;
        elements_index += group->elements_length * triangleElementsBuffer.itemSize;
    }



//    for (var i = 0; i < bufferGroups.length; i++) {
//        var group = bufferGroups[i];
//        group.vaos[layer.id].bind(gl, program, group.layout.vertex, group.layout.element);
//        gl.drawElements(gl.TRIANGLES, group.layout.element.length * 3, gl.UNSIGNED_SHORT, 0);
//    }

}

//void ExtrusionBucket::drawElements(ExtrusionTextureShader& shader, gl::ObjectStore& store) {
//    extrusionTextureArray.bind(*shader, tileStencilBuffer, BUFFER_OFFSET(0), store);
//    MBGL_CHECK_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)tileStencilBuffer.index()));
//
//}
//
//void ExtrusionBucket::drawVertices(OutlineShader& shader, gl::ObjectStore& store) {
//    GLbyte* vertex_index = BUFFER_OFFSET(0);
//    GLbyte* elements_index = BUFFER_OFFSET(0);
//    for (auto& group : lineGroups) {
//        assert(group);
//        group->array[0].bind(shader, vertexBuffer, lineElementsBuffer, vertex_index, store);
//        MBGL_CHECK_ERROR(glDrawElements(GL_LINES, group->elements_length * 2, GL_UNSIGNED_SHORT, elements_index));
//        vertex_index += group->vertex_length * vertexBuffer.itemSize;
//        elements_index += group->elements_length * lineElementsBuffer.itemSize;
//    }
//}
//
//void FillBucket::drawVertices(OutlinePatternShader& shader, gl::ObjectStore& store) {
//    GLbyte* vertex_index = BUFFER_OFFSET(0);
//    GLbyte* elements_index = BUFFER_OFFSET(0);
//    for (auto& group : lineGroups) {
//        assert(group);
//        group->array[1].bind(shader, vertexBuffer, lineElementsBuffer, vertex_index, store);
//        MBGL_CHECK_ERROR(glDrawElements(GL_LINES, group->elements_length * 2, GL_UNSIGNED_SHORT, elements_index));
//        vertex_index += group->vertex_length * vertexBuffer.itemSize;
//        elements_index += group->elements_length * lineElementsBuffer.itemSize;
//    }
//}
//
} // namespace mbgl
