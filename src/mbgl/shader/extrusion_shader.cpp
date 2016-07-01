#include <mbgl/shader/extrusion_shader.hpp>
#include <mbgl/shader/extrusion.vertex.hpp>
#include <mbgl/shader/extrusion.fragment.hpp>
#include <mbgl/gl/gl.hpp>

using namespace mbgl;
using namespace shaders::extrusion;

ExtrusionShader::ExtrusionShader(gl::ObjectStore& store)
: Shader(::name, ::vertex, ::fragment, store) {
}

void ExtrusionShader::bind(GLbyte* offset) {
    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_pos));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_pos, 2, GL_SHORT, false, 0, offset));
}
