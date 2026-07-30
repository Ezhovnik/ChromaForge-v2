#pragma once

#include <memory>

#include <typedefs.h>
#include <util/span.h>

namespace compression {
    enum class Method {
        None,
        Extrle8,
        Extrle16,
        Zip
    };

    std::unique_ptr<ubyte[]> compress(
        const ubyte* src, size_t srclen, size_t& len, Method method
    );

    std::unique_ptr<ubyte[]> decompress(
        const ubyte* src, size_t srclen, size_t dstlen, Method method
    );

    void decompress(
        const util::span<ubyte> src, ubyte* dst, size_t dstlen, Method method
    );
}
