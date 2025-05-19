#pragma once

#include <vector>

namespace kRendrr
{
    /**
     * Generates an array of floats forming a single triangle
     * | 3xFLOAT position | 2xFLOAT uv |
     */
    constexpr std::vector<float> GenerateTriangleMeshVertices(bool bUseIndices)
    {
        // It's same with indices and without them

        return {
            /* Position */  0.f,   0.5f, 0.f, /* UV */  0.5f,  0.0f,
            /* Position */  0.5f, -0.5f, 0.f, /* UV */  1.0f,  1.0f,
            /* Position */ -0.5f, -0.5f, 0.f, /* UV */  0.0f,  1.0f,
        };
    }

    constexpr std::vector<std::uint32_t> GenerateTriangleMeshIndices()
    {
        return {
            0, 1, 2
        };
    }

    /**
     * Generates an array of floats forming a unit cube
     * | 3xFLOAT position | 2xFLOAT uv |
     */
    constexpr std::vector<float> GenerateCubeMeshVertices(bool bUseIndices)
    {
        if(bUseIndices)
        {
            return {
                /* Vert 0 */ /* Position */ -1.0f, -1.0f, -1.0f, /* UV */ 0.f, 0.f,
                /* Vert 1 */ /* Position */ -1.0f,  1.0f, -1.0f, /* UV */ 0.f, 1.f,
                /* Vert 2 */ /* Position */  1.0f,  1.0f, -1.0f, /* UV */ 1.f, 1.f,
                /* Vert 3 */ /* Position */  1.0f, -1.0f, -1.0f, /* UV */ 1.f, 0.f,
                /* Vert 4 */ /* Position */ -1.0f, -1.0f,  1.0f, /* UV */ 0.f, 1.f,
                /* Vert 5 */ /* Position */ -1.0f,  1.0f,  1.0f, /* UV */ 0.f, 0.f,
                /* Vert 6 */ /* Position */  1.0f,  1.0f,  1.0f, /* UV */ 1.f, 0.f,
                /* Vert 7 */ /* Position */  1.0f, -1.0f,  1.0f, /* UV */ 1.f, 1.f,
            };
        }

        return {
            /* Face 1 */

            /* Vert 0 */ /* Position */ -1.0f, -1.0f, -1.0f, /* UV */ 0.f, 0.f,
            /* Vert 1 */ /* Position */ -1.0f,  1.0f, -1.0f, /* UV */ 0.f, 1.f,
            /* Vert 2 */ /* Position */  1.0f,  1.0f, -1.0f, /* UV */ 1.f, 1.f,
            /* Vert 0 */ /* Position */ -1.0f, -1.0f, -1.0f, /* UV */ 0.f, 0.f,
            /* Vert 2 */ /* Position */  1.0f,  1.0f, -1.0f, /* UV */ 1.f, 1.f,
            /* Vert 3 */ /* Position */  1.0f, -1.0f, -1.0f, /* UV */ 1.f, 0.f,

            /* Face 2 */

            /* Vert 4 */ /* Position */ -1.0f, -1.0f,  1.0f, /* UV */ 0.f, 1.f,
            /* Vert 6 */ /* Position */  1.0f,  1.0f,  1.0f, /* UV */ 1.f, 0.f,
            /* Vert 5 */ /* Position */ -1.0f,  1.0f,  1.0f, /* UV */ 0.f, 0.f,
            /* Vert 4 */ /* Position */ -1.0f, -1.0f,  1.0f, /* UV */ 0.f, 1.f,
            /* Vert 7 */ /* Position */  1.0f, -1.0f,  1.0f, /* UV */ 1.f, 1.f,
            /* Vert 6 */ /* Position */  1.0f,  1.0f,  1.0f, /* UV */ 1.f, 0.f,

            /* Face 3 */

            /* Vert 4 */ /* Position */ -1.0f, -1.0f,  1.0f, /* UV */ 0.f, 1.f,
            /* Vert 5 */ /* Position */ -1.0f,  1.0f,  1.0f, /* UV */ 0.f, 0.f,
            /* Vert 1 */ /* Position */ -1.0f,  1.0f, -1.0f, /* UV */ 0.f, 1.f,
            /* Vert 4 */ /* Position */ -1.0f, -1.0f,  1.0f, /* UV */ 0.f, 1.f,
            /* Vert 1 */ /* Position */ -1.0f,  1.0f, -1.0f, /* UV */ 0.f, 1.f,
            /* Vert 0 */ /* Position */ -1.0f, -1.0f, -1.0f, /* UV */ 0.f, 0.f,

            /* Face 4 */

            /* Vert 3 */ /* Position */  1.0f, -1.0f, -1.0f, /* UV */ 1.f, 0.f,
            /* Vert 2 */ /* Position */  1.0f,  1.0f, -1.0f, /* UV */ 1.f, 1.f,
            /* Vert 6 */ /* Position */  1.0f,  1.0f,  1.0f, /* UV */ 1.f, 0.f,
            /* Vert 3 */ /* Position */  1.0f, -1.0f, -1.0f, /* UV */ 1.f, 0.f,
            /* Vert 6 */ /* Position */  1.0f,  1.0f,  1.0f, /* UV */ 1.f, 0.f,
            /* Vert 7 */ /* Position */  1.0f, -1.0f,  1.0f, /* UV */ 1.f, 1.f,

            /* Face 5 */

            /* Vert 1 */ /* Position */ -1.0f,  1.0f, -1.0f, /* UV */ 0.f, 1.f,
            /* Vert 5 */ /* Position */ -1.0f,  1.0f,  1.0f, /* UV */ 0.f, 0.f,
            /* Vert 6 */ /* Position */  1.0f,  1.0f,  1.0f, /* UV */ 1.f, 0.f,
            /* Vert 1 */ /* Position */ -1.0f,  1.0f, -1.0f, /* UV */ 0.f, 1.f,
            /* Vert 6 */ /* Position */  1.0f,  1.0f,  1.0f, /* UV */ 1.f, 0.f,
            /* Vert 2 */ /* Position */  1.0f,  1.0f, -1.0f, /* UV */ 1.f, 1.f,

            /* Face 6 */

            /* Vert 4 */ /* Position */ -1.0f, -1.0f,  1.0f, /* UV */ 0.f, 1.f,
            /* Vert 0 */ /* Position */ -1.0f, -1.0f, -1.0f, /* UV */ 0.f, 0.f,
            /* Vert 3 */ /* Position */  1.0f, -1.0f, -1.0f, /* UV */ 1.f, 0.f,
            /* Vert 4 */ /* Position */ -1.0f, -1.0f,  1.0f, /* UV */ 0.f, 1.f,
            /* Vert 3 */ /* Position */  1.0f, -1.0f, -1.0f, /* UV */ 1.f, 0.f,
            /* Vert 7 */ /* Position */  1.0f, -1.0f,  1.0f, /* UV */ 1.f, 1.f,
        };
    }

    constexpr std::vector<std::uint32_t> GenerateCubeMeshIndices()
    {
        return {
            /* Face 1 */ 0, 1, 2, 0, 2, 3,
            /* Face 2 */ 4, 6, 5, 4, 7, 6,
            /* Face 3 */ 4, 5, 1, 4, 1, 0,
            /* Face 4 */ 3, 2, 6, 3, 6, 7,
            /* Face 5 */ 1, 5, 6, 1, 6, 2,
            /* Face 6 */ 4, 0, 3, 4, 3, 7
        };
    }

}
