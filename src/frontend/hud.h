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
class UIDocument;
class Inventory;

namespace gui {
    class GUI;
    class UINode;
    class Panel;
    class Container;
}

enum class HudElementMode {
    InGame, // Элемент скрыт, если открыто меню или инвентарь
    Permanent, // Элемент виден, если виден hud
    InventoryAny, // Элемент виден при открытом инвенторе
    InventoryBound // Элемент будет удален при закрытии инвентаря
};

class HudElement {
	HudElementMode mode;
	UIDocument* document;
	std::shared_ptr<gui::UINode> node;

	bool debug;
	bool removed = false;
public:
	HudElement(HudElementMode mode, UIDocument* document, std::shared_ptr<gui::UINode> node, bool debug);

	void update(bool pause, bool inventoryOpen, bool debug);

	UIDocument* getDocument() const;
	std::shared_ptr<gui::UINode> getNode() const;

	bool isRemoved() const {
		return removed;
	}

    void setRemoved() {
        removed = true;
    }
};

class Hud {
private:
	std::unique_ptr<Camera> uicamera;
    Assets* assets;
    Engine* engine;

    std::shared_ptr<gui::Container> contentAccessPanel;
    std::shared_ptr<InventoryView> contentAccess;
    std::shared_ptr<InventoryView> hotbarView;

    std::shared_ptr<InventoryView> inventoryView = nullptr;
    std::shared_ptr<InventoryView> blockUI = nullptr;

	glm::ivec3 currentblock {};
    blockid_t currentblockid = 0;

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

    Player* player;

    std::vector<HudElement> elements;

    std::shared_ptr<gui::UINode> createDebugPanel(Engine* engine);
    std::shared_ptr<InventoryView> createContentAccess();
    std::shared_ptr<InventoryView> createHotbar();

    void cleanup();
public:
	Hud(Engine* engine, LevelFrontend* levelFrontend, Player* player);
	~Hud();

    void update(bool hudVisible);
	void draw(const GfxContext& context);
	void drawDebug(int fps);

	bool isPause() const;
    void setPause(bool pause);

    void openInventory();
    void openInventory(glm::ivec3 block, UIDocument* doc, std::shared_ptr<Inventory> blockInv, bool playerInventory);
    void closeInventory();
    bool isInventoryOpen() const;
    void openPermanent(UIDocument* doc);

    void add(HudElement element);
	void remove(HudElement& element);
    void remove(std::shared_ptr<gui::UINode> node);

    Player* getPlayer() const;
};

#endif // SRC_HUD_RENDER_H_
