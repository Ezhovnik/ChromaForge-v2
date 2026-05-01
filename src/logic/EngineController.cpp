#include <logic/EngineController.h>

#include <memory>
#include <filesystem>
#include <algorithm>

#include <content/ContentReport.h>
#include <debug/Logger.h>
#include <engine.h>
#include <files/WorldFiles.h>
#include <files/WorldConverter.h>
#include <frontend/locale/langs.h>
#include <frontend/screens/MenuScreen.h>
#include <frontend/screens/LevelScreen.h>
#include <graphics/ui/elements/display/Label.h>
#include <graphics/ui/elements/control/Button.h>
#include <graphics/ui/elements/layout/Panel.h>
#include <graphics/ui/elements/layout/Menu.h>
#include <graphics/ui/gui_util.h>
#include <interfaces/Task.h>
#include <util/stringutil.h>
#include <world/World.h>
#include <world/Level.h>
#include <logic/LevelController.h>
#include <debug/Logger.h>
#include <frontend/menu.h>
#include <coders/commons.h>
#include <settings.h>

EngineController::EngineController(Engine* engine) : engine(engine) {
}

void EngineController::deleteWorld(const std::string& name) {
    std::filesystem::path folder = engine->getPaths()->getWorldFolderByName(name);
    guiutil::confirm(engine->getGUI(), langs::get(L"delete-confirm", L"world") +
    L" (" + util::str2wstr_utf8(folder.u8string()) + L")", [=]() {
        LOG_INFO("Deleting {}", folder.u8string());
        std::filesystem::remove_all(folder);
    });
}

std::shared_ptr<Task> create_converter(
    Engine* engine,
    const std::shared_ptr<WorldFiles>& worldFiles, 
    const Content* content, 
    const std::shared_ptr<ContentReport>& report, 
    const runnable& postRunnable)
{
    return WorldConverter::startTask(worldFiles, content, report, [=](){
            auto menu = engine->getGUI()->getMenu();
            menu->reset();
            menu->setPage("main", false);
            engine->getGUI()->postRunnable([=]() {
                postRunnable();
            });
        },
        report->isUpgradeRequired(),
        true
    );
}

void show_convert_request(
    Engine* engine, 
    const Content* content, 
    const std::shared_ptr<ContentReport>& report,
    const std::shared_ptr<WorldFiles>& worldFiles,
    const runnable& postRunnable
) {
    guiutil::confirm(engine->getGUI(), langs::get(report->isUpgradeRequired() ? L"world.upgrade-request" : L"world.convert-request"), [=]() {
        auto converter = create_converter(engine, worldFiles, content, report, postRunnable);
        menus::show_process_panel(engine, converter, L"Converting world...");
    }, L"", langs::get(L"Cancel"));
}

static void show_content_missing(
    Engine* engine,
    const std::shared_ptr<ContentReport>& report
) {
    auto root = dv::object();
    auto& contentEntries = root.list("content");

    for (auto& entry : report->getMissingContent()) {
        std::string contentName = contenttype_name(entry.type);
        auto& contentEntry = contentEntries.object();
        contentEntry["type"] = contentName;
        contentEntry["name"] = entry.name;
    }

    menus::show(engine, "reports/missing_content", {std::move(root)});
}

static bool loadWorldContent(Engine* engine, const std::filesystem::path& folder) {
    return menus::call(engine, [engine, folder]() {
        engine->loadWorldContent(folder);
    });
}

static void loadWorld(Engine* engine, const std::shared_ptr<WorldFiles>& worldFiles) {
    try {
        auto content = engine->getContent();
        auto& packs = engine->getContentPacks();
        auto& settings = engine->getSettings();

        auto level = World::load(worldFiles, settings, content, packs);
        engine->setScreen(std::make_shared<LevelScreen>(engine, std::move(level)));
    } catch (const world_load_error& error) {
        guiutil::alert(
            engine->getGUI(), langs::get(L"Error") + L": " +
            util::str2wstr_utf8(error.what())
        );
        return;
    }
}

