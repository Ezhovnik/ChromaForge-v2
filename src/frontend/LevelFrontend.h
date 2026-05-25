#pragma once

#include <memory>

class Level;
class Assets;
class ContentGfxCache;
class LevelController;
class Player;

class LevelFrontend {
private:
    Level& level;
    LevelController* controller;
    const Assets& assets;
    std::unique_ptr<ContentGfxCache> contentCache;
public:
    LevelFrontend(Player* currentPlayer, LevelController* controller, Assets& assets);
    ~LevelFrontend();

    Level& getLevel();
    const Level& getLevel() const;
    const Assets& getAssets() const;
    const ContentGfxCache& getContentGfxCache() const;
    LevelController* getController() const;
};
