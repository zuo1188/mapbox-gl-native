#ifndef MBGL_TEXT_FONT
#define MBGL_TEXT_FONT

#include <string>

#include <harfbuzz/hb.h>

namespace mbgl {

class Font {
public:
    Font(const std::string &filename);
    ~Font();

    void shape(const std::u32string &text);

private:
    void load();

    std::string filename_;
    hb_font_t *font_;
    hb_buffer_t *buffer_;
};

} // end namespace mbgl

#endif
