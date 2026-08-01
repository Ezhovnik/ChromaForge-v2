#pragma once

#include <memory>

#include <assets/Assets.h>

class Task;
class AssetsLoader;
class Engine;
class Content;

class AssetsManagement final {
public:
    AssetsManagement(Engine& engine);
    ~AssetsManagement();

    void loadAssets(Content* content);
    void update();

    Assets* getStorage();
    const Assets* getStorage() const;

    AssetsLoader& acquireBackgroundLoader();
private:
    void finishBackgroundLoader();

    Engine& engine;
    std::unique_ptr<Assets> assets;
    std::unique_ptr<AssetsLoader> backgroundLoader;
    std::shared_ptr<Task> backgroundLoaderTask;
};
