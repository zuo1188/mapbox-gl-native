#pragma once

#include <mbgl/geometry/buffer.hpp>
#include <vector>
#include <cstdint>

namespace mbgl {

    class ExtrusionVertexBuffer : public Buffer<
    4 // bytes per coordinates (2 * unsigned short == 4 bytes)
    // TODO ^^ that?
    > {
    public:
        typedef int16_t vertex_type;

        void add(vertex_type x, vertex_type y, vertex_type minH, vertex_type maxH, vertex_type nx, vertex_type ny, vertex_type nz, unsigned short t, unsigned short isUpper, vertex_type e);
    };
    
} // namespace mbgl
