// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

#pragma once

#include <mbgl/style/layout_property.hpp>
#include <mbgl/style/paint_property.hpp>
#include <mbgl/util/rapidjson.hpp>

namespace mbgl {
namespace style {

class CascadeParameters;
class CalculationParameters;

class ExtrusionPaintProperties {
public:
    void parse(const JSValue&);
    void cascade(const CascadeParameters&);
    bool recalculate(const CalculationParameters&);

    PaintProperty<bool> extrusionAntialias { true };
    PaintProperty<float> extrusionOpacity { 1 };
    PaintProperty<float> extrusionLayerOpacity { 1 };
    PaintProperty<float> extrusionHeight { 3 };
    PaintProperty<float> extrusionMinHeight { 0 };
    PaintProperty<Color> extrusionColor { { 0, 0, 0, 1 } };
    PaintProperty<Color> extrusionShadowColor { { 0, 0, 255, 1 } };
    PaintProperty<Color> extrusionOutlineColor { { 0, 0, 0, -1 } };
    PaintProperty<std::array<float, 2>> extrusionTranslate { {{ 0, 0 }} };
    PaintProperty<TranslateAnchorType> extrusionTranslateAnchor { TranslateAnchorType::Map };
    PaintProperty<std::string, CrossFadedPropertyEvaluator> extrusionPattern { "" };
};

} // namespace style
} // namespace mbgl
