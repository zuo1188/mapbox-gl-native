#include <mbgl/geometry/extrusion_buffer.hpp>

#include <mbgl/gl/gl.hpp>

#include <climits>
#include <cmath>

using namespace mbgl;

void ExtrusionVertexBuffer::add(vertex_type x, vertex_type y, vertex_type minH, vertex_type maxH, double nx, double ny, double nz, unsigned short t, vertex_type e) {
    vertex_type *vertices = static_cast<vertex_type *>(addElement());

    const auto factor = pow(2, 13);

    // a_pos
    vertices[0] = x;
    vertices[1] = y;

    // a_heights -- temp workaround
    vertices[2] = minH;
    vertices[3] = maxH;

    // a_normal
    vertices[4] = floor(nx * factor) * 2 + t;
    vertices[5] = ny * factor * 2;
    vertices[6] = nz * factor * 2;

    // a_isUpper
    vertices[7] = t;

    // a_edgedistance
    vertices[8] = e;
}
