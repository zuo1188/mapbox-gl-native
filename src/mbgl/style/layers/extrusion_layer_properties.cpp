// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#include <mbgl/style/layers/extrusion_layer_properties.hpp>

namespace mbgl {
namespace style {

void ExtrusionPaintProperties::parse(const JSValue& value) {
    extrusionAntialias.parse("extrusion-antialias", value);
    extrusionOpacity.parse("extrusion-opacity", value);
    extrusionLayerOpacity.parse("extrusion-layer-opacity", value);
    extrusionHeight.parse("extrusion-height", value);
    extrusionMinHeight.parse("extrusion-min-height", value);
    extrusionColor.parse("extrusion-color", value);
    extrusionShadowColor.parse("extrusion-shadow-color", value);
    extrusionOutlineColor.parse("extrusion-outline-color", value);
    extrusionTranslate.parse("extrusion-translate", value);
    extrusionTranslateAnchor.parse("extrusion-translate-anchor", value);
    extrusionPattern.parse("extrusion-pattern", value);
}

void ExtrusionPaintProperties::cascade(const CascadeParameters& parameters) {
    extrusionAntialias.cascade(parameters);
    extrusionOpacity.cascade(parameters);
    extrusionLayerOpacity.cascade(parameters);
    extrusionHeight.cascade(parameters);
    extrusionMinHeight.cascade(parameters);
    extrusionColor.cascade(parameters);
    extrusionShadowColor.cascade(parameters);
    extrusionOutlineColor.cascade(parameters);
    extrusionTranslate.cascade(parameters);
    extrusionTranslateAnchor.cascade(parameters);
    extrusionPattern.cascade(parameters);
}

bool ExtrusionPaintProperties::recalculate(const CalculationParameters& parameters) {
    bool hasTransitions = false;

    hasTransitions |= extrusionAntialias.calculate(parameters);
    hasTransitions |= extrusionOpacity.calculate(parameters);
    hasTransitions |= extrusionLayerOpacity.calculate(parameters);
    hasTransitions |= extrusionHeight.calculate(parameters);
    hasTransitions |= extrusionMinHeight.calculate(parameters);
    hasTransitions |= extrusionColor.calculate(parameters);
    hasTransitions |= extrusionShadowColor.calculate(parameters);
    hasTransitions |= extrusionOutlineColor.calculate(parameters);
    hasTransitions |= extrusionTranslate.calculate(parameters);
    hasTransitions |= extrusionTranslateAnchor.calculate(parameters);
    hasTransitions |= extrusionPattern.calculate(parameters);

    return hasTransitions;
}

} // namespace style
} // namespace mbgl
