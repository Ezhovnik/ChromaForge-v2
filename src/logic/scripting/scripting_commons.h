#pragma once

#include <string>

#include <io/fwd.h>

namespace scripting {
    void load_script(const io::path& name, bool throwable);

    [[nodiscard]]
    int load_script(
        int env,
        const std::string& type,
        const io::path& file,
        const std::string& fileName
    );
}
