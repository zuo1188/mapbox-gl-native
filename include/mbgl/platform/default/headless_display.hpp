#pragma once

#include <mbgl/gl/implementation.hpp>

#if MBGL_USE_CGL
#include <OpenGL/OpenGL.h>
#endif

#if MBGL_USE_EGL
#include <EGL/egl.h>
#endif

namespace mbgl {

class HeadlessDisplay {
public:
    HeadlessDisplay();
    ~HeadlessDisplay();

#if MBGL_USE_CGL
    CGLPixelFormatObj pixelFormat = nullptr;
#endif

#if MBGL_USE_EGL
    EGLDisplay dpy = 0;
    EGLConfig config = 0;
#endif
};

} // namespace mbgl
