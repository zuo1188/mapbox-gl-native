#include <mbgl/shader/extrusion_pattern_shader.hpp>
#include <mbgl/shader/extrusionpattern.vertex.hpp>
#include <mbgl/shader/extrusionpattern.fragment.hpp>
#include <mbgl/gl/gl.hpp>

using namespace mbgl;
using namespace shaders::extrusionpattern;

ExtrusionPatternShader::ExtrusionPatternShader(gl::ObjectStore& store)
: Shader(::name, ::vertex, ::fragment, store) {
}

void ExtrusionPatternShader::bind(GLbyte* offset) {
    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_pos));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_pos, 2, GL_SHORT, false, 0, offset));
}
