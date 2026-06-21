#include <io/path.h>

#include <stdexcept>

using namespace io;

void path::checkValid() const {
    if (colonPos == std::string::npos) {
        throw std::runtime_error("path entry point is not specified: " + str);
    }
}
