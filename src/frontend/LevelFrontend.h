#pragma once

#include <memory>

class Level;
class Assets;
class ContentGfxCache;
class LevelController;
class Player;

class LevelFrontend {
private:
    Level* level;
    LevelController* controller;
    Assets* assets;
    std::unique_ptr<ContentGfxCache> contentCache;
public:
    LevelFrontend(Player* currentPlayer, LevelController* controller, Assets* assets);
    ~LevelFrontend();

    Level* getLevel() const;
    Assets* getAssets() const;
    ContentGfxCache* getContentGfxCache() const;
    LevelController* getController() const;
};
