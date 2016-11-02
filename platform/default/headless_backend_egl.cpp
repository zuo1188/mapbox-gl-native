#include <mbgl/platform/default/headless_backend.hpp>
#include <mbgl/platform/default/headless_display.hpp>

#include <mbgl/platform/log.hpp>

#include <cassert>

#include <GL/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gbm.h>

namespace mbgl {

gl::glProc HeadlessBackend::initializeExtension(const char* name) {
    return eglGetProcAddress(name);
}

void HeadlessBackend::createContext() {
    auto dpy = display->dpy;
    auto config = display->config;

    assert(dpy != EGL_NO_DISPLAY);
    assert(glContext == EGL_NO_CONTEXT);
    assert(config != nullptr);

    eglBindAPI(EGL_OPENGL_API);

    glContext = eglCreateContext(dpy, config, EGL_NO_CONTEXT, NULL);
    if (glContext == EGL_NO_CONTEXT) {
        mbgl::Log::Error(mbgl::Event::OpenGL, "eglCreateContext() returned error 0x%04x",
                         eglGetError());
        throw std::runtime_error("Error creating GL context object");
    }
}

void HeadlessBackend::destroyContext() {
    if (glContext) {
        auto dpy = display->dpy;
        if (!eglDestroyContext(dpy, glContext)) {
            throw std::runtime_error("Failed to destroy context.");
        }

        glContext = nullptr;
    }
}

void HeadlessBackend::activateContext() {
    auto dpy = display->dpy;
    if (!eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, glContext)) {
        throw std::runtime_error("Switching OpenGL context failed.\n");
    }
}

void HeadlessBackend::deactivateContext() {
    auto dpy = display->dpy;
    if (!eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        throw std::runtime_error("Removing OpenGL context failed.\n");
    }
}

} // namespace mbgl
