// This file is generated. Do not edit.

#pragma once

#include <mbgl/style/layer.hpp>
#include <mbgl/style/filter.hpp>
#include <mbgl/style/property_value.hpp>

#include <mbgl/util/color.hpp>

namespace mbgl {
namespace style {

class ExtrusionLayer : public Layer {
public:
    ExtrusionLayer(const std::string& layerID);
    ~ExtrusionLayer() final;

    // Source

    void setSource(const std::string& sourceID, const std::string& sourceLayer);
    const std::string& getSourceID() const;
    const std::string& getSourceLayer() const;

    void setFilter(const Filter&);
    const Filter& getFilter() const;

    // Paint properties

    PropertyValue<bool> getExtrusionAntialias() const;
    void setExtrusionAntialias(PropertyValue<bool>);

    PropertyValue<float> getExtrusionOpacity() const;
    void setExtrusionOpacity(PropertyValue<float>);

    PropertyValue<float> getExtrusionLayerOpacity() const;
    void setExtrusionLayerOpacity(PropertyValue<float>);

    PropertyValue<float> getExtrusionHeight() const;
    void setExtrusionHeight(PropertyValue<float>);

    PropertyValue<float> getExtrusionMinHeight() const;
    void setExtrusionMinHeight(PropertyValue<float>);

    PropertyValue<Color> getExtrusionColor() const;
    void setExtrusionColor(PropertyValue<Color>);

    PropertyValue<Color> getExtrusionShadowColor() const;
    void setExtrusionShadowColor(PropertyValue<Color>);

    PropertyValue<Color> getExtrusionOutlineColor() const;
    void setExtrusionOutlineColor(PropertyValue<Color>);

    PropertyValue<std::array<float, 2>> getExtrusionTranslate() const;
    void setExtrusionTranslate(PropertyValue<std::array<float, 2>>);

    PropertyValue<TranslateAnchorType> getExtrusionTranslateAnchor() const;
    void setExtrusionTranslateAnchor(PropertyValue<TranslateAnchorType>);

    PropertyValue<std::string> getExtrusionPattern() const;
    void setExtrusionPattern(PropertyValue<std::string>);

    // Private implementation

    class Impl;
    Impl* const impl;

    ExtrusionLayer(const Impl&);
    ExtrusionLayer(const ExtrusionLayer&) = delete;
};

template <>
inline bool Layer::is<ExtrusionLayer>() const {
    return type == Type::Extrusion;
}

} // namespace style
} // namespace mbgl
