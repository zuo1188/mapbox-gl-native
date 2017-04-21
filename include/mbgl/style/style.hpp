#pragma once

#include <mbgl/style/transition_options.hpp>
#include <mbgl/util/geo.hpp>

#include <string>
#include <vector>

namespace mbgl {

class FileSource;
class Scheduler;

namespace style {

class Image;
class Source;
class Layer;

class Style {
public:
    Style(Scheduler&, FileSource&, float pixelRatio);
    ~Style();

    // Defaults
    std::string getName() const;
    LatLng getDefaultLatLng() const;
    double getDefaultZoom() const;
    double getDefaultBearing() const;
    double getDefaultPitch() const;

    // TransitionOptions
    TransitionOptions getTransitionOptions() const;
    void setTransitionOptions(const TransitionOptions&);

    // Images
    const Image* getImage(const std::string&) const;
    void addImage(const std::string&, std::unique_ptr<Image>);
    void removeImage(const std::string&);

    // Sources
    std::vector<      Source*> getSources();
    std::vector<const Source*> getSources() const;

          Source* getSource(const std::string&);
    const Source* getSource(const std::string&) const;

    void addSource(std::unique_ptr<Source>);
    std::unique_ptr<Source> removeSource(const std::string& sourceID);

    // Layers
    std::vector<      Layer*> getLayers();
    std::vector<const Layer*> getLayers() const;

          Layer* getLayer(const std::string&);
    const Layer* getLayer(const std::string&) const;

    void addLayer(std::unique_ptr<Layer>, const optional<std::string>& beforeLayerID = {});
    std::unique_ptr<Layer> removeLayer(const std::string& layerID);

    // Classes
    void addClass(const std::string&);
    void removeClass(const std::string&);
    bool hasClass(const std::string&) const;
    void setClasses(const std::vector<std::string>&);
    std::vector<std::string> getClasses() const;

    // Private implementation
    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace style
} // namespace mbgl
