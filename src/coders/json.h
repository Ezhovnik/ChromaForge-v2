#pragma once

#include <string>

#include <typedefs.h>
#include <coders/binary_json.h>
#include <data/dv.h>

namespace json {
    dv::value parse(std::string_view filename, std::string_view source);
    dv::value parse(std::string_view source);

    std::string stringify(
        const dv::value& value,
        bool nice,
        const std::string& indent="  "
    );
}
