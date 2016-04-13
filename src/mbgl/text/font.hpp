#ifndef MBGL_TEXT_FONT
#define MBGL_TEXT_FONT

#include <mbgl/text/glyph.hpp>
#include <mbgl/util/math.hpp>

#include <string>

#include <harfbuzz/hb.h>

namespace mbgl {

class Font {
public:
    Font(const std::string &filename);
    ~Font();

    Shaping shape(const std::u32string &text, const float spacing, const vec2<float> &translate);

private:
    void load();

    std::string filename_;
    hb_font_t *font_;
    hb_buffer_t *buffer_;
    unsigned int upem_;
};

} // end namespace mbgl

#endif
