#ifndef CODERS_ZIP_H_
#define CODERS_ZIP_H_

#include <vector>

#include "../typedefs.h"

namespace zip {
    const ubyte MAGIC[] = "\x1F\x8B";

    std::vector<ubyte> compress(const ubyte* src, size_t size);
    std::vector<ubyte> decompress(const ubyte* src, size_t size);
}

#endif // CODERS_ZIP_H_
