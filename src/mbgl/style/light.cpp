// This file is generated. Do not edit.

#include <mbgl/style/light.hpp>
#include <mbgl/style/light_impl.hpp>
#include <mbgl/style/light_observer.hpp>
#include <mbgl/style/light_properties.hpp>

namespace mbgl {
namespace style {

static LightObserver nullObserver;

Light::Light()
    : impl(makeMutable<Impl>()) {
}

Light::~Light() = default;

void Light::setObserver(LightObserver* observer_) {
    observer = observer_ ? observer_ : &nullObserver;
}

Mutable<Light::Impl> Light::mutableImpl() const {
    return makeMutable<Impl>(*impl);
}

LightAnchorType Light::getDefaultAnchor() {
    return LightAnchor::defaultValue();
}

PropertyValue<LightAnchorType> Light::getAnchor() const {
    return impl->properties.template get<LightAnchor>().value;
}

void Light::setAnchor(PropertyValue<LightAnchorType> property) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<LightAnchor>().value = property;
    impl = std::move(impl_);
    observer->onLightChanged(*this);
}

void Light::setAnchorTransition(const TransitionOptions& transition) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<LightAnchor>().transition = transition;
    impl = std::move(impl_);
    observer->onLightChanged(*this);
}

TransitionOptions Light::getAnchorTransition() const {
    return impl->properties.template get<LightAnchor>().transition;
}

Position Light::getDefaultPosition() {
    return LightPosition::defaultValue();
}

PropertyValue<Position> Light::getPosition() const {
    return impl->properties.template get<LightPosition>().value;
}

void Light::setPosition(PropertyValue<Position> property) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<LightPosition>().value = property;
    impl = std::move(impl_);
    observer->onLightChanged(*this);
}

void Light::setPositionTransition(const TransitionOptions& transition) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<LightPosition>().transition = transition;
    impl = std::move(impl_);
    observer->onLightChanged(*this);
}

TransitionOptions Light::getPositionTransition() const {
    return impl->properties.template get<LightPosition>().transition;
}

Color Light::getDefaultColor() {
    return LightColor::defaultValue();
}

PropertyValue<Color> Light::getColor() const {
    return impl->properties.template get<LightColor>().value;
}

void Light::setColor(PropertyValue<Color> property) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<LightColor>().value = property;
    impl = std::move(impl_);
    observer->onLightChanged(*this);
}

void Light::setColorTransition(const TransitionOptions& transition) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<LightColor>().transition = transition;
    impl = std::move(impl_);
    observer->onLightChanged(*this);
}

TransitionOptions Light::getColorTransition() const {
    return impl->properties.template get<LightColor>().transition;
}

float Light::getDefaultIntensity() {
    return LightIntensity::defaultValue();
}

PropertyValue<float> Light::getIntensity() const {
    return impl->properties.template get<LightIntensity>().value;
}

void Light::setIntensity(PropertyValue<float> property) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<LightIntensity>().value = property;
    impl = std::move(impl_);
    observer->onLightChanged(*this);
}

void Light::setIntensityTransition(const TransitionOptions& transition) {
    auto impl_ = mutableImpl();
    impl_->properties.template get<LightIntensity>().transition = transition;
    impl = std::move(impl_);
    observer->onLightChanged(*this);
}

TransitionOptions Light::getIntensityTransition() const {
    return impl->properties.template get<LightIntensity>().transition;
}


} // namespace style
} // namespace mbgl
