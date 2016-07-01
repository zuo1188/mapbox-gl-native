#include <mbgl/shader/extrusion_texture_shader.hpp>
#include <mbgl/shader/extrusiontexture.vertex.hpp>
#include <mbgl/shader/extrusiontexture.fragment.hpp>
#include <mbgl/gl/gl.hpp>

using namespace mbgl;
using namespace shaders::extrusiontexture;

ExtrusionTextureShader::ExtrusionTextureShader(gl::ObjectStore& store)
: Shader(::name, ::vertex, ::fragment, store) {
}

void ExtrusionTextureShader::bind(GLbyte* offset) {
    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_pos));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_pos, 2, GL_SHORT, false, 0, offset));
}
