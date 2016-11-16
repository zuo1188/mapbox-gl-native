#include "native_map_view.hpp"
#include "jni.hpp"

#include <cstdlib>
#include <ctime>
#include <cassert>
#include <memory>
#include <list>
#include <tuple>

#include <sys/system_properties.h>

#include <mbgl/platform/platform.hpp>
#include <mbgl/platform/event.hpp>
#include <mbgl/platform/log.hpp>
#include <mbgl/gl/extension.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/image.hpp>

namespace mbgl {
namespace android {

void log_egl_string(EGLDisplay display, EGLint name, const char *label) {
    const char *str = eglQueryString(display, name);
    if (str == nullptr) {
        mbgl::Log::Error(mbgl::Event::OpenGL, "eglQueryString(%d) returned error %d", name,
                         eglGetError());
        throw std::runtime_error("eglQueryString() failed");
    } else {
        char buf[513];
        for (int len = std::strlen(str), pos = 0; len > 0; len -= 512, pos += 512) {
            strncpy(buf, str + pos, 512);
            buf[512] = 0;
            mbgl::Log::Info(mbgl::Event::OpenGL, "EGL %s: %s", label, buf);
        }
    }
}

NativeMapView::NativeMapView(JNIEnv *env_, jobject obj_, float pixelRatio, int availableProcessors_, size_t totalMemory_)
    : env(env_),
      availableProcessors(availableProcessors_),
      totalMemory(totalMemory_),
      threadPool(4) {
    mbgl::Log::Debug(mbgl::Event::Android, "NativeMapView::NativeMapView");

    assert(env_ != nullptr);
    assert(obj_ != nullptr);

    if (env->GetJavaVM(&vm) < 0) {
        env->ExceptionDescribe();
        return;
    }

    obj = env->NewWeakGlobalRef(obj_);
    if (obj == nullptr) {
        env->ExceptionDescribe();
        return;
    }

    fileSource = std::make_unique<mbgl::DefaultFileSource>(
        mbgl::android::cachePath + "/mbgl-offline.db",
        mbgl::android::apkPath);

    map = std::make_unique<mbgl::Map>(
        *this, mbgl::Size{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) },
        pixelRatio, *fileSource, threadPool, MapMode::Continuous);

    float zoomFactor   = map->getMaxZoom() - map->getMinZoom() + 1;
    float cpuFactor    = availableProcessors;
    float memoryFactor = static_cast<float>(totalMemory) / 1000.0f / 1000.0f / 1000.0f;
    float sizeFactor   = (static_cast<float>(map->getSize().width)  / mbgl::util::tileSize) *
                         (static_cast<float>(map->getSize().height) / mbgl::util::tileSize);

    size_t cacheSize = zoomFactor * cpuFactor * memoryFactor * sizeFactor * 0.5f;

