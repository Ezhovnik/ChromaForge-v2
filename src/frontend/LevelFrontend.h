#pragma once

#include <memory>

class Level;
class Assets;
class ContentGfxCache;
class LevelController;
class Player;
struct EngineSettings;
class Engine;

class LevelFrontend {
private:
    Level& level;
    LevelController* controller;
    Assets& assets;
    std::unique_ptr<ContentGfxCache> contentCache;
public:
    LevelFrontend(
        Engine& engine,
        Player* currentPlayer,
        LevelController* controller,
        const EngineSettings& settings
    );
    ~LevelFrontend();

    Level& getLevel();
    const Level& getLevel() const;
    const Assets& getAssets() const;
    const ContentGfxCache& getContentGfxCache() const;
    ContentGfxCache& getContentGfxCache();
    LevelController* getController() const;
};
