#include <mbgl/shader/extrusion_shader.hpp>
#include <mbgl/shader/extrusion.vertex.hpp>
#include <mbgl/shader/extrusion.fragment.hpp>
#include <mbgl/gl/gl.hpp>

using namespace mbgl;
using namespace shaders::extrusion;

ExtrusionShader::ExtrusionShader(gl::ObjectStore& store)
    : Shader(::name, ::vertex, ::fragment, store)
    , a_minH(MBGL_CHECK_ERROR(glGetAttribLocation(getID(), "a_minH")))
    , a_maxH(MBGL_CHECK_ERROR(glGetAttribLocation(getID(), "a_maxH")))
    , a_normal(MBGL_CHECK_ERROR(glGetAttribLocation(getID(), "a_normal")))
    , a_isUpper(MBGL_CHECK_ERROR(glGetAttribLocation(getID(), "a_isUpper")))
    , a_edgedistance(MBGL_CHECK_ERROR(glGetAttribLocation(getID(), "a_edgedistance")))
{
}

void ExtrusionShader::bind(GLbyte* offset) {
    const GLsizei stride = 18;

    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_pos));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_pos, 2, GL_SHORT, false, stride, offset));

    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_minH));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_minH, 1, GL_SHORT, false, stride, offset + 4));

    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_maxH));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_maxH, 1, GL_SHORT, false, stride, offset + 6));

    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_normal));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_normal, 3, GL_SHORT, false, stride, offset + 8));

    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_isUpper));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_isUpper, 1, GL_SHORT, false, stride, offset + 14));

    MBGL_CHECK_ERROR(glEnableVertexAttribArray(a_edgedistance));
    MBGL_CHECK_ERROR(glVertexAttribPointer(a_edgedistance, 1, GL_SHORT, false, stride, offset + 16));
}
