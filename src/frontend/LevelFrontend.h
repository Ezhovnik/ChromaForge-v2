#pragma once

#include <memory>

class Level;
class Assets;
class ContentGfxCache;
class LevelController;
class PlayerController;
struct EngineSettings;
class Engine;

class LevelFrontend {
private:
    Level& level;
    LevelController& controller;
    Assets& assets;
    std::unique_ptr<ContentGfxCache> contentCache;
public:
    LevelFrontend(
        Engine& engine,
        PlayerController& currentPlayer,
        LevelController& controller,
        const EngineSettings& settings
    );
    ~LevelFrontend();

    Level& getLevel();
    const ContentGfxCache& getContentGfxCache() const;
    ContentGfxCache& getContentGfxCache();
    LevelController& getController() const;
};
