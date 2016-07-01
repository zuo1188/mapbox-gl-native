#include <mbgl/platform/platform.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/gl/object_store.hpp>
#include <mbgl/platform/log.hpp>
#include <mbgl/util/extrusion_texture.hpp>

#include <cassert>
#include <cstring>

using namespace mbgl;

void ExtrusionTexture::bind(gl::TexturePool& texturePool, gl::ObjectStore& store) {
    if (!width || !height) {
        Log::Error(Event::OpenGL, "trying to bind texture without dimension");
        return;
    }

    if (!texture) {
        upload(texturePool, store);
    } else if (texture) {
        MBGL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, *texture));
    }

    if (!fbo) {
        bindFramebuffer(store);
    } else if (fbo) {
        MBGL_CHECK_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, *fbo));
        MBGL_CHECK_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0));
    }
}

void ExtrusionTexture::upload(gl::TexturePool& texturePool, gl::ObjectStore& store) {
    if (!texture) {
        texture = texturePool.acquireTexture(store);
        MBGL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, *texture));
#ifndef GL_ES_VERSION_2_0
        MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));
#endif
        MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        MBGL_CHECK_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));  // TODO does 0 at the end here work in place of null or img data? or `void`?
    }
}

void ExtrusionTexture::bindFramebuffer(gl::ObjectStore& store) {
    if (!fbo) {
        fbo = store.createFramebuffer();
        stencil = store.createRenderbuffer();
        depthRbo = store.createRenderbuffer();
        MBGL_CHECK_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, *stencil));
        MBGL_CHECK_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, *depthRbo));
        MBGL_CHECK_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, width, height));
        MBGL_CHECK_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));
        MBGL_CHECK_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, *fbo));
        MBGL_CHECK_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *stencil));
        MBGL_CHECK_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *depthRbo));
        MBGL_CHECK_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0));

//        fbo = gl::createFramebuffer();
//        stencil = gl::createRenderbuffer();
//        depthRbo = gl::createRenderbuffer();
//        MBGL_CHECK_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, stencil));
//        MBGL_CHECK_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer));
//        MBGL_CHECK_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, width, height));
//        MBGL_CHECK_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));
//        MBGL_CHECK_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
//        MBGL_CHECK_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, stencil));
//        MBGL_CHECK_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo));
//        MBGL_CHECK_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0));
    }
}

void ExtrusionTexture::unbindFramebuffer() {
    MBGL_CHECK_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0)); // TODO is 0 right?
    // TODO delete fbo and rbos??????
}