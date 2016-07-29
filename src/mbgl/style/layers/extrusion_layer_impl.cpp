#include <mbgl/style/layers/extrusion_layer_impl.hpp>
#include <mbgl/style/bucket_parameters.hpp>
#include <mbgl/renderer/extrusion_bucket.hpp>
#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/intersection_tests.hpp>

namespace mbgl {
    namespace style {

        void ExtrusionLayer::Impl::parsePaints(const JSValue& layer) {
            paint.parse(layer);
        }

        void ExtrusionLayer::Impl::cascade(const CascadeParameters& parameters) {
            paint.cascade(parameters);
        }

        bool ExtrusionLayer::Impl::recalculate(const CalculationParameters& parameters) {
            bool hasTransitions = paint.recalculate(parameters);

            passes = RenderPass::None;

            if (paint.extrusionAntialias) {
                passes |= RenderPass::Translucent;
            }

            if (!paint.extrusionPattern.value.from.empty() || (paint.extrusionColor.value.a * paint.extrusionOpacity) < 1.0f) {
                passes |= RenderPass::Translucent;
            } else {
                passes |= RenderPass::Opaque;
            }

            return hasTransitions;
        }

        std::unique_ptr<Bucket> ExtrusionLayer::Impl::createBucket(BucketParameters& parameters) const {
            auto bucket = std::make_unique<ExtrusionBucket>();

            auto& name = bucketName();
            parameters.eachFilteredFeature(filter, [&] (const auto& feature, std::size_t index, const std::string& layerName) {
                auto geometries = feature.getGeometries();
                bucket->addGeometry(geometries
//                                    , feature
                                    );
                parameters.featureIndex.insert(geometries, index, layerName, name);
            });

            return std::move(bucket);
        }

        float ExtrusionLayer::Impl::getQueryRadius() const {
            // TODO
            const std::array<float, 2>& translate = paint.extrusionTranslate;
            return util::length(translate[0], translate[1]);
        }

        bool ExtrusionLayer::Impl::queryIntersectsGeometry(
                                                      const GeometryCollection& queryGeometry,
                                                      const GeometryCollection& geometry,
                                                      const float bearing,
                                                      const float pixelsToTileUnits) const {
            
            auto translatedQueryGeometry = FeatureIndex::translateQueryGeometry(
                                                                                queryGeometry, paint.extrusionTranslate, paint.extrusionTranslateAnchor, bearing, pixelsToTileUnits);
            
            return util::multiPolygonIntersectsMultiPolygon(translatedQueryGeometry.value_or(queryGeometry), geometry);
        }
        
    } // namespace style
} // namespace mbgl
