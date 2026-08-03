#include <engine/AssetsManagement.h>

#include <assets/Assets.h>
#include <assets/AssetsLoader.h>
#include <engine/Engine.h>
#include <debug/Logger.h>
#include <content/Content.h>
#include <graphics/core/ShaderProgram.h>
#include <engine/EnginePaths.h>
#include <coders/GLSLExtension.h>
#include <graphics/render/ModelsGenerator.h>
#include <graphics/ui/GUI.h>

static debug::Logger logger("assets-managment");

AssetsManagement::AssetsManagement(
    Engine& engine
) : engine(engine),
    settings(engine.getSettings()) {}

AssetsManagement::~AssetsManagement() {
    finishBackgroundLoader();
}

const Assets* AssetsManagement::getStorage() const {
    return assets.get();
}

Assets* AssetsManagement::getStorage() {
    return assets.get();
}

AssetsLoader& AssetsManagement::acquireBackgroundLoader() {
    if (backgroundLoader) return *backgroundLoader;

    if (assets == nullptr) {
        throw std::runtime_error("no assets storage available");
    }
    backgroundLoader = std::make_unique<AssetsLoader>(
        engine, *assets, engine.getResPaths()
    );
    backgroundLoaderTask = backgroundLoader->startTask(
        nullptr, settings.system.maxBgAssetLoaders.get()
    );
    return *backgroundLoader;
}

void AssetsManagement::loadAssets(Content* content) {
    finishBackgroundLoader();

    logger.info() << "Loading assets";
    const auto& paths = engine.getPaths();
    ShaderProgram::preprocessor->setPaths(&paths.resPaths);

    auto new_assets = std::make_unique<Assets>();
    AssetsLoader loader(engine, *new_assets, paths.resPaths);
    AssetsLoader::addDefaults(loader, content);

    bool threading = false;
    if (threading) {
        auto task = loader.startTask(
            [=]() {}, 0
        );
        task->waitForEnd();
    } else {
        while (loader.hasNext()) {
            loader.loadNext();
        }
    }
    assets = std::move(new_assets);
    if (content) {
        ModelsGenerator::prepare(*content, *assets);
    }
    assets->setup();
    engine.getGUI().onAssetsLoad(assets.get());
    logger.info() << "Assets loaded successfully";
}

void AssetsManagement::update() {
    if (backgroundLoaderTask) {
        backgroundLoaderTask->update();
    }
}

void AssetsManagement::finishBackgroundLoader() {
    if (backgroundLoaderTask == nullptr) return;

    backgroundLoaderTask.reset();
    backgroundLoader.reset();
}
