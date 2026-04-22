#pragma once

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
