#pragma once

#include <mbgl/shader/shader.hpp>
#include <mbgl/shader/uniform.hpp>
#include <mbgl/util/color.hpp>

namespace mbgl {

class ExtrusionShader : public Shader {
public:
    ExtrusionShader(gl::ObjectStore&);

    void bind(GLbyte *offset) final;

    UniformMatrix<4>                u_matrix   = {"u_matrix", *this};
    Uniform<Color>                  u_shadow   = {"u_shadow", *this};
    Uniform<Color>                  u_color    = {"u_color",  *this};
    Uniform<GLfloat>                u_opacity  = {"u_opacity", *this};
    UniformVector<3> u_lightdir = {"u_lightdir", *this};

protected:
    GLint a_minH = -1;
    GLint a_maxH = -1;
    GLint a_normal = -1;
    GLint a_isUpper = -1;
    GLint a_edgedistance = -1;
};

} // namespace mbgl
