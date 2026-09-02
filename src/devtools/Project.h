#pragma once

#include <string>
#include <vector>
#include <memory>
#include <set>

#include <interfaces/Serializable.h>
#include <io/path.h>

namespace scripting {
    class IClientProjectScript;
}

struct Permissions {
    static inline std::string DEBUGGING = "debugging";
    static inline std::string NETWORK = "network";
    static inline std::string RECORD_AUDIO = "record-audio";
    static inline std::string WRITE_TO_USER = "write-to-user";
    static inline std::string SUB_INSTANCES = "sub-instances";

    std::set<std::string> permissions;

    bool has(const std::string& name) const;
};

struct Project : Serializable {
    std::string name;
    std::string title;
    io::path path;
    std::vector<std::string> basePacks;
    Permissions permissions;

    ~Project();

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
