#include <mbgl/text/font.hpp>

#include <fstream>
#include <iostream>
#include <codecvt>

// freetype2
extern "C" {
#include FT_GLYPH_H
// #include FT_OUTLINE_H
}

namespace mbgl {

struct ft_library_guard {
    ft_library_guard(FT_Library* lib) :
        library_(lib) {}

    ~ft_library_guard()
    {
        if (library_) FT_Done_FreeType(*library_);
    }

    FT_Library* library_;
};

struct ft_glyph_guard {
    ft_glyph_guard(FT_Glyph* glyph) :
        glyph_(glyph) {}

    ~ft_glyph_guard()
    {
        if (glyph_) FT_Done_Glyph(*glyph_);
    }

    FT_Glyph* glyph_;
};

Font::Font(const std::string &filename)
    : filename_(filename),
    font_(0),
    buffer_(hb_buffer_create()) {
      load();
}

Font::~Font() {
    /*
    for (FT_Face ft_face : ft_faces) {
        FT_Done_Face(ft_face);
    }
    */

    hb_buffer_destroy(buffer_);
    hb_font_destroy(font_);
}

void Font::load() {
    if (font_) return;

    char *font_data;
    size_t font_size;

    std::ifstream file(filename_.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open())
    {
        font_size = file.tellg();
        font_data = new char[font_size];
        file.seekg(0, std::ios::beg);
        file.read(font_data, font_size);
        file.close();
    } else {
        std::cerr << "Could not open font!\n";
        return;
    }

/* ------------------------------------------------------------------ */

    FT_Library library = nullptr;
    ft_library_guard library_guard(&library);
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        throw std::runtime_error("Could not open FreeType library");
    }

    FT_Face ft_face = 0;
    size_t num_faces = 0;
    for (size_t i = 0; ft_face == 0 || i < num_faces; ++i) {
        FT_Error face_error = FT_New_Memory_Face(library, reinterpret_cast<const FT_Byte*>(font_data), static_cast<FT_Long>(font_size), i, &ft_face);
        if (face_error) {
            throw std::runtime_error("Could not open font file");
        }

        if (num_faces == 0) {
            num_faces = ft_face->num_faces;
            ft_faces.reserve(num_faces);
        }

        ft_faces.push_back(ft_face);
    }

/* ------------------------------------------------------------------ */

    hb_blob_t *blob = hb_blob_create(font_data, font_size, HB_MEMORY_MODE_WRITABLE, font_data, [](void* data) {
        delete[] static_cast<char*>(data);
    });
    hb_face_t *face = hb_face_create(blob, 0 /*face_index*/);
    hb_blob_destroy(blob);
    font_ = hb_font_create(face);

    // TODO: Font size?
    upem_ = hb_face_get_upem(face);

    // TOOD: currently hardcoded in node-fontnik
    size = (FT_F26Dot6)(24 * (1<<6)) / (double)upem_;
    hb_font_set_scale(font_, size, size);

    hb_face_destroy(face);
    hb_ft_font_set_funcs(font_);
}

Shaping Font::shape(const std::u32string &text, const float spacing, const vec2<float> &translate, const std::map<uint32_t, SDFGlyph> &sdfs) {
    Shaping shaping(translate.x * 24, translate.y * 24, text);

    // TODO: the y offset *should* be part of the font metadata
    const int32_t yOffset = -17;

    float x = 0;
    const float y = yOffset;

    if (!font_) return shaping;

    // TODO: itemize string here
    hb_buffer_reset(buffer_);
    hb_buffer_add_utf32(buffer_, reinterpret_cast<const uint32_t*>(text.c_str()), text.length(), 0, text.length());

    hb_buffer_set_direction(buffer_, HB_DIRECTION_LTR);
    // hb_buffer_set_direction(buffer, hb_direction_from_string (direction, -1));

    hb_buffer_set_script(buffer_, HB_SCRIPT_LATIN);
    // hb_buffer_set_script(buffer, hb_script_from_string (script, -1));

    hb_buffer_set_language(buffer_, hb_language_from_string("en", -1));
    // hb_buffer_set_language(buffer, hb_language_from_string (language, -1));

    hb_shape(font_, buffer_, 0 /*features*/, 0 /*num_features*/);

    int num_glyphs = hb_buffer_get_length(buffer_);
    hb_glyph_info_t *hb_glyph = hb_buffer_get_glyph_infos(buffer_, NULL);
    hb_glyph_position_t *hb_position = hb_buffer_get_glyph_positions(buffer_, NULL);

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
    std::cout << cv.to_bytes(text) << 
                 " num_glyphs: " << num_glyphs <<
                 " upem: " << upem_ <<
                 " size: " << size <<
                 " spacing: " << spacing << std::endl;

    // Loop through all characters of this label and shape.
    for (uint32_t chr : text) {
        auto it = sdfs.find(chr);
        if (it != sdfs.end()) {
            std::cout << "chr: " << chr <<
                         " x_advance: "<< it->second.metrics.advance + spacing << std::endl;

            shaping.positionedGlyphs.emplace_back(chr, x, y);
            x += it->second.metrics.advance + spacing;
        }
    }

    for (int i = 0; i < num_glyphs; i++) {
        uint32_t glyph = 0;
        hb_font_get_glyph(font_, hb_glyph[i].codepoint, 0, &glyph);

        char* name = new char[128];
        hb_font_glyph_to_string(font_, hb_glyph[i].codepoint, name, 128);

        std::cout << "glyph_index: " << hb_glyph[i].codepoint <<
                     " name: " << name <<
                     " x_advance: "<< hb_position[i].x_advance <<
                     " cluster: " << hb_glyph[i].cluster <<
                     " mask: " << hb_glyph[i].mask <<
                     " x_advance: "<< hb_position[i].x_advance <<
                     " y_advance: "<< hb_position[i].y_advance <<
                     " x_offset: "<< hb_position[i].x_offset <<
                     " y_offset: "<< hb_position[i].y_offset << std::endl;

        FT_ULong char_code;
        FT_UInt glyph_index;
        char_code = FT_Get_First_Char(ft_faces[0], &glyph_index);
        std::cout << "char_code: " << char_code << std::endl;

        /*
        FT_UInt glyph_index = reinterpret_cast<FT_UInt>(hb_glyph[i].codepoint);

        if (FT_Load_Glyph(ft_faces[0], glyph_index, FT_LOAD_NO_HINTING)) {
            throw std::runtime_error("Failed to load glyph");
        }

        FT_Glyph ft_glyph = nullptr;
        ft_glyph_guard glyph_guard(&ft_glyph);
        if (FT_Get_Glyph(ft_faces[0]->glyph, &ft_glyph)) {
            throw std::runtime_error("Failed to get glyph");
        }
        */
    }

    return shaping;
}

} // end namespace mbgl
