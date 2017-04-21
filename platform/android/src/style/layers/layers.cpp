#include "layers.hpp"

#include <mbgl/style/layer.hpp>
#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layers/raster_layer.hpp>
#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/style/layers/custom_layer.hpp>

#include "background_layer.hpp"
#include "circle_layer.hpp"
#include "custom_layer.hpp"
#include "fill_layer.hpp"
#include "line_layer.hpp"
#include "raster_layer.hpp"
#include "symbol_layer.hpp"
#include "unknown_layer.hpp"

namespace mbgl {
namespace android {

static Layer* initializeLayerPeer(mbgl::style::Style& style, mbgl::style::Layer& coreLayer) {
    if (coreLayer.is<mbgl::style::BackgroundLayer>()) {
        return new BackgroundLayer(style, *coreLayer.as<mbgl::style::BackgroundLayer>());
    } else if (coreLayer.is<mbgl::style::CircleLayer>()) {
        return new CircleLayer(style, *coreLayer.as<mbgl::style::CircleLayer>());
    } else if (coreLayer.is<mbgl::style::FillLayer>()) {
        return new FillLayer(style, *coreLayer.as<mbgl::style::FillLayer>());
    } else if (coreLayer.is<mbgl::style::LineLayer>()) {
        return new LineLayer(style, *coreLayer.as<mbgl::style::LineLayer>());
    } else if (coreLayer.is<mbgl::style::RasterLayer>()) {
        return new RasterLayer(style, *coreLayer.as<mbgl::style::RasterLayer>());
    } else if (coreLayer.is<mbgl::style::SymbolLayer>()) {
        return new SymbolLayer(style, *coreLayer.as<mbgl::style::SymbolLayer>());
    } else if (coreLayer.is<mbgl::style::CustomLayer>()) {
        return new CustomLayer(style, *coreLayer.as<mbgl::style::CustomLayer>());
    } else {
        return new UnknownLayer(style, coreLayer);
    }
}

template <class LayerType, class PeerType>
static PeerType* createPeer(mbgl::style::Style& style, std::unique_ptr<mbgl::style::Layer> layer) {
    return new PeerType(style, std::move(std::unique_ptr<LayerType>(layer.release()->as<LayerType>())));
}

static Layer* initializeLayerPeer(mbgl::style::Style& style, std::unique_ptr<mbgl::style::Layer> coreLayer) {
    if (coreLayer->is<style::BackgroundLayer>()) {
        return createPeer<style::BackgroundLayer, BackgroundLayer>(style, std::move(coreLayer));
    } else if (coreLayer->is<style::CircleLayer>()) {
        return createPeer<style::CircleLayer, CircleLayer>(style, std::move(coreLayer));
    } else if (coreLayer->is<style::FillLayer>()) {
        return createPeer<style::FillLayer, FillLayer>(style, std::move(coreLayer));
    } else if (coreLayer->is<style::LineLayer>()) {
        return createPeer<style::LineLayer, LineLayer>(style, std::move(coreLayer));
    } else if (coreLayer->is<style::RasterLayer>()) {
        return createPeer<style::RasterLayer, RasterLayer>(style, std::move(coreLayer));
    } else if (coreLayer->is<style::SymbolLayer>()) {
        return createPeer<style::SymbolLayer, SymbolLayer>(style, std::move(coreLayer));
    } else if (coreLayer->is<mbgl::style::CustomLayer>()) {
        return createPeer<style::SymbolLayer, SymbolLayer>(style, std::move(coreLayer));
    } else {
        return new UnknownLayer(style, std::move(coreLayer));
    }
}

jni::jobject* createJavaLayerPeer(jni::JNIEnv& env, mbgl::style::Style& style, style::Layer& coreLayer) {
    std::unique_ptr<Layer> peerLayer = std::unique_ptr<Layer>(initializeLayerPeer(style, coreLayer));
    jni::jobject* result = peerLayer->createJavaPeer(env);
    peerLayer.release();
    return result;
}

jni::jobject* createJavaLayerPeer(jni::JNIEnv& env, mbgl::style::Style& style, std::unique_ptr<mbgl::style::Layer> coreLayer) {
    std::unique_ptr<Layer> peerLayer = std::unique_ptr<Layer>(initializeLayerPeer(style, std::move(coreLayer)));
    jni::jobject* result = peerLayer->createJavaPeer(env);
    peerLayer.release();
    return result;
}

void registerNativeLayers(jni::JNIEnv& env) {
    Layer::registerNative(env);
    BackgroundLayer::registerNative(env);
    CircleLayer::registerNative(env);
    CustomLayer::registerNative(env);
    FillLayer::registerNative(env);
    LineLayer::registerNative(env);
    RasterLayer::registerNative(env);
    SymbolLayer::registerNative(env);
    UnknownLayer::registerNative(env);
}

} // namespace android
} // namespace mbgl
