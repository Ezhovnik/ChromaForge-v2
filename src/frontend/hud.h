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
class SlotView;
class InventoryInteraction;

namespace gui {
    class GUI;
    class UINode;
    class Panel;
}

class HudRenderer {
private:
	std::unique_ptr<Camera> uicamera;
    Assets* assets;

    std::shared_ptr<gui::Panel> contentAccessPanel;
    std::shared_ptr<InventoryView> contentAccess;
    std::shared_ptr<InventoryView> hotbarView;
    std::shared_ptr<InventoryView> inventoryView;

    int fps = 0;
    int fpsMin = 60;
    int fpsMax = 60;
    std::wstring fpsString;

    bool inventoryOpen = false;
    bool pause = false;

    std::shared_ptr<gui::UINode> debugPanel;
    std::unique_ptr<InventoryInteraction> interaction;
    std::shared_ptr<SlotView> grabbedItemView;
    std::shared_ptr<gui::Panel> darkOverlay;

	gui::GUI* guiController;
    LevelFrontend* levelFrontend;

    std::shared_ptr<gui::UINode> createDebugPanel(Engine* engine);
    std::shared_ptr<InventoryView> createContentAccess();
    std::shared_ptr<InventoryView> createHotbar();
    std::shared_ptr<InventoryView> createInventory();
public:
	HudRenderer(Engine* engine, LevelFrontend* levelFrontend);
	~HudRenderer();

    void update(bool hudVisible);
	void draw(const GfxContext& context);
	void drawDebug(int fps);

    bool isInventoryOpen() const;
	bool isPause() const;

    void closeInventory();
};

#endif // SRC_HUD_RENDER_H_
