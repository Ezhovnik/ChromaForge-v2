#ifndef FRONTEND_LEVEL_FRONTEND_H_
#define FRONTEND_LEVEL_FRONTEND_H_

#include <memory>

class Level;
class Assets;
class ContentGfxCache;
class Atlas;
class LevelController;

class LevelFrontend {
private:
    Level* level;
    LevelController* controller;
    Assets* assets;
    std::unique_ptr<ContentGfxCache> contentCache;
    std::unique_ptr<Atlas> blocksAtlas;
public:
    LevelFrontend(LevelController* controller, Assets* assets);
    ~LevelFrontend();

    Level* getLevel() const;
    Assets* getAssets() const;
    Atlas* getBlocksAtlas() const;
    ContentGfxCache* getContentGfxCache() const;
    LevelController* getController() const;
};


#endif // FRONTEND_LEVEL_FRONTEND_H_
