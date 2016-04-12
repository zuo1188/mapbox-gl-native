#include <mbgl/text/font.hpp>

#include <harfbuzz/hb-ft.h>

#include <fstream>
#include <iostream>

#include <cstdint>
#include <codecvt>

namespace mbgl {

Font::Font(const std::string &filename)
    : filename_(filename),
    font_(0),
    buffer_(hb_buffer_create()) {
      load();
}

Font::~Font() {
    hb_buffer_destroy(buffer_);
    hb_font_destroy(font_);
}

void Font::load() {
    if (font_) return;

    char *font_data;
    unsigned int size;

    std::ifstream file(filename_.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        font_data = new char[size];
        file.seekg(0, std::ios::beg);
        file.read(font_data, size);
        file.close();
    } else {
        std::cerr << "Could not open font!\n";
        return;
    }

    hb_blob_t *blob = hb_blob_create(font_data, size, HB_MEMORY_MODE_WRITABLE, font_data, [](void* data) {
        delete[] static_cast<char*>(data);
    });
    hb_face_t *face = hb_face_create(blob, 0 /*face_index*/);
    hb_blob_destroy(blob);
    font_ = hb_font_create(face);

#if 1
    // TODO: Font size
    unsigned int upem = hb_face_get_upem(face);
    hb_font_set_scale(font_, upem, upem);
#endif
    hb_face_destroy(face);
    hb_ft_font_set_funcs(font_);
}

void Font::shape(const std::u32string &text)
{
    if (!font_) return;

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    std::cout << cv.to_bytes(text) << std::endl;

    hb_buffer_reset(buffer_);

    hb_buffer_add_utf32(buffer_, reinterpret_cast<const uint32_t*>(text.c_str()), text.length(), 0, text.length());

    hb_buffer_set_direction(buffer_, HB_DIRECTION_RTL);
    hb_buffer_set_script(buffer_, HB_SCRIPT_ARABIC);
    hb_buffer_set_language(buffer_, hb_language_from_string("ar", -1));

#if 0
    hb_buffer_set_direction(buffer, hb_direction_from_string (direction, -1));
    hb_buffer_set_script(buffer, hb_script_from_string (script, -1));
    hb_buffer_set_language(buffer, hb_language_from_string (language, -1));
#endif

    hb_shape(font_, buffer_, 0 /*features*/, 0 /*num_features*/);

    int num_glyphs = hb_buffer_get_length(buffer_);
    hb_glyph_info_t *hb_glyph = hb_buffer_get_glyph_infos(buffer_, NULL);
    hb_glyph_position_t *hb_position = hb_buffer_get_glyph_positions(buffer_, NULL);

    for (int i = 0; i < num_glyphs; i++) {
        std::cout << "glyph codepoint:" << hb_glyph[i].codepoint <<
                     " cluster: " << hb_glyph[i].cluster <<
                     " mask: " << hb_glyph[i].mask <<
                     " x_advance: "<< hb_position[i].x_advance << "\n";
                     // " y_advance: "<< hb_position[i].y_advance <<
                     // " x_offset: "<< hb_position[i].x_offset <<
                     // " y_offset: "<< hb_position[i].y_offset << "\n";
        // std::cout << "glyph:" << hb_glyph->codepoint << "\n";
    }

    /*
    auto hb_buffer_deleter = [](hb_buffer_t * buffer) { hb_buffer_destroy(buffer);};
    const std::unique_ptr<hb_buffer_t, decltype(hb_buffer_deleter)> buffer(hb_buffer_create(),hb_buffer_deleter);
    hb_buffer_pre_allocate(buffer.get(), string.length());

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
}

} // end namespace mbgl
