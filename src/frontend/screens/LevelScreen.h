#ifndef FRONTEND_SCREENS_LEVEL_SCREEN_H_
#define FRONTEND_SCREENS_LEVEL_SCREEN_H_

#include <memory>

#include <frontend/screens/Screen.h>

class Engine;
class LevelFrontend;
class Hud;
class LevelController;
class WorldRenderer;
class TextureAnimator;
class Level;
class PostProcessing;
class ContentPackRuntime;

class LevelScreen : public Screen {
private:
    std::unique_ptr<LevelFrontend> frontend;
    std::unique_ptr<LevelController> controller;
    std::unique_ptr<WorldRenderer> worldRenderer;
    std::unique_ptr<TextureAnimator> animator;
    std::unique_ptr<PostProcessing> postProcessing;
    std::unique_ptr<Hud> hud;

    void saveWorldPreview();

    bool hudVisible = true;
    void updateHotkeys();

    void initializeContent();
    void initializePack(ContentPackRuntime* pack);
public:
    LevelScreen(Engine* engine, std::unique_ptr<Level> level);
    ~LevelScreen();

    void update(float deltaTime) override;
    void draw(float deltaTime) override;

    void onEngineShutdown() override;

    LevelController* getLevelController() const;
};

#endif // FRONTEND_SCREENS_LEVEL_SCREEN_H_
