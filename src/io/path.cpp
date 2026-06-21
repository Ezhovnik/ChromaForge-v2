#include <io/path.h>

#include <stack>

#include <debug/Logger.h>

using namespace io;

void path::checkValid() const {
    if (colonPos == std::string::npos) {
        LOG_ERROR("Path entry point is not specified: {}", str);
        throw std::runtime_error("Path entry point is not specified: " + str);
    }
}

path path::normalized() const {
    io::path path = pathPart();

    std::stack<std::string> parts;
    do {
        parts.push(path.name());
        path.str = path.parent().string();
    } while (!path.empty());

    while (!parts.empty()) {
        const std::string part = parts.top();
        parts.pop();
        if (part == ".") continue;

        if (part == "..") {
            LOG_ERROR("Entry point reached");
            throw access_error("Entry point reached");
        }

        path = path / part;
    }
    if (path.colonPos != std::string::npos) {
        path = path.entryPoint() + ":" + path.string();
    }
    return path;
}
