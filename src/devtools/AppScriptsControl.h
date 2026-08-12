#pragma once

#include <interfaces/Process.h>

#include <memory>
#include <string>

namespace scripting {
    class IClientProjectScript;
}

struct CoreParameters;
struct Project;

class AppScriptsControl {
public:
    AppScriptsControl(const CoreParameters& params, const Project& project);

    void spark();
    void loadProjectClientScript();
    void terminate(std::string_view reason);

    void onScreenChange(const std::string& name, bool show);
private:
    const Project& project;
    std::unique_ptr<scripting::IClientProjectScript> clientScript;
    std::unique_ptr<Process> scriptCoroutine;
};
