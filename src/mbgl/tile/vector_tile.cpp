#include <mbgl/tile/vector_tile.hpp>
#include <mbgl/tile/tile_loader_impl.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mapbox/vector_tile.hpp>
#include <protozero/pbf_reader.hpp>

#include <unordered_map>
#include <functional>
#include <utility>

// https://github.com/mapbox/mapbox-gl-native/pull/7939/files#diff-2cd9e5588088fefd22dbf6308e783567
namespace mbgl {

class VectorTileLayer;

struct VectorTileLayerData {
    VectorTileLayerData(std::shared_ptr<const std::string>);
    
    // Hold a reference to the underlying pbf data that backs the lazily-built
    // components of the owning VectorTileLayer and VectorTileFeature objects
    std::shared_ptr<const std::string> data;
};

class VectorTileFeature : public GeometryTileFeature {
public:
    VectorTileFeature(const protozero::data_view &,
                      const mapbox::vector_tile::layer & layer,
                      std::shared_ptr<VectorTileLayerData> layerData);

    FeatureType getType() const override { return static_cast<FeatureType>(tile_feature.getType()); }
    optional<Value> getValue(const std::string&) const override;
    std::unordered_map<std::string,Value> getProperties() const override;
    optional<FeatureIdentifier> getID() const override;
    GeometryCollection getGeometries() const override;

private:
    mapbox::vector_tile::feature tile_feature;
    std::shared_ptr<VectorTileLayerData> layerData;
};
    
class VectorTileLayer : public GeometryTileLayer {
public:
    VectorTileLayer(const protozero::data_view &, std::shared_ptr<const std::string>);

    std::size_t featureCount() const override { return tile_layer.featureCount(); }
    std::unique_ptr<GeometryTileFeature> getFeature(std::size_t) const override;
    std::string getName() const override;

private:
    friend class VectorTileData;
    friend class VectorTileFeature;
    mapbox::vector_tile::layer tile_layer;
    std::shared_ptr<VectorTileLayerData> data;
};

class VectorTileData : public GeometryTileData {
public:
    VectorTileData(std::shared_ptr<const std::string> data);

    std::unique_ptr<GeometryTileData> clone() const override {
        return std::make_unique<VectorTileData>(*this);
    }

    const GeometryTileLayer* getLayer(const std::string&) const override;

private:
    std::shared_ptr<const std::string> data;
    mutable bool parsed = false;
    mutable std::unordered_map<std::string, VectorTileLayer> layers;
};

VectorTile::VectorTile(const OverscaledTileID& id_,
                       std::string sourceID_,
                       const style::UpdateParameters& parameters,
                       const Tileset& tileset)
    : GeometryTile(id_, sourceID_, parameters),
      loader(*this, id_, parameters, tileset) {
}

void VectorTile::setNecessity(Necessity necessity) {
    loader.setNecessity(necessity);
}

void VectorTile::setData(std::shared_ptr<const std::string> data_,
                         optional<Timestamp> modified_,
                         optional<Timestamp> expires_) {
    modified = modified_;
    expires = expires_;

    GeometryTile::setData(data_ ? std::make_unique<VectorTileData>(data_) : nullptr);
}

VectorTileFeature::VectorTileFeature(const protozero::data_view & feature_data,
                                     const mapbox::vector_tile::layer & layer,
                                     std::shared_ptr<VectorTileLayerData> layerData_)
    : tile_feature(feature_data,layer),
      layerData(std::move(layerData_)) {
}

optional<Value> VectorTileFeature::getValue(const std::string& key) const {
    return tile_feature.getValue(key);
}

std::unordered_map<std::string,Value> VectorTileFeature::getProperties() const {
    return tile_feature.getProperties();
}

optional<FeatureIdentifier> VectorTileFeature::getID() const {
    return tile_feature.getID();
}

GeometryCollection VectorTileFeature::getGeometries() const {
    const float scale = float(util::EXTENT) / tile_feature.getExtent();
    GeometryCollection lines = tile_feature.getGeometries<GeometryCollection>(scale);
    if (tile_feature.getVersion() >= 2 || tile_feature.getType() != mapbox::vector_tile::GeomType::POLYGON) {
        return lines;
    }
    return fixupPolygons(lines);
}

VectorTileData::VectorTileData(std::shared_ptr<const std::string> data_)
    : data(std::move(data_)) {
}

const GeometryTileLayer* VectorTileData::getLayer(const std::string& name) const {
    if (!parsed) {
        parsed = true;
        mapbox::vector_tile::buffer tile_buffer(*data);
        for (const auto & tile_layer : tile_buffer.getLayers()) {
            VectorTileLayer layer(tile_layer.second, data);
            layers.emplace(tile_layer.first, std::move(layer));
        }
    }

    auto it = layers.find(name);
    if (it != layers.end()) {
        return &it->second;
    }
    return nullptr;
}

VectorTileLayerData::VectorTileLayerData(std::shared_ptr<const std::string> pbfData) :
    data(std::move(pbfData))
{}

VectorTileLayer::VectorTileLayer(const protozero::data_view & layer_view,
                                 std::shared_ptr<const std::string> pbfData)
    : tile_layer(layer_view),
      data(std::make_shared<VectorTileLayerData>(std::move(pbfData)))
{}

std::unique_ptr<GeometryTileFeature> VectorTileLayer::getFeature(std::size_t i) const {
    return std::make_unique<VectorTileFeature>(tile_layer.getFeature(i), tile_layer, data);
}

std::string VectorTileLayer::getName() const {
    return tile_layer.getName();
}

} // namespace mbgl
