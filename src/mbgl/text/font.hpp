#ifndef MBGL_TEXT_FONT
#define MBGL_TEXT_FONT

#include <mbgl/text/glyph.hpp>
#include <mbgl/util/math.hpp>

#include <string>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

namespace mbgl {

class Font {
public:
    Font(const std::string &filename);
    ~Font();

    Shaping shape(const std::u32string &text, const float spacing, const vec2<float> &translate, const std::map<uint32_t, SDFGlyph> &sdfs);

private:
    void load();

    std::string filename_;
    std::vector<FT_Face> ft_faces;
    hb_font_t *font_;
    hb_buffer_t *buffer_;
    unsigned int upem_;
    double size = 0.0;
};

} // end namespace mbgl

#endif
