#pragma once

#include <string>
#include <vector>

#include <devtools/syntax.h>

namespace lua {
    std::vector<devtools::Token> tokenize(
        std::string_view file, std::wstring_view source
    );
}