    map->setSourceTileCacheSize(cacheSize);
}

NativeMapView::~NativeMapView() {
    mbgl::Log::Debug(mbgl::Event::Android, "NativeMapView::~NativeMapView");

    assert(vm != nullptr);
    assert(obj != nullptr);

    map.reset();
    fileSource.reset();

    env->DeleteWeakGlobalRef(obj);

    obj = nullptr;
    env = nullptr;
    vm = nullptr;
}

mbgl::Size NativeMapView::getFramebufferSize() const {
    return { static_cast<uint32_t>(fbWidth), static_cast<uint32_t>(fbHeight) };
}

void NativeMapView::bind() {
    getContext().bindFramebuffer = 0;
    getContext().viewport = { 0, 0, getFramebufferSize() };
}

void NativeMapView::activate() {
    return;
}

void NativeMapView::deactivate() {
    return;
}

void NativeMapView::invalidate() {
    assert(vm != nullptr);
    assert(obj != nullptr);

    //env->CallVoidMethod(obj, onInvalidateId);
    //if (env->ExceptionCheck()) {
    //    env->ExceptionDescribe();
    //}
}

void NativeMapView::render() {
    if (framebufferSizeChanged) {
        getContext().viewport = { 0, 0, getFramebufferSize() };
        framebufferSizeChanged = false;
    }

    map->render(*this);
}

mbgl::Map &NativeMapView::getMap() { return *map; }

mbgl::DefaultFileSource &NativeMapView::getFileSource() { return *fileSource; }

void NativeMapView::initializeDisplay() {
    mbgl::Log::Debug(mbgl::Event::Android, "NativeMapView::initializeDisplay");
}

void NativeMapView::terminateDisplay() {
    mbgl::Log::Debug(mbgl::Event::Android, "NativeMapView::terminateDisplay");
}

void NativeMapView::initializeContext() {
    mbgl::Log::Debug(mbgl::Event::Android, "NativeMapView::initializeContext");
}

void NativeMapView::terminateContext() {
    mbgl::Log::Debug(mbgl::Event::Android, "NativeMapView::terminateContext");
}

void NativeMapView::createSurface(ANativeWindow *window_) {
    mbgl::Log::Debug(mbgl::Event::Android, "NativeMapView::createSurface");

    assert(window == nullptr);
    assert(window_ != nullptr);
    window = window_;
}

void NativeMapView::destroySurface() {
    mbgl::Log::Debug(mbgl::Event::Android, "NativeMapView::destroySurface");

    if (window != nullptr) {
        ANativeWindow_release(window);
        window = nullptr;
    }
}

void NativeMapView::scheduleTakeSnapshot() {
    snapshot = true;
}

// Speed
/*
typedef enum {
    Format16Bit = 0,
    Format32BitNoAlpha = 1,
    Format32BitAlpha = 2,
    Format24Bit = 3,
    Unknown = 4
} BufferFormat;

typedef enum {
    Format16Depth8Stencil = 0,
    Format24Depth8Stencil = 1,
} DepthStencilFormat;
*/

// Quality
typedef enum {
    Format16Bit = 3,
    Format32BitNoAlpha = 1,
    Format32BitAlpha = 2,
    Format24Bit = 0,
    Unknown = 4
} BufferFormat;

typedef enum {
    Format16Depth8Stencil = 1,
    Format24Depth8Stencil = 0,
} DepthStencilFormat;

// Tuple is <buffer_format, depth_stencil_format, is_not_conformant, is_caveat, config_num,
// config_id>
typedef std::tuple<BufferFormat, DepthStencilFormat, bool, bool, int, EGLConfig> ConfigProperties;

EGLConfig NativeMapView::chooseConfig(const EGLConfig configs[], EGLint numConfigs) {
    mbgl::Log::Info(mbgl::Event::OpenGL, "Found %d configs", numConfigs);

    // Create a list of configs that pass our filters
    std::list<ConfigProperties> configList;
    for (int i = 0; i < numConfigs; i++) {
        mbgl::Log::Info(mbgl::Event::OpenGL, "Config %d:", i);

        EGLint caveat, conformant, bits, red, green, blue, alpha, alphaMask, depth, stencil,
            sampleBuffers, samples;

        mbgl::Log::Info(mbgl::Event::OpenGL, "...Caveat: %d", caveat);
        mbgl::Log::Info(mbgl::Event::OpenGL, "...Conformant: %d", conformant);
        mbgl::Log::Info(mbgl::Event::OpenGL, "...Color: %d", bits);
        mbgl::Log::Info(mbgl::Event::OpenGL, "...Red: %d", red);
        mbgl::Log::Info(mbgl::Event::OpenGL, "...Green: %d", green);
        mbgl::Log::Info(mbgl::Event::OpenGL, "...Blue: %d", blue);
        mbgl::Log::Info(mbgl::Event::OpenGL, "...Alpha: %d", alpha);
        mbgl::Log::Info(mbgl::Event::OpenGL, "...Alpha mask: %d", alphaMask);
        mbgl::Log::Info(mbgl::Event::OpenGL, "...Depth: %d", depth);
        mbgl::Log::Info(mbgl::Event::OpenGL, "...Stencil: %d", stencil);
        mbgl::Log::Info(mbgl::Event::OpenGL, "...Sample buffers: %d", sampleBuffers);
        mbgl::Log::Info(mbgl::Event::OpenGL, "...Samples: %d", samples);

        bool configOk = true;
        configOk &= (depth == 24) || (depth == 16);
        configOk &= stencil == 8;
        configOk &= sampleBuffers == 0;
        configOk &= samples == 0;

        // Filter our configs first for depth, stencil and anti-aliasing
        if (configOk) {
            // Work out the config's buffer format
            BufferFormat bufferFormat;
            if ((bits == 16) && (red == 5) && (green == 6) && (blue == 5) && (alpha == 0)) {
                bufferFormat = Format16Bit;
            } else if ((bits == 32) && (red == 8) && (green == 8) && (blue == 8) && (alpha == 0)) {
                bufferFormat = Format32BitNoAlpha;
            } else if ((bits == 32) && (red == 8) && (green == 8) && (blue == 8) && (alpha == 8)) {
                bufferFormat = Format32BitAlpha;
            } else if ((bits == 24) && (red == 8) && (green == 8) && (blue == 8) && (alpha == 0)) {
                bufferFormat = Format24Bit;
            } else {
                bufferFormat = Unknown;
            }

            // Work out the config's depth stencil format
            DepthStencilFormat depthStencilFormat;
            if ((depth == 16) && (stencil == 8)) {
                depthStencilFormat = Format16Depth8Stencil;
            } else {
                depthStencilFormat = Format24Depth8Stencil;
            }

            bool isNotConformant = (conformant & EGL_OPENGL_ES2_BIT) != EGL_OPENGL_ES2_BIT;
            bool isCaveat = caveat != EGL_NONE;
            EGLConfig configId = configs[i];

            // Ignore formats we don't recognise
            if (bufferFormat != Unknown) {
                configList.push_back(std::make_tuple(bufferFormat, depthStencilFormat,
                                                     isNotConformant, isCaveat, i, configId));
            }
        }
    }

    if (configList.empty()) {
        mbgl::Log::Error(mbgl::Event::OpenGL, "Config list was empty.");
    }

    // Sort the configs to find the best one
    configList.sort();
    bool isConformant = !std::get<2>(configList.front());
    bool isCaveat = std::get<3>(configList.front());
    int configNum = std::get<4>(configList.front());
    EGLConfig configId = std::get<5>(configList.front());

    mbgl::Log::Info(mbgl::Event::OpenGL, "Chosen config is %d", configNum);

    if (isCaveat) {
        mbgl::Log::Warning(mbgl::Event::OpenGL, "Chosen config has a caveat.");
    }
    if (!isConformant) {
        mbgl::Log::Warning(mbgl::Event::OpenGL, "Chosen config is not conformant.");
    }

    return configId;
}

void NativeMapView::notifyMapChange(__attribute__((unused)) mbgl::MapChange change) {
    //assert(change != 99);
    //assert(obj != nullptr);

    //env->CallVoidMethod(obj, onMapChangedId, change);
    //if (env->ExceptionCheck()) {
    //    env->ExceptionDescribe();
    //}
}

void NativeMapView::enableFps(bool enable) {
    mbgl::Log::Debug(mbgl::Event::Android, "NativeMapView::enableFps()");

    fpsEnabled = enable;
}

void NativeMapView::updateFps() {
    if (!fpsEnabled) {
        return;
    }

    static int frames = 0;
    static int64_t timeElapsed = 0LL;

    frames++;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    int64_t currentTime = now.tv_sec * 1000000000LL + now.tv_nsec;

    if (currentTime - timeElapsed >= 1) {
        fps = frames / ((currentTime - timeElapsed) / 1E9);
        mbgl::Log::Debug(mbgl::Event::Render, "FPS: %4.2f", fps);
        timeElapsed = currentTime;
        frames = 0;
    }

    assert(vm != nullptr);
    assert(obj != nullptr);

    env->CallVoidMethod(obj, onFpsChangedId, fps);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
    }
}

void NativeMapView::resizeView(int w, int h) {
    width = w;
    height = h;
    map->setSize({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
}

void NativeMapView::resizeFramebuffer(int w, int h) {
    fbWidth = w;
    fbHeight = h;
    framebufferSizeChanged = true;
    invalidate();
}

void NativeMapView::setInsets(mbgl::EdgeInsets insets_) {
    insets = insets_;
}

}
}
