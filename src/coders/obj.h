#ifndef CODERS_OBJ_H_
#define CODERS_OBJ_H_

#include <string>
#include <memory>

namespace model {
    struct Model;
}

namespace obj {
    std::unique_ptr<model::Model> parse(
        const std::string_view file, const std::string_view src
    );
}

#endif // CODERS_OBJ_H_