void EngineController::openWorld(const std::string& name, bool confirmConvert) {
    auto paths = engine->getPaths();
    auto folder = paths->getWorldsFolder()/std::filesystem::u8path(name);
    if (!loadWorldContent(engine, folder)) return;

    auto* content = engine->getContent();

    auto worldFiles = std::make_shared<WorldFiles>(
        folder, engine->getSettings().debug
    );
    if (auto report = World::checkIndices(worldFiles, content)) {
        if (report->hasMissingContent()) {
            engine->setScreen(std::make_shared<MenuScreen>(engine));
            show_content_missing(engine, report);
        } else {
            if (confirmConvert) {
                menus::show_process_panel(engine, create_converter(engine, worldFiles, content, report, [=]() {
                    openWorld(name, false);
                }), L"Converting world...");
            } else {
                show_convert_request(engine, content, report, std::move(worldFiles), [=](){
                    openWorld(name, false);
                });
            }
        }
        return;
    }
    loadWorld(engine, std::move(worldFiles));
}

inline uint64_t str2seed(const std::string& seedstr) {
    if (util::is_integer(seedstr)) {
        try {
            return std::stoull(seedstr);
        } catch (const std::out_of_range& err) {
            std::hash<std::string> hash;
            return hash(seedstr);
        }
    } else {
        std::hash<std::string> hash;
        return hash(seedstr);
    }
}

void EngineController::createWorld(
    const std::string& name, 
    const std::string& seedstr,
    const std::string& generatorID
) {
    uint64_t seed = str2seed(seedstr);

    EnginePaths* paths = engine->getPaths();
    auto folder = paths->getWorldsFolder()/std::filesystem::u8path(name);
    if (!menus::call(engine, [this, paths, folder]() {
        engine->loadContent();
        paths->setCurrentWorldFolder(folder);
    })) {
        return;
    }

    auto level = World::create(
        name, generatorID, folder, seed, 
        engine->getSettings(), 
        engine->getContent(),
        engine->getContentPacks()
    );
    engine->setScreen(std::make_shared<LevelScreen>(engine, std::move(level)));
}

void EngineController::reopenWorld(World* world) {
    std::string wname = world->wfile->getFolder().filename().u8string();
    engine->setScreen(nullptr);
    engine->setScreen(std::make_shared<MenuScreen>(engine));
    openWorld(wname, true);
}

void EngineController::reconfigPacks(
    LevelController* controller,
    const std::vector<std::string>& packsToAdd,
    const std::vector<std::string>& packsToRemove)
{
    auto content = engine->getContent();
    bool hasIndices = false;

    std::stringstream ss;
    for (const auto& id : packsToRemove) {
        auto runtime = content->getPackRuntime(id);
        if (runtime && runtime->getStats().hasSavingContent()) {
            if (hasIndices) ss << ", ";
            hasIndices = true;
            ss << id;
        }
    }

    runnable removeFunc = [=]() {
        if (controller == nullptr) {
            try {
                auto manager = engine->createPacksManager(std::filesystem::path(""));
                manager.scan();
                std::vector<std::string> names = PacksManager::getNames(engine->getContentPacks());
                for (const auto& id : packsToAdd) {
                    names.push_back(id);
                }
                for (const auto& id : packsToRemove) {
                    manager.exclude(id);
                    names.erase(std::find(names.begin(), names.end(), id));
                }
                names = manager.assembly(names);
                engine->getContentPacks() = manager.getAll(names);
            } catch (const contentpack_error& err) {
                std::string errorLog = std::string(err.what()) + " [" + err.getPackId() + "]";
                LOG_ERROR("{}", errorLog);
                throw std::runtime_error(errorLog);
            }
        } else {
            auto world = controller->getLevel()->getWorld();
            auto wfile = world->wfile.get();
            controller->saveWorld();
            auto manager = engine->createPacksManager(wfile->getFolder());
            manager.scan();

            auto names = PacksManager::getNames(world->getPacks());
            for (const auto& id : packsToAdd) {
                names.push_back(id);
            }
            for (const auto& id : packsToRemove) {
                manager.exclude(id);
                names.erase(std::find(names.begin(), names.end(), id));
            }
            wfile->removeIndices(packsToRemove);
            wfile->writePacks(manager.getAll(names));
            reopenWorld(world);
        }
    };

    if (hasIndices) {
        guiutil::confirm(
            engine->getGUI(), 
            langs::get(L"remove-confirm", L"pack") +
            L" (" + util::str2wstr_utf8(ss.str()) + L")", 
            [=]() {removeFunc();}
        );
    } else {
        removeFunc();
    }
}
