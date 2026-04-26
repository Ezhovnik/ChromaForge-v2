#pragma once

#include <memory>

#include <typedefs.h>

namespace compression {
    enum class Method {
        None,
        Extrle8,
        Zip
    };

    std::unique_ptr<ubyte[]> compress(
        const ubyte* src, size_t srclen, size_t& len, Method method
    );

    std::unique_ptr<ubyte[]> decompress(
        const ubyte* src, size_t srclen, size_t dstlen, Method method
    );
}
