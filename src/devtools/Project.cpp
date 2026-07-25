#include <devtools/Project.h>

#include <data/dv_util.h>
#include <logic/scripting/scripting.h>

Project::~Project() = default;

dv::value Project::serialize() const {
    return dv::object({
        {"name", name},
        {"title", title},
        {"base_packs", dv::to_value(basePacks)},
    });
}

void Project::deserialize(const dv::value& src) {
    src.at("name").get(name);
    src.at("title").get(title);
    dv::get(src, "base_packs", basePacks);
}
