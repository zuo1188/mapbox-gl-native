#pragma once

#include <mbgl/gl/gl.hpp>
#include <mbgl/gl/texture_pool.hpp>
#include <mbgl/util/ptr.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/optional.hpp>
#include <mbgl/gl/object_store.hpp>

#include <mutex>

namespace mbgl {

    class ExtrusionTexture : public std::enable_shared_from_this<ExtrusionTexture> {

    public:
        // bind current texture
        void bind(
//                  gl::TexturePool&,
                  gl::ObjectStore&);

        void load(uint16_t _width, uint16_t _height);

        // uploads the texture if it hasn't been uploaded yet.
//        void upload(
////                    gl::TexturePool&,
//                    gl::ObjectStore&);

        // creates and binds framebuffer and renderbuffer
        void bindFramebuffer(gl::ObjectStore&);
        void unbindFramebuffer();

    public:
        // loaded image dimensions
        GLsizei width = 0;
        GLsizei height = 0;

        // GL buffer object handle.
        mbgl::optional<gl::UniqueTexture> texture;
//        mbgl::optional<gl::PooledTexture> texture;

        // GL framebuffer + renderbuffer object handles
        mbgl::optional<gl::UniqueFramebuffer> fbo;
        mbgl::optional<gl::UniqueRenderbuffer> stencil;
        mbgl::optional<gl::UniqueRenderbuffer> depthRbo;

        // texture opacity
        double opacity = 0;

    private:
        mutable std::mutex mtx;

//        // min/mag filter
//        GLint filter = 0;
    };
    
} // namespace mbgl
