#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include <graphics/core/DrawContext.h>
#include <util/ObjectsKeeper.h>
#include <data/dv.h>

class Camera;
class Assets;
class Player;
class Engine;
class Block;
class LevelFrontend;
class UIDocument;
class Inventory;
class ImageData;

namespace gui {
    class GUI;
    class UINode;
    class Panel;
    class Container;
    class SlotView;
    class InventoryView;
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

    bool isInventoryBound() const {
        return mode == HudElementMode::InventoryBound;
    }

    void setRemoved() {
        removed = true;
    }
};

class Hud : public util::ObjectsKeeper {
private:
    Engine* engine;
    Assets* assets;
    std::unique_ptr<Camera> uicamera;
    gui::GUI* guiController;
    LevelFrontend& levelFrontend;
    Player* player;

    std::shared_ptr<gui::Container> contentAccessPanel;
    std::shared_ptr<gui::InventoryView> contentAccess;
    std::shared_ptr<gui::InventoryView> hotbarView;

    std::shared_ptr<gui::InventoryView> inventoryView = nullptr;
    std::shared_ptr<gui::InventoryView> blockUI = nullptr;
    std::shared_ptr<gui::InventoryView> secondInvView = nullptr;

	glm::ivec3 blockPos {};
    blockid_t currentblockid = 0;

    bool showContentPanel = true;

    bool allowDebugCheats = true;

    bool inventoryOpen = false;
    bool pause = false;

    std::shared_ptr<gui::UINode> debugPanel;
    std::shared_ptr<gui::SlotView> exchangeSlot;
    std::shared_ptr<Inventory> exchangeSlotInv = nullptr;
    std::shared_ptr<gui::UINode> darkOverlay;
    std::shared_ptr<gui::UINode> secondUI = nullptr;
    std::shared_ptr<gui::UINode> debugMinimap = nullptr;

    std::unique_ptr<ImageData> debugImgWorldGen;

    std::vector<HudElement> elements;

    std::shared_ptr<gui::InventoryView> createContentAccess();
    std::shared_ptr<gui::InventoryView> createHotbar();

    void processInput(bool visible);
    void updateElementsPosition(const Viewport& viewport);
    void updateHotbarControl();
    void cleanup();

    void showExchangeSlot();
    void updateWorldGenDebugVisualization();
public:
	Hud(Engine* engine, LevelFrontend& levelFrontend, Player* player);
	~Hud();

    void update(bool hudVisible);
	void draw(const DrawContext& context);

	bool isPause() const;
    void setPause(bool pause);

    void openInventory();
    std::shared_ptr<Inventory> openInventory(
        UIDocument* doc,
        std::shared_ptr<Inventory> inv,
        bool playerInventory
    );
    void openInventory(
        glm::ivec3 block,
        UIDocument* doc,
        std::shared_ptr<Inventory> blockInv,
        bool playerInventory
    );
    void closeInventory();
    bool isInventoryOpen() const;
    void openPermanent(UIDocument* doc);
    void showOverlay(
        UIDocument* doc,
        bool playerInventory,
        const dv::value& arg=nullptr
    );
    bool isContentAccess() const;
    void setContentAccess(bool flag);
    void setDebugCheats(bool flag);

    void add(const HudElement& element, const dv::value& arg=nullptr);
	void onRemove(const HudElement& element);
    void remove(const std::shared_ptr<gui::UINode>& node);

    Player* getPlayer() const;

    std::shared_ptr<Inventory> getBlockInventory();

    static bool showGeneratorMinimap;

    inline static std::string DEBUG_WORLDGEN_IMAGE = "#debug.img.worldgen";
};
