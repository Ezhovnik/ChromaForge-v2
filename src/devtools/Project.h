#pragma once

#include <string>
#include <vector>
#include <memory>

#include <interfaces/Serializable.h>

namespace scripting {
    class IProjectScript;
}

struct Project : Serializable {
    std::string name;
    std::string title;
    std::vector<std::string> basePacks;
    std::unique_ptr<scripting::IProjectScript> script;

    ~Project();

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
