#pragma once

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/geometry/elements_buffer.hpp>
#include <mbgl/geometry/extrusion_buffer.hpp>
#include <mbgl/util/extrusion_texture.hpp>

#include <vector>
#include <memory>

namespace mbgl {

//    class OutlinePatternShader;
//    class PlainShader;
//    class PatternShader;
//    class OutlineShader;
    class ExtrusionShader;
    class ExtrusionPatternShader;
    class ExtrusionTextureShader;

    class ExtrusionBucket : public Bucket {
    public:
        ExtrusionBucket();
        ~ExtrusionBucket() override;

        void upload(gl::TexturePool&, gl::ObjectStore&) override;
        void render(Painter&, const style::Layer&, const UnwrappedTileID&, const mat4&) override;
        bool hasData() const override;
        bool needsClipping() const override;

        void addGeometry(const GeometryCollection&);

        void drawElements(ExtrusionShader&, gl::ObjectStore&);
        void drawElements(ExtrusionPatternShader&, gl::ObjectStore&);
        void drawElements(ExtrusionTextureShader&, gl::ObjectStore&);
//        void drawVertices(OutlineShader&, gl::ObjectStore&);
//        void drawVertices(OutlinePatternShader&, gl::ObjectStore&);

        ExtrusionTexture texture;

    private:
        ExtrusionVertexBuffer vertexBuffer;
        TriangleElementsBuffer triangleElementsBuffer;
        LineElementsBuffer lineElementsBuffer;

        typedef ElementGroup<3> TriangleGroup;
        typedef ElementGroup<2> LineGroup;

        std::vector<std::unique_ptr<TriangleGroup>> triangleGroups;
        std::vector<std::unique_ptr<LineGroup>> lineGroups;

    };
    
} // namespace mbgl
