#include <devtools/Project.h>

#include <data/dv_util.h>
#include <logic/scripting/scripting.h>
#include <debug/Logger.h>
#include <io/io.h>
#include <io/path.h>

Project::~Project() = default;

dv::value Project::serialize() const {
    auto permissionsList = dv::list();
    for (const auto& perm : permissions.permissions) {
        permissionsList.add(perm);
    }
    return dv::object({
        {"name", name},
        {"title", title},
        {"base_packs", dv::to_value(basePacks)},
        {"permissions", std::move(permissionsList)}
    });
}

void Project::deserialize(const dv::value& src) {
    src.at("name").get(name);
    src.at("title").get(title);
    dv::get(src, "base_packs", basePacks);

    if (src.has("permissions")) {
        std::vector<std::string> perms;
        dv::get(src, "permissions", perms);
        permissions.permissions = std::set<std::string>(perms.begin(), perms.end());
    }
    LOG_INFO("Project's permissions:");
    for (const auto& perm : permissions.permissions) {
        LOG_INFO(" - {}", perm);
    }
}

void Project::loadProjectClientScript() {
    io::path scriptFile = "project:project_client.lua";
    if (io::exists(scriptFile)) {
        LOG_INFO("Starting project client script");
        clientScript = scripting::load_client_project_script(scriptFile);
    } else {
        LOG_WARN("Project client script does not exists");
    }
}

void Project::loadProjectStartScript() {
    io::path scriptFile = "project:start.lua";
    if (io::exists(scriptFile)) {
        LOG_INFO("Starting project start script");
        setupCoroutine = scripting::start_app_script(scriptFile);
    } else {
        LOG_WARN("Project start script does not exists");
    }
}

bool Permissions::has(const std::string& name) const {
    return permissions.find(name) != permissions.end();
}
