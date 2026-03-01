#ifndef SRC_HUD_RENDER_H_
#define SRC_HUD_RENDER_H_

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "../graphics/GfxContext.h"

class Camera;
class Assets;
class Player;
class Engine;
class Block;
class InventoryView;
class LevelFrontend;

namespace gui {
    class GUI;
    class UINode;
}

class HudRenderer {
private:
	Camera* uicamera;
    Assets* assets;

    std::unique_ptr<InventoryView> contentAccess;
    std::unique_ptr<InventoryView> hotbarView;

    int fps = 0;
    int fpsMin = 60;
    int fpsMax = 60;
    std::wstring fpsString;

    bool inventoryOpen = false;
    bool pause = false;

    std::shared_ptr<gui::UINode> debugPanel;
	gui::GUI* guiController;
    LevelFrontend* levelFrontend;

    void createDebugPanel(Engine* engine);
public:
	HudRenderer(Engine* engine, LevelFrontend* levelFrontend);
	~HudRenderer();

    void update(bool hudVisible);
    void drawOverlay(const GfxContext& context);
	void draw(const GfxContext& context);
	void drawDebug(int fps);

    bool isInventoryOpen() const;
	bool isPause() const;
};

#endif // SRC_HUD_RENDER_H_
