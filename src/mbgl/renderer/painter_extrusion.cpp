#include <mbgl/renderer/painter.hpp>
#include <mbgl/renderer/extrusion_bucket.hpp>
#include <mbgl/style/layers/extrusion_layer.hpp>
#include <mbgl/style/layers/extrusion_layer_impl.hpp>
#include <mbgl/sprite/sprite_atlas.hpp>
//#include <mbgl/shader/outline_shader.hpp>
//#include <mbgl/shader/outlinepattern_shader.hpp>
#include <mbgl/shader/extrusion_shader.hpp>
#include <mbgl/shader/extrusion_pattern_shader.hpp>
#include <mbgl/shader/extrusion_texture_shader.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/vec3.hpp>


namespace mbgl {

    using namespace style;

    void Painter::renderExtrusion(ExtrusionBucket& bucket,
                                  const ExtrusionLayer& layer,
                                  const UnwrappedTileID& tileID,
                                  const mat4& matrix) {
        const ExtrusionPaintProperties& properties = layer.impl->paint;
        mat4 vertexMatrix;
        const auto zScale = pow(2, state.getZoom() / 50000);
        matrix::scale(vertexMatrix, translatedMatrix(matrix, properties.extrusionTranslate, tileID, properties.extrusionTranslateAnchor), 1, 1, zScale);

//        mat4 vertexMatrix =
//            translatedMatrix(matrix, properties.extrusionTranslate, tileID, properties.extrusionTranslateAnchor);
        // TODO not sure if the intermediary translate matrix works -- have not yet checked in JS

        Color extrusionColor = properties.extrusionColor;
        Color extrusionShadowColor = properties.extrusionShadowColor;
        float opacity = properties.extrusionOpacity;

        Color strokeColor = properties.extrusionOutlineColor;
        bool isOutlineColorDefined = strokeColor.a >= 0;
        if (!isOutlineColorDefined) {
            strokeColor = extrusionColor;
        }

        config.depthMask = GL_TRUE;

        // CREATE AND BIND NEW TEXTURE
//        config.activeTexture = GL_TEXTURE1;
//        bucket.texture.load(state.getWidth(), state.getHeight());
//        bucket.texture.bind(texturePool, store);

        // SET TEXTURE CONFIG OPTIONS

        // TODO are these available as config options?
        // TODO i commented these out to make this not turn all blue -- ...
//        glClearStencil(0x80);
//        glStencilMask(0xFF);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glStencilMask(0x00);

        setDepthSublayer(2);
//        config.depthTest = GL_TRUE;

        // DRAW EXTRUSIONS ONTO TEXTURE

        bool pattern = !properties.extrusionPattern.value.from.empty();
//        bool outline = properties.extrusionAntialias && !pattern && isOutlineColorDefined;
//        bool fringeline = properties.extrusionAntialias && !pattern && !isOutlineColorDefined;

        if (pattern) {
            // TODO TODO
        } else {
            // No pattern.
            if ((extrusionColor.a >= 1.0f && opacity >= 1.0f) == (pass == RenderPass::Opaque)) {
                // Only draw the extrusion when it's either opaque and we're drawing opaque
                // fragments or when it's translucent and we're drawing translucent
                // fragments
                // Draw filling rectangle.
                config.program = isOverdraw() ? extrusionShader->getOverdrawID() : extrusionShader->getID();
                extrusionShader->u_color = extrusionColor;
                extrusionShader->u_shadow = extrusionShadowColor;
                extrusionShader->u_opacity = opacity;
                extrusionShader->u_matrix = vertexMatrix;
                mat3 lightmat;
                vec3 lightdir;
                matrix::rotate(lightmat, lightmat, -state.getAngle());
                vector::transformMat3(lightdir, lightdir, lightmat);
                extrusionShader->u_lightdir = lightdir;

                // Draw the actual triangles into the color & stencil buffer.
//                setDepthSublayer(1);

                bucket.drawElements(*extrusionShader, store);
            }
        }

        // DRAW STROKE ONTO TEXTURE

        // UNBIND TEXTURE + SET CONFIG

//        bucket.texture.unbindFramebuffer();
//
//        config.activeTexture = GL_TEXTURE0;
//        // TODO i suspect i need to separate the texture ptr out here, judging from searching glBindTexture elsewhere in this codebase
//        MBGL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
////        MBGL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, *bucket.texture.texture));
//
//        extrusionTextureShader->u_opacity = opacity;
//        extrusionTextureShader->u_texture = *bucket.texture.texture;
//
//        mat4 textureMatrix;
//        matrix::ortho(textureMatrix, 0, state.getWidth(), state.getHeight(), 0, 0, 1);
//        extrusionTextureShader->u_matrix = textureMatrix;
//
//        config.depthTest = GL_FALSE;
//
//        extrusionTextureShader->u_xdim = state.getWidth();
//        extrusionTextureShader->u_ydim = state.getHeight();
//
//        extrusionTextureArray.bind(*extrusionTextureShader, tileStencilBuffer, BUFFER_OFFSET(0), store);
//        MBGL_CHECK_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)tileStencilBuffer.index()));


        // PAINT TEXTURE TO MAP








//
////        auto worldSize = util::convert<GLfloat>(frame.framebufferSize);
//
//        config.stencilOp.reset();
//        config.stencilTest = GL_TRUE;
//        config.depthFunc.reset();
//        config.depthTest = GL_TRUE;
//        config.depthMask = GL_TRUE;
//        config.lineWidth = 2.0f; // This is always fixed and does not depend on the pixelRatio!
//
//        // Because we're drawing top-to-bottom, and we update the stencil mask
//        // befrom, we have to draw the outline first (!)
//        if (outline && pass == RenderPass::Translucent) {
//            config.program = isOverdraw() ? outlineShader->getOverdrawID() : outlineShader->getID();
//            outlineShader->u_matrix = vertexMatrix;
//
//            outlineShader->u_outline_color = strokeColor;
//            outlineShader->u_opacity = opacity;
//
//            // Draw the entire line
//            outlineShader->u_world = worldSize;
//            if (isOutlineColorDefined) {
//                // If we defined a different color for the fill outline, we are
//                // going to ignore the bits in 0x07 and just care about the global
//                // clipping mask.
//                setDepthSublayer(2); // OK
//            } else {
//                // Otherwise, we only want to drawFill the antialiased parts that are
//                // *outside* the current shape. This is important in case the fill
//                // or stroke color is translucent. If we wouldn't clip to outside
//                // the current shape, some pixels from the outline stroke overlapped
//                // the (non-antialiased) fill.
//                setDepthSublayer(0); // OK
//            }
//            bucket.drawVertices(*outlineShader, store);
//        }
//
//        if (pattern) {
//            optional<SpriteAtlasPosition> imagePosA = spriteAtlas->getPosition(properties.fillPattern.value.from, true);
//            optional<SpriteAtlasPosition> imagePosB = spriteAtlas->getPosition(properties.fillPattern.value.to, true);
//
//            // Image fill.
//            if (pass == RenderPass::Translucent && imagePosA && imagePosB) {
//                config.program = isOverdraw() ? patternShader->getOverdrawID() : patternShader->getID();
//                patternShader->u_matrix = vertexMatrix;
//                patternShader->u_pattern_tl_a = imagePosA->tl;
//                patternShader->u_pattern_br_a = imagePosA->br;
//                patternShader->u_pattern_tl_b = imagePosB->tl;
//                patternShader->u_pattern_br_b = imagePosB->br;
//                patternShader->u_opacity = properties.fillOpacity;
//                patternShader->u_image = 0;
//                patternShader->u_mix = properties.fillPattern.value.t;
//                patternShader->u_pattern_size_a = imagePosA->size;
//                patternShader->u_pattern_size_b = imagePosB->size;
//                patternShader->u_scale_a = properties.fillPattern.value.fromScale;
//                patternShader->u_scale_b = properties.fillPattern.value.toScale;
//                patternShader->u_tile_units_to_pixels = 1.0f / tileID.pixelsToTileUnits(1.0f, state.getIntegerZoom());
//
//                GLint tileSizeAtNearestZoom = util::tileSize * state.zoomScale(state.getIntegerZoom() - tileID.canonical.z);
//                GLint pixelX = tileSizeAtNearestZoom * (tileID.canonical.x + tileID.wrap * state.zoomScale(tileID.canonical.z));
//                GLint pixelY = tileSizeAtNearestZoom * tileID.canonical.y;
//                patternShader->u_pixel_coord_upper = {{ float(pixelX >> 16), float(pixelY >> 16) }};
//                patternShader->u_pixel_coord_lower = {{ float(pixelX & 0xFFFF), float(pixelY & 0xFFFF) }};
//
//                config.activeTexture = GL_TEXTURE0;
//                spriteAtlas->bind(true, store);
//
//                // Draw the actual triangles into the color & stencil buffer.
//                setDepthSublayer(0);
//                bucket.drawElements(*patternShader, store);
//
//                if (properties.fillAntialias && !isOutlineColorDefined) {
//                    config.program = isOverdraw() ? outlinePatternShader->getOverdrawID() : outlinePatternShader->getID();
//                    outlinePatternShader->u_matrix = vertexMatrix;
//
//                    // Draw the entire line
//                    outlineShader->u_world = worldSize;
//
//                    outlinePatternShader->u_pattern_tl_a = imagePosA->tl;
//                    outlinePatternShader->u_pattern_br_a = imagePosA->br;
//                    outlinePatternShader->u_pattern_tl_b = imagePosB->tl;
//                    outlinePatternShader->u_pattern_br_b = imagePosB->br;
//                    outlinePatternShader->u_opacity = properties.fillOpacity;
//                    outlinePatternShader->u_image = 0;
//                    outlinePatternShader->u_mix = properties.fillPattern.value.t;
//                    outlinePatternShader->u_pattern_size_a = imagePosA->size;
//                    outlinePatternShader->u_pattern_size_b = imagePosB->size;
//                    outlinePatternShader->u_scale_a = properties.fillPattern.value.fromScale;
//                    outlinePatternShader->u_scale_b = properties.fillPattern.value.toScale;
//                    outlinePatternShader->u_tile_units_to_pixels = 1.0f / tileID.pixelsToTileUnits(1.0f, state.getIntegerZoom());
//                    outlinePatternShader->u_pixel_coord_upper = {{ float(pixelX >> 16), float(pixelY >> 16) }};
//                    outlinePatternShader->u_pixel_coord_lower = {{ float(pixelX & 0xFFFF), float(pixelY & 0xFFFF) }};
//
//                    config.activeTexture = GL_TEXTURE0;
//                    spriteAtlas->bind(true, store);
//
//                    setDepthSublayer(2);
//                    bucket.drawVertices(*outlinePatternShader, store);
//                }
//            }
//        } else {
//            // No image fill.
//            if ((fillColor.a >= 1.0f && opacity >= 1.0f) == (pass == RenderPass::Opaque)) {
//                // Only draw the fill when it's either opaque and we're drawing opaque
//                // fragments or when it's translucent and we're drawing translucent
//                // fragments
//                // Draw filling rectangle.
//                config.program = isOverdraw() ? plainShader->getOverdrawID() : plainShader->getID();
//                plainShader->u_matrix = vertexMatrix;
//                plainShader->u_color = fillColor;
//                plainShader->u_opacity = opacity;
//
//                // Draw the actual triangles into the color & stencil buffer.
//                setDepthSublayer(1);
//                bucket.drawElements(*plainShader, store);
//            }
//        }
//        
//        // Because we're drawing top-to-bottom, and we update the stencil mask
//        // below, we have to draw the outline first (!)
//        if (fringeline && pass == RenderPass::Translucent) {
//            config.program = isOverdraw() ? outlineShader->getOverdrawID() : outlineShader->getID();
//            outlineShader->u_matrix = vertexMatrix;
//            
//            outlineShader->u_outline_color = fillColor;
//            outlineShader->u_opacity = opacity;
//            
//            // Draw the entire line
//            outlineShader->u_world = worldSize;
//            
//            setDepthSublayer(2);
//            bucket.drawVertices(*outlineShader, store);
//        }
    }
    
} // namespace mbgl
