#include <mbgl/text/font_stack.hpp>
#include <cassert>
#include <mbgl/platform/log.hpp>
#include <mbgl/util/math.hpp>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <iostream>
#include <codecvt>

namespace mbgl {

void FontStack::insert(uint32_t id, const SDFGlyph &glyph) {
    auto it = sdfs.find(id);
    if (it == sdfs.end()) {
        // Glyph doesn't exist yet.
        sdfs.emplace(id, glyph);
    } else if (it->second.metrics == glyph.metrics) {
        if (it->second.bitmap != glyph.bitmap) {
            // The actual bitmap was updated; this is unsupported.
            Log::Warning(Event::Glyph, "Modified glyph changed bitmap represenation");
        }
        // At least try to update it in case it's currently unsused.
        // If it is already used; we won't attempt to update the glyph atlas texture.
        it->second.bitmap = glyph.bitmap;
    } else {
        // The metrics were updated; this is unsupported.
        Log::Warning(Event::Glyph, "Modified glyph has different metrics");
        return;
    }
}

const std::map<uint32_t, SDFGlyph> &FontStack::getSDFs() const {
    return sdfs;
}

const Shaping FontStack::getShaping(const std::u32string &string, 
                                    const std::string &font, const float maxWidth,
                                    const float lineHeight, const float horizontalAlign,
                                    const float verticalAlign, const float justify,
                                    const float spacing, const vec2<float> &translate) const {
    Shaping shaping(translate.x * 24, translate.y * 24, string);

    // the y offset *should* be part of the font metadata
    const int32_t yOffset = -17;

    float x = 0;
    const float y = yOffset;

    // TODO: pass string through harfbuzz
    auto hb_buffer_deleter = [](hb_buffer_t * buffer) { hb_buffer_destroy(buffer);};
    const std::unique_ptr<hb_buffer_t, decltype(hb_buffer_deleter)> buffer(hb_buffer_create(),hb_buffer_deleter);
    hb_buffer_pre_allocate(buffer.get(), string.length());

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    std::cout << cv.to_bytes(string) << std::endl;
    std::cout << font << std::endl;

    /*
    mapnik::value_unicode_string const& text = itemizer.text();

    for (auto const& text_item : list)
    {
        face_set_ptr face_set = font_manager.get_face_set(text_item.format_->face_name, text_item.format_->fontset);
        double size = text_item.format_->text_size * scale_factor;
        face_set->set_unscaled_character_sizes();
        std::size_t num_faces = face_set->size();
        std::size_t pos = 0;
        font_feature_settings const& ff_settings = text_item.format_->ff_settings;
        int ff_count = safe_cast<int>(ff_settings.count());

        // rendering information for a single glyph
        struct glyph_face_info
        {
            face_ptr face;
            hb_glyph_info_t glyph;
            hb_glyph_position_t position;
        };
        // this table is filled with information for rendering each glyph, so that 
        // several font faces can be used in a single text_item
        std::vector<glyph_face_info> glyphinfos;
        unsigned valid_glyphs = 0;

        for (auto const& face : *face_set)
        {
            ++pos;
            hb_buffer_clear_contents(buffer.get());
            hb_buffer_add_utf16(buffer.get(), uchar_to_utf16(text.getBuffer()), text.length(), text_item.start, static_cast<int>(text_item.end - text_item.start));
            hb_buffer_set_direction(buffer.get(), (text_item.dir == UBIDI_RTL)?HB_DIRECTION_RTL:HB_DIRECTION_LTR);
            hb_buffer_set_script(buffer.get(), _icu_script_to_script(text_item.script));
            hb_font_t *font(hb_ft_font_create(face->get_face(), nullptr));
            // https://github.com/mapnik/test-data-visual/pull/25
            #if HB_VERSION_MAJOR > 0
             #if HB_VERSION_ATLEAST(1, 0 , 5)
            hb_ft_font_set_load_flags(font,FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);
             #endif
            #endif
            hb_shape(font, buffer.get(), ff_settings.get_features(), ff_count);
            hb_font_destroy(font);

            unsigned num_glyphs = hb_buffer_get_length(buffer.get());

            // if the number of rendered glyphs has increased, we need to resize the table 
            if (num_glyphs > glyphinfos.size())
            {
                glyphinfos.resize(num_glyphs);
            }

            hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer.get(), nullptr);
            hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer.get(), nullptr);

            // Check if all glyphs are valid.
            for (unsigned i=0; i<num_glyphs; ++i)
            {
                // if we have a valid codepoint, save rendering info.
                if (glyphs[i].codepoint)
                {
                    if (!glyphinfos[i].glyph.codepoint)
                    {
                        ++valid_glyphs;
                    }
                    glyphinfos[i] = { face, glyphs[i], positions[i] };
                }
            }
            if (valid_glyphs < num_glyphs && (pos < num_faces))
            {
                //Try next font in fontset
                continue;
            }

            double max_glyph_height = 0;
            for (unsigned i=0; i<num_glyphs; ++i)
            {
                auto& gpos = positions[i];
                auto& glyph = glyphs[i];
                face_ptr theface = face;
                if (glyphinfos[i].glyph.codepoint)
                {
                    gpos = glyphinfos[i].position;
                    glyph = glyphinfos[i].glyph;
                    theface = glyphinfos[i].face;
                }
                unsigned char_index = glyph.cluster;
                glyph_info g(glyph.codepoint,char_index,text_item.format_);
                if (theface->glyph_dimensions(g))
                {
                    g.face = theface;
                    g.scale_multiplier = size / theface->get_face()->units_per_EM;
                    //Overwrite default advance with better value provided by HarfBuzz
                    g.unscaled_advance = gpos.x_advance;
                    g.offset.set(gpos.x_offset * g.scale_multiplier, gpos.y_offset * g.scale_multiplier);
                    double tmp_height = g.height();
                    if (tmp_height > max_glyph_height) max_glyph_height = tmp_height;
                    width_map[char_index] += g.advance();
                    line.add_glyph(std::move(g), scale_factor);
                }
            }
            line.update_max_char_height(max_glyph_height);
            break; //When we reach this point the current font had all glyphs.
        }
    }
    */


    // Loop through all characters of this label and shape.
    for (uint32_t chr : string) {
        auto it = sdfs.find(chr);
        if (it != sdfs.end()) {
            shaping.positionedGlyphs.emplace_back(chr, x, y);
            x += it->second.metrics.advance + spacing;
        }
    }

    if (shaping.positionedGlyphs.empty())
        return shaping;

    lineWrap(shaping, lineHeight, maxWidth, horizontalAlign, verticalAlign, justify, translate);

    return shaping;
}

void align(Shaping &shaping, const float justify, const float horizontalAlign,
           const float verticalAlign, const uint32_t maxLineLength, const float lineHeight,
           const uint32_t line, const vec2<float> &translate) {
    const float shiftX = (justify - horizontalAlign) * maxLineLength + ::round(translate.x * 24/* one em */);
    const float shiftY = (-verticalAlign * (line + 1) + 0.5) * lineHeight + ::round(translate.y * 24/* one em */);

    for (auto& glyph : shaping.positionedGlyphs) {
        glyph.x += shiftX;
        glyph.y += shiftY;
    }
}

void justifyLine(std::vector<PositionedGlyph> &positionedGlyphs, const std::map<uint32_t, SDFGlyph> &sdfs, uint32_t start,
                 uint32_t end, float justify) {
    PositionedGlyph &glyph = positionedGlyphs[end];
    auto it = sdfs.find(glyph.glyph);
    if (it != sdfs.end()) {
        const uint32_t lastAdvance = it->second.metrics.advance;
        const float lineIndent = float(glyph.x + lastAdvance) * justify;

        for (uint32_t j = start; j <= end; j++) {
            positionedGlyphs[j].x -= lineIndent;
        }
    }
}

void FontStack::lineWrap(Shaping &shaping, const float lineHeight, const float maxWidth,
                         const float horizontalAlign, const float verticalAlign,
                         const float justify, const vec2<float> &translate) const {
    uint32_t lastSafeBreak = 0;

    uint32_t lengthBeforeCurrentLine = 0;
    uint32_t lineStartIndex = 0;
    uint32_t line = 0;

    uint32_t maxLineLength = 0;

    std::vector<PositionedGlyph> &positionedGlyphs = shaping.positionedGlyphs;

    if (maxWidth) {
        for (uint32_t i = 0; i < positionedGlyphs.size(); i++) {
            PositionedGlyph &shape = positionedGlyphs[i];

            shape.x -= lengthBeforeCurrentLine;
            shape.y += lineHeight * line;

            if (shape.x > maxWidth && lastSafeBreak > 0) {

                uint32_t lineLength = positionedGlyphs[lastSafeBreak + 1].x;
                maxLineLength = util::max(lineLength, maxLineLength);

                for (uint32_t k = lastSafeBreak + 1; k <= i; k++) {
                    positionedGlyphs[k].y += lineHeight;
                    positionedGlyphs[k].x -= lineLength;
                }

                if (justify) {
                    // Collapse invisible characters.
                    uint32_t breakGlyph = positionedGlyphs[lastSafeBreak].glyph;
                    uint32_t lineEnd = lastSafeBreak;
                    if (breakGlyph == 0x20 /* space */
                        || breakGlyph == 0x200b /* zero-width space */) {
                        lineEnd--;
                    }

                    justifyLine(positionedGlyphs, sdfs, lineStartIndex, lineEnd, justify);
                }

                lineStartIndex = lastSafeBreak + 1;
                lastSafeBreak = 0;
                lengthBeforeCurrentLine += lineLength;
                line++;
            }

            // Spaces, plus word-breaking punctuation that often appears without surrounding spaces.
            if (shape.glyph == 0x20 /* space */
                || shape.glyph == 0x26 /* ampersand */
                || shape.glyph == 0x2b /* plus sign */
                || shape.glyph == 0x2d /* hyphen-minus */
                || shape.glyph == 0x2f /* solidus */
                || shape.glyph == 0xad /* soft hyphen */
                || shape.glyph == 0xb7 /* middle dot */
                || shape.glyph == 0x200b /* zero-width space */
                || shape.glyph == 0x2010 /* hyphen */
                || shape.glyph == 0x2013 /* en dash */) {
                lastSafeBreak = i;
            }
        }
    }

    const PositionedGlyph& lastPositionedGlyph = positionedGlyphs.back();
    const auto lastGlyphIt = sdfs.find(lastPositionedGlyph.glyph);
    assert(lastGlyphIt != sdfs.end());
    const uint32_t lastLineLength = lastPositionedGlyph.x + lastGlyphIt->second.metrics.advance;
    maxLineLength = std::max(maxLineLength, lastLineLength);

    const uint32_t height = (line + 1) * lineHeight;

    justifyLine(positionedGlyphs, sdfs, lineStartIndex, uint32_t(positionedGlyphs.size()) - 1, justify);
    align(shaping, justify, horizontalAlign, verticalAlign, maxLineLength, lineHeight, line, translate);

    // Calculate the bounding box
    shaping.top += -verticalAlign * height;
    shaping.bottom = shaping.top + height;
    shaping.left += -horizontalAlign * maxLineLength;
    shaping.right = shaping.left + maxLineLength;
}

} // end namespace mbgl
