// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/extrusion_layer.hpp>
#include <mbgl/style/layers/extrusion_layer_impl.hpp>

namespace mbgl {
namespace style {

ExtrusionLayer::ExtrusionLayer(const std::string& layerID)
    : Layer(Type::Extrusion, std::make_unique<Impl>())
    , impl(static_cast<Impl*>(baseImpl.get())) {
    impl->id = layerID;
}

ExtrusionLayer::ExtrusionLayer(const Impl& other)
    : Layer(Type::Extrusion, std::make_unique<Impl>(other))
    , impl(static_cast<Impl*>(baseImpl.get())) {
}

ExtrusionLayer::~ExtrusionLayer() = default;

std::unique_ptr<Layer> ExtrusionLayer::Impl::clone() const {
    return std::make_unique<ExtrusionLayer>(*this);
}

// Source

void ExtrusionLayer::setSource(const std::string& sourceID, const std::string& sourceLayer) {
    impl->source = sourceID;
    impl->sourceLayer = sourceLayer;
}

const std::string& ExtrusionLayer::getSourceID() const {
    return impl->source;
}

const std::string& ExtrusionLayer::getSourceLayer() const {
    return impl->sourceLayer;
}

// Filter

void ExtrusionLayer::setFilter(const Filter& filter) {
    impl->filter = filter;
}

const Filter& ExtrusionLayer::getFilter() const {
    return impl->filter;
}

// Layout properties


// Paint properties

PropertyValue<bool> ExtrusionLayer::getExtrusionAntialias() const {
    return impl->paint.extrusionAntialias.get();
}

void ExtrusionLayer::setExtrusionAntialias(PropertyValue<bool> value) {
    impl->paint.extrusionAntialias.set(value);
}

PropertyValue<float> ExtrusionLayer::getExtrusionOpacity() const {
    return impl->paint.extrusionOpacity.get();
}

void ExtrusionLayer::setExtrusionOpacity(PropertyValue<float> value) {
    impl->paint.extrusionOpacity.set(value);
}

PropertyValue<float> ExtrusionLayer::getExtrusionLayerOpacity() const {
    return impl->paint.extrusionLayerOpacity.get();
}

void ExtrusionLayer::setExtrusionLayerOpacity(PropertyValue<float> value) {
    impl->paint.extrusionLayerOpacity.set(value);
}

PropertyValue<float> ExtrusionLayer::getExtrusionHeight() const {
    return impl->paint.extrusionHeight.get();
}

void ExtrusionLayer::setExtrusionHeight(PropertyValue<float> value) {
    impl->paint.extrusionHeight.set(value);
}

PropertyValue<float> ExtrusionLayer::getExtrusionMinHeight() const {
    return impl->paint.extrusionMinHeight.get();
}

void ExtrusionLayer::setExtrusionMinHeight(PropertyValue<float> value) {
    impl->paint.extrusionMinHeight.set(value);
}

PropertyValue<Color> ExtrusionLayer::getExtrusionColor() const {
    return impl->paint.extrusionColor.get();
}

void ExtrusionLayer::setExtrusionColor(PropertyValue<Color> value) {
    impl->paint.extrusionColor.set(value);
}

PropertyValue<Color> ExtrusionLayer::getExtrusionShadowColor() const {
    return impl->paint.extrusionShadowColor.get();
}

void ExtrusionLayer::setExtrusionShadowColor(PropertyValue<Color> value) {
    impl->paint.extrusionShadowColor.set(value);
}

PropertyValue<Color> ExtrusionLayer::getExtrusionOutlineColor() const {
    return impl->paint.extrusionOutlineColor.get();
}

void ExtrusionLayer::setExtrusionOutlineColor(PropertyValue<Color> value) {
    impl->paint.extrusionOutlineColor.set(value);
}

PropertyValue<std::array<float, 2>> ExtrusionLayer::getExtrusionTranslate() const {
    return impl->paint.extrusionTranslate.get();
}

void ExtrusionLayer::setExtrusionTranslate(PropertyValue<std::array<float, 2>> value) {
    impl->paint.extrusionTranslate.set(value);
}

PropertyValue<TranslateAnchorType> ExtrusionLayer::getExtrusionTranslateAnchor() const {
    return impl->paint.extrusionTranslateAnchor.get();
}

void ExtrusionLayer::setExtrusionTranslateAnchor(PropertyValue<TranslateAnchorType> value) {
    impl->paint.extrusionTranslateAnchor.set(value);
}

PropertyValue<std::string> ExtrusionLayer::getExtrusionPattern() const {
    return impl->paint.extrusionPattern.get();
}

void ExtrusionLayer::setExtrusionPattern(PropertyValue<std::string> value) {
    impl->paint.extrusionPattern.set(value);
}

} // namespace style
} // namespace mbgl
