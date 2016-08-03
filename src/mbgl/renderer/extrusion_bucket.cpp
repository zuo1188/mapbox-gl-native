#include <mbgl/renderer/extrusion_bucket.hpp>
#include <mbgl/style/layers/extrusion_layer.hpp>
#include <mbgl/renderer/painter.hpp>
#include <mbgl/shader/extrusion_shader.hpp>
//#include <mbgl/shader/pattern_shader.hpp>
//#include <mbgl/shader/outline_shader.hpp>
//#include <mbgl/shader/outlinepattern_shader.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/platform/log.hpp>

#include <mapbox/earcut.hpp>

#include <algorithm>
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

void ExtrusionBucket::addGeometry(const GeometryCollection& geometry
//                                      , const GeometryTileFeature& feature
                                  ) {
    for (auto& polygon : classifyRings(geometry)) {
        // Optimize polygons with many interior rings for earcut tesselation.
        limitHoles(polygon, 500);

        std::size_t totalVertices = 0;

        for (const auto& ring : polygon) {
            totalVertices += ring.size();
            if (totalVertices > 65535)
                throw GeometryTooLongException();
        }

        std::vector<GLsizei> flatIndices;
        flatIndices.reserve(totalVertices);

        if (triangleGroups.empty() || triangleGroups.back()->vertex_length + (5 * (totalVertices - 1) + 1)  > 65535) {
            triangleGroups.emplace_back(std::make_unique<TriangleGroup>());
        }

        TriangleGroup& triangleGroup = *triangleGroups.back();
        GLsizei triangleIndex = triangleGroup.vertex_length;

        assert(triangleIndex + (5 * (totalVertices - 1) + 1) <= 65535);

        for (const auto& ring : polygon) {
            // Iterate over all the rings in this polygon -- usually this will be 1

            std::size_t nVertices = ring.size();

            if (nVertices == 0)
                continue;

            if (lineGroups.empty() || lineGroups.back()->vertex_length + (5 * (totalVertices - 1) + 1) > 65535)
                lineGroups.emplace_back(std::make_unique<LineGroup>());

            LineGroup& lineGroup = *lineGroups.back();
            GLsizei lineIndex = lineGroup.vertex_length;

//                const int STANDIN_HEIGHT = rand() % 40 + 20;  // TODO TODO
            const int STANDIN_HEIGHT = 60;
//                const auto STANDIN_HEIGHT = PropertyValue<T>::visit(feature.getProperties().find("height")->second);

            // Add vertex for the first upper corner:
            vertexBuffer.add(ring[0].x, ring[0].y, 0, STANDIN_HEIGHT, 0, 0, 1, 1, 0);
            flatIndices.emplace_back(triangleIndex);
            triangleIndex++;

            // Also add what would be the closing line segment:
            lineElementsBuffer.add(lineIndex + nVertices - 1, lineIndex);

            float edgeDistance = 0;

            for (uint32_t i = 1; i < nVertices; i++) {
                const auto& p1 = ring[i - 1];
                const auto& p2 = ring[i];

                // Add vertex for the i'th upper corner: this is p2; p1 was already added
                vertexBuffer.add(p2.x, p2.y, 0, STANDIN_HEIGHT, 0, 0, 1, 1, 0);
//                    if (i < nVertices - 1)
                    flatIndices.emplace_back(triangleIndex);
                // Add line segment from the previous corner to this one
                lineElementsBuffer.add(lineIndex + i - 1, lineIndex + i);

                const auto s1 = convertPoint<double>(p1);
                const auto s2 = convertPoint<double>(p2);

                const Point<double> perp = util::unit(util::perp(s2 - s1));

                // Add four more vertices: this corner, prev corner, bottom two corners (duplicating corners because of the normal vertex calculations)

                vertexBuffer.add(p1.x, p1.y, 0, STANDIN_HEIGHT, perp.x, perp.y, 0, 0, edgeDistance);
                vertexBuffer.add(p1.x, p1.y, 0, STANDIN_HEIGHT, perp.x, perp.y, 0, 1, edgeDistance);

                edgeDistance += util::dist<float>(s2, s1);

                vertexBuffer.add(p2.x, p2.y, 0, STANDIN_HEIGHT, perp.x, perp.y, 0, 0, edgeDistance);
                vertexBuffer.add(p2.x, p2.y, 0, STANDIN_HEIGHT, perp.x, perp.y, 0, 1, edgeDistance);

                triangleElementsBuffer.add(triangleIndex + 1, triangleIndex + 2, triangleIndex + 3);
                triangleElementsBuffer.add(triangleIndex + 2, triangleIndex + 3, triangleIndex + 4);
                triangleIndex += 5;
                triangleGroup.elements_length += 2;
            }

            lineGroup.vertex_length += 5 * nVertices + 1;
            lineGroup.elements_length += 5 * nVertices + 1;
        }

        std::vector<uint32_t> indices = mapbox::earcut(polygon);

        std::size_t nIndices = indices.size();
        assert(nIndices % 3 == 0);
//            assert(std::none_of(indices.begin(), indices.end(), [&](uint32_t i){ return i == totalVertices - 1; }));

        for (uint32_t i = 0; i < nIndices; i += 3) {
            triangleElementsBuffer.add(flatIndices[indices[i]],
                                       flatIndices[indices[i + 1]],
                                       flatIndices[indices[i + 2]]);
        }

        triangleGroup.vertex_length = triangleIndex;
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
    return false;
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
} // namespace mbgl










//void ExtrusionBucket::drawElements(ExtrusionShader& shader, gl::ObjectStore& store) {
//    GLbyte* vertex_index = BUFFER_OFFSET(0);
//    GLbyte* elements_index = BUFFER_OFFSET(0);
//    for (auto& group : triangleGroups) {
//        assert(group);
//        group->array[0].bind(shader, vertexBuffer, triangleElementsBuffer, vertex_index, store);
//        MBGL_CHECK_ERROR(glDrawElements(GL_TRIANGLES, group->elements_length * 3, GL_UNSIGNED_SHORT, elements_index));
//        vertex_index += group->vertex_length * vertexBuffer.itemSize;
//        elements_index += group->elements_length * triangleElementsBuffer.itemSize;
//    }
//}
//
//void ExtrusionBucket::drawElements(ExtrusionPatternShader& shader, gl::ObjectStore& store) {
//    GLbyte* vertex_index = BUFFER_OFFSET(0);
//    GLbyte* elements_index = BUFFER_OFFSET(0);
//    for (auto& group : triangleGroups) {
//        assert(group);
//        // TODO this index - ??
//        group->array[1].bind(shader, vertexBuffer, triangleElementsBuffer, vertex_index, store);
//        MBGL_CHECK_ERROR(glDrawElements(GL_TRIANGLES, group->elements_length * 3, GL_UNSIGNED_SHORT, elements_index));
//        vertex_index += group->vertex_length * vertexBuffer.itemSize;
//        elements_index += group->elements_length * triangleElementsBuffer.itemSize;
//    }
//
//
//
////    for (var i = 0; i < bufferGroups.length; i++) {
////        var group = bufferGroups[i];
////        group.vaos[layer.id].bind(gl, program, group.layout.vertex, group.layout.element);
////        gl.drawElements(gl.TRIANGLES, group.layout.element.length * 3, gl.UNSIGNED_SHORT, 0);
////    }
//
//}
//
////void ExtrusionBucket::drawElements(ExtrusionTextureShader& shader, gl::ObjectStore& store) {
////    extrusionTextureArray.bind(*shader, tileStencilBuffer, BUFFER_OFFSET(0), store);
////    MBGL_CHECK_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)tileStencilBuffer.index()));
////
////}
////
////void ExtrusionBucket::drawVertices(OutlineShader& shader, gl::ObjectStore& store) {
////    GLbyte* vertex_index = BUFFER_OFFSET(0);
////    GLbyte* elements_index = BUFFER_OFFSET(0);
////    for (auto& group : lineGroups) {
////        assert(group);
////        group->array[0].bind(shader, vertexBuffer, lineElementsBuffer, vertex_index, store);
////        MBGL_CHECK_ERROR(glDrawElements(GL_LINES, group->elements_length * 2, GL_UNSIGNED_SHORT, elements_index));
////        vertex_index += group->vertex_length * vertexBuffer.itemSize;
////        elements_index += group->elements_length * lineElementsBuffer.itemSize;
////    }
////}
////
////void FillBucket::drawVertices(OutlinePatternShader& shader, gl::ObjectStore& store) {
////    GLbyte* vertex_index = BUFFER_OFFSET(0);
////    GLbyte* elements_index = BUFFER_OFFSET(0);
////    for (auto& group : lineGroups) {
////        assert(group);
////        group->array[1].bind(shader, vertexBuffer, lineElementsBuffer, vertex_index, store);
////        MBGL_CHECK_ERROR(glDrawElements(GL_LINES, group->elements_length * 2, GL_UNSIGNED_SHORT, elements_index));
////        vertex_index += group->vertex_length * vertexBuffer.itemSize;
////        elements_index += group->elements_length * lineElementsBuffer.itemSize;
////    }
////}
////
//} // namespace mbgl
