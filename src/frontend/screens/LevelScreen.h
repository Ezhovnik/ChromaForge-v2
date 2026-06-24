#pragma once

#include <memory>

#include <frontend/screens/Screen.h>
#include <typedefs.h>

class Engine;
class LevelFrontend;
class Hud;
class LevelController;
class WorldRenderer;
class TextureAnimator;
class Level;
class PostProcessing;
class ContentPackRuntime;
class Decorator;
class PlayerController;
class World;

class LevelScreen : public Screen {
    World& world;
    std::unique_ptr<LevelFrontend> frontend;
    std::unique_ptr<LevelController> controller;
    std::unique_ptr<PlayerController> playerController;
    std::unique_ptr<WorldRenderer> renderer;
    std::unique_ptr<TextureAnimator> animator;
    std::unique_ptr<PostProcessing> postProcessing;
    std::unique_ptr<Decorator> decorator;
    std::unique_ptr<Hud> hud;

    void saveWorldPreview();

    bool hudVisible = true;
    bool debug = false;
    void updateHotkeys();

    void initializeContent();
    void initializePack(ContentPackRuntime* pack);

    void loadDecorations();
    void saveDecorations();
    void updateAudio();
public:
    LevelScreen(
        Engine& engine, std::unique_ptr<Level> level, int64_t localPlayer
    );
    ~LevelScreen();

    void update(float deltaTime) override;
    void draw(float deltaTime) override;

    void onEngineShutdown() override;
};
