#include <mbgl/style/style.hpp>
#include <mbgl/style/style_impl.hpp>
#include <mbgl/style/image.hpp>
#include <mbgl/style/source.hpp>
#include <mbgl/style/layer.hpp>

namespace mbgl {
namespace style {

Style::Style(Scheduler& scheduler, FileSource& fileSource, float pixelRatio)
    : impl(std::make_unique<Impl>(scheduler, fileSource, pixelRatio)) {
}

Style::~Style() = default;

std::string Style::getName() const {
    return impl->getName();
}

LatLng Style::getDefaultLatLng() const {
    return impl->getDefaultLatLng();
}

double Style::getDefaultZoom() const {
    return impl->getDefaultZoom();
}

double Style::getDefaultBearing() const {
    return impl->getDefaultBearing();
}

double Style::getDefaultPitch() const {
    return impl->getDefaultPitch();
}

TransitionOptions Style::getTransitionOptions() const {
    return impl->getTransitionOptions();
}

void Style::setTransitionOptions(const TransitionOptions& options) {
    impl->mutated = true;
    impl->setTransitionOptions(options);
}

const Image* Style::getImage(const std::string& name) const {
    return impl->getImage(name);
}

void Style::addImage(const std::string& name, std::unique_ptr<Image> image) {
    impl->mutated = true;
    impl->addImage(name, std::move(image));
    impl->observer->onUpdate(Update::Repaint);
}

void Style::removeImage(const std::string& name) {
    impl->mutated = true;
    impl->removeImage(name);
    impl->observer->onUpdate(Update::Repaint);
}

std::vector<Source*> Style::getSources() {
    impl->mutated = true;
    return impl->getSources();
}

std::vector<const Source*> Style::getSources() const {
    return const_cast<const Impl&>(*impl).getSources();
}

Source* Style::getSource(const std::string& id) {
    impl->mutated = true;
    return impl->getSource(id);
}

const Source* Style::getSource(const std::string& id) const {
    return impl->getSource(id);
}

void Style::addSource(std::unique_ptr<Source> source) {
    impl->mutated = true;
    impl->addSource(std::move(source));
}

std::unique_ptr<Source> Style::removeSource(const std::string& sourceID) {
    impl->mutated = true;
    return impl->removeSource(sourceID);
}

std::vector<Layer*> Style::getLayers() {
    impl->mutated = true;
    return impl->getLayers();
}

std::vector<const Layer*> Style::getLayers() const {
    return const_cast<const Impl&>(*impl).getLayers();
}

Layer* Style::getLayer(const std::string& layerID) {
    impl->mutated = true;
    return impl->getLayer(layerID);
}

const Layer* Style::getLayer(const std::string& layerID) const {
    return impl->getLayer(layerID);
}

void Style::addLayer(std::unique_ptr<Layer> layer, const optional<std::string>& before) {
    impl->mutated = true;
//    BackendScope guard(impl->backend);

    impl->observer->onUpdate(Update::Classes);
    impl->addLayer(std::move(layer), before);
}

std::unique_ptr<Layer> Style::removeLayer(const std::string& id) {
    impl->mutated = true;
//    BackendScope guard(impl->backend);

    impl->observer->onUpdate(Update::Classes);
    return impl->removeLayer(id);
}

void Style::addClass(const std::string& className) {
    if (impl->addClass(className)) {
        impl->observer->onUpdate(Update::Classes);
    }
}

void Style::removeClass(const std::string& className) {
    if (impl->removeClass(className)) {
        impl->observer->onUpdate(Update::Classes);
    }
}

void Style::setClasses(const std::vector<std::string>& classNames) {
    impl->setClasses(classNames);
    impl->observer->onUpdate(Update::Classes);
}

bool Style::hasClass(const std::string& className) const {
    return impl->hasClass(className);
}

std::vector<std::string> Style::getClasses() const {
    return impl->getClasses();
}

} // namespace style
} // namespace mbgl
