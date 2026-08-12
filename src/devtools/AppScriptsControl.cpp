#include <devtools/AppScriptsControl.h>

#include <debug/Logger.h>
#include <engine/CoreParameters.h>
#include <io/io.h>
#include <devtools/Project.h>
#include <logic/scripting/scripting.h>

static debug::Logger logger("app-scripts");

AppScriptsControl::AppScriptsControl(
    const CoreParameters& params, const Project& project
) : project(project) {
    io::path scriptFile =
        params.scriptFile.empty()
            ? "project:start.lua"
            : std::string("script:") + params.scriptFile.filename().u8string();
    if (io::exists(scriptFile)) {
        logger.info() << "Starting script: " << params.scriptFile.u8string();
        scriptCoroutine = scripting::start_app_script(scriptFile);
    } else {
        logger.warning() << "Script does not exists: " << params.scriptFile.u8string();
    }

    if (!params.headless) {
        loadProjectClientScript();
    }
}

void AppScriptsControl::loadProjectClientScript() {
    io::path scriptFile = "project:project_client.lua";
    if (io::exists(scriptFile)) {
        logger.info() << "Starting project client script: " << scriptFile.string();
        clientScript = scripting::load_client_project_script(scriptFile);
    } else {
        logger.warning() << "Project client script does not exists: " << scriptFile.string();
    }
}

void AppScriptsControl::onScreenChange(const std::string& name, bool show) {
    if (clientScript) {
        clientScript->onScreenChange(name, show);
    }
}

void AppScriptsControl::spark() {
    if (scriptCoroutine && scriptCoroutine->isActive()) {
        scriptCoroutine->update();
    }
}

void AppScriptsControl::terminate(std::string_view reason) {
    if (scriptCoroutine->isActive()) {
        scriptCoroutine->terminate();
        logger.info() << "Script has been terminated due to " << reason;
    }
}

bool AppScriptsControl::isFinished() const {
    return scriptCoroutine == nullptr || !scriptCoroutine->isActive();
}
