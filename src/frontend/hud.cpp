#include "hud.h"

#include <sstream>
#include <assert.h>
#include <memory>

#include "../assets/Assets.h"
#include "../graphics/core/ShaderProgram.h"
#include "../graphics/core/Batch2D.h"
#include "../graphics/core/Font.h"
#include "../graphics/core/Mesh.h"
#include "../graphics/core/Atlas.h"
#include "../window/Camera.h"
#include "../window/Window.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../voxels/Chunks.h"
#include "../voxels/Block.h"
#include "../world/World.h"
#include "../world/Level.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../debug/Logger.h"
#include "../graphics/ui/elements/layout/Menu.h"
#include "../graphics/ui/elements/layout/Panel.h"
#include "../graphics/ui/elements/display/Plotter.h"
#include "../graphics/ui/elements/UINode.h"
#include "../graphics/ui/GUI.h"
#include "../engine.h"
#include "../input_bindings.h"
#include "../window/input.h"
#include "menu.h"
#include "../content/Content.h"
#include "../math/voxmaths.h"
#include "ContentGfxCache.h"
#include "../graphics/render/WorldRenderer.h"
#include "../util/timeutil.h"
#include "../util/stringutil.h"
#include "../graphics/core/Batch3D.h"
#include "BlocksPreview.h"
#include "InventoryView.h"
#include "LevelFrontend.h"
#include "../items/Item.h"
#include "../items/Inventory.h"
#include "UIDocument.h"
#include "../logic/scripting/scripting.h"
#include "../delegates.h"
#include "../graphics/core/Texture.h"
#include "../items/Inventories.h"
#include "../voxels/Chunk.h"

extern std::shared_ptr<gui::UINode> create_debug_panel(
    Engine* engine, 
    Level* level, 
    Player* player
);

HudElement::HudElement(
    HudElementMode mode, 
    UIDocument* document, 
    std::shared_ptr<gui::UINode> node, 
    bool debug
) : mode(mode), 
	document(document), 
	node(node), 
	debug(debug)
{
}

void HudElement::update(bool pause, bool inventoryOpen, bool debugMode) {
    if (debug && !debugMode) {
        node->setVisible(false);
        return;
    }

    switch (mode) {
        case HudElementMode::Permanent:
            node->setVisible(true);
            break;
        case HudElementMode::InGame:
            node->setVisible(!pause && !inventoryOpen);
            break;
        case HudElementMode::InventoryAny:
            node->setVisible(inventoryOpen);
            break;
        case HudElementMode::InventoryBound:
            removed = !inventoryOpen;
            break;
    }
}

UIDocument* HudElement::getDocument() const {
    return document;
}

std::shared_ptr<gui::UINode> HudElement::getNode() const {
    return node;
}

std::shared_ptr<InventoryView> Hud::createContentAccess() {
    auto level = levelFrontend->getLevel();
    auto content = level->content;
    auto indices = content->getIndices();
    auto inventory = player->getInventory();

    int itemsCount = indices->countItemDefs();
    auto accessInventory = std::make_shared<Inventory>(0, itemsCount);
    for (int id = 1; id < itemsCount; ++id) {
        accessInventory->getSlot(id - 1).set(ItemStack(id, 1));
    }

    SlotLayout slotLayout(-1, glm::vec2(), false, true, nullptr,
    [=](uint, ItemStack& item) {
        auto copy = ItemStack(item);
        inventory->move(copy, indices);
    }, 
    [=](uint, ItemStack& item) {
        inventory->getSlot(player->getChosenSlot()).set(item);
    });

    InventoryBuilder builder;
    builder.addGrid(8, itemsCount - 1, glm::vec2(), 8, true, slotLayout);
    auto view = builder.build();
    view->bind(accessInventory, levelFrontend, interaction.get());
	view->setMargin(glm::vec4());
    return view;
}

std::shared_ptr<InventoryView> Hud::createHotbar() {
    auto inventory = player->getInventory();

    SlotLayout slotLayout(-1, glm::vec2(), false, false, nullptr, nullptr, nullptr);
    InventoryBuilder builder;
    builder.addGrid(10, 10, glm::vec2(), 4, true, slotLayout);
    auto view = builder.build();
    view->setOrigin(glm::vec2(view->getSize().x / 2, 0));
    view->bind(inventory, levelFrontend, interaction.get());
	view->setInteractive(false);
    return view;
}

Hud::Hud(Engine* engine, LevelFrontend* levelFrontend, Player* player) : assets(engine->getAssets()), guiController(engine->getGUI()), levelFrontend(levelFrontend), player(player) {
    interaction = std::make_unique<InventoryInteraction>();
    grabbedItemView = std::make_shared<SlotView>(SlotLayout(-1, glm::vec2(), false, false, nullptr, nullptr, nullptr));
	grabbedItemView->bind(0, interaction->getGrabbedItem(), levelFrontend, interaction.get());
    grabbedItemView->setColor(glm::vec4());
    grabbedItemView->setInteractive(false);
	grabbedItemView->setZIndex(1);

    contentAccess = createContentAccess();
    contentAccessPanel = std::make_shared<gui::Panel>(contentAccess->getSize(), glm::vec4(0.0f), 0.0f);
    contentAccessPanel->setColor(glm::vec4());
    contentAccessPanel->add(contentAccess);
    contentAccessPanel->setScrollable(true);

    hotbarView = createHotbar();

	darkOverlay = std::make_unique<gui::Panel>(glm::vec2(4000.0f));
    darkOverlay->setColor(glm::vec4(0, 0, 0, 0.5f));
	darkOverlay->setZIndex(-1);
	darkOverlay->setVisible(false);

	uicamera = std::make_unique<Camera>(glm::vec3(), 1);
	uicamera->perspective = false;
	uicamera->flipped = true;

    debugPanel = create_debug_panel(engine, levelFrontend->getLevel(), player);
	debugPanel->setZIndex(2);

	guiController->add(darkOverlay);
    guiController->add(hotbarView);
	guiController->add(debugPanel);
    guiController->add(contentAccessPanel);
    guiController->add(grabbedItemView);

    auto dplotter = std::make_shared<gui::Plotter>(350, 250, 2000, 16);
    dplotter->setGravity(gui::Gravity::bottom_right);
    add(HudElement(HudElementMode::Permanent, nullptr, dplotter, true));
}

Hud::~Hud() {
    guiController->remove(grabbedItemView);
    for (auto& element : elements) {
        onRemove(element);
    }
    guiController->remove(hotbarView);
	guiController->remove(darkOverlay);
    guiController->remove(contentAccessPanel);
	guiController->remove(debugPanel);
}

void Hud::cleanup() {
    auto it = std::remove_if(elements.begin(), elements.end(), [](const HudElement& e) {
        return e.isRemoved();
    });
    elements.erase(it, elements.end());
}

void Hud::processInput(bool visible) {
    if (Events::justPressed(keycode::ESCAPE)) {
        if (pause) {
            setPause(false);
        } else if (inventoryOpen) {
            closeInventory();
        } else {
            setPause(true);
        }
    }

    if (!pause && visible && Events::justActive(BIND_HUD_INVENTORY)) {
        if (inventoryOpen) {
            closeInventory();
        } else {
            openInventory();
        }
    }

    if (!pause) {
        if (!inventoryOpen && Events::scroll) {
            int slot = player->getChosenSlot();
            slot = (slot - Events::scroll) % 10;
            if (slot < 0) slot += 10;
            player->setChosenSlot(slot);
        }
        for (
            int i = static_cast<int>(keycode::NUM_1); 
            i <= static_cast<int>(keycode::NUM_9); 
            i++
        ) {
            if (Events::justPressed(i)) {
                player->setChosenSlot(i - static_cast<int>(keycode::NUM_1));
            }
        }
        if (Events::justPressed(keycode::NUM_0)) {
            player->setChosenSlot(9);
        }
    }
}

void Hud::update(bool hudVisible) {
	auto level = levelFrontend->getLevel();
	auto menu = guiController->getMenu();

	debugPanel->setVisible(player->debug && hudVisible);

	if (!hudVisible && inventoryOpen) closeInventory();
	if (pause && menu->getCurrent().panel == nullptr) setPause(false);

	if (!guiController->isFocusCaught()) processInput(hudVisible);

	if ((pause || inventoryOpen) == Events::_cursor_locked) Events::toggleCursor();

	if (blockUI) {
        voxel* vox = level->chunks->getVoxel(blockPos.x, blockPos.y, blockPos.z);
        if (vox == nullptr || vox->id != currentblockid) closeInventory();
    }

	for (auto& element : elements) {
        element.getNode()->setVisible(hudVisible);
    }

	glm::vec2 invSize = contentAccessPanel->getSize();
    contentAccessPanel->setVisible(inventoryView != nullptr);
    contentAccessPanel->setSize(glm::vec2(invSize.x, Window::height));
    contentAccess->setMinSize(glm::vec2(1, Window::height));
	hotbarView->setVisible(hudVisible);

	if (hudVisible) {
        for (auto& element : elements) {
            element.update(pause, inventoryOpen, player->debug);
            if (element.isRemoved()) onRemove(element);
        }
    }

    cleanup();
}

void Hud::draw(const GfxContext& context) {
    const Viewport& viewport = context.getViewport();
	const uint width = viewport.getWidth();
	const uint height = viewport.getHeight();

    updateElementsPosition(viewport);

    uicamera->setFov(height);

	auto batch = context.getBatch2D();
	batch->begin();

	ShaderProgram* uiShader = assets->getShader("ui");
	uiShader->use();
	uiShader->uniformMatrix("u_projview", uicamera->getProjView());

	if (!pause && !inventoryOpen && !player->debug) {
		GfxContext crosshair_context = context.sub();
        crosshair_context.setBlendMode(BlendMode::Inversion);
        auto texture = assets->getTexture("gui/crosshair");
        batch->texture(texture);
        int chsizex = texture != nullptr ? texture->getWidth() : 16;
        int chsizey = texture != nullptr ? texture->getHeight(): 16;
        batch->rect((width - chsizex) / 2, (height - chsizey) / 2, chsizex, chsizey, 0, 0, 1, 1, 1, 1, 1, 1);
	}
}

void Hud::updateElementsPosition(const Viewport& viewport) {
    const uint width = viewport.getWidth();
    const uint height = viewport.getHeight();

	if (inventoryOpen) {
		float caWidth = inventoryView ? contentAccess->getSize().x : 0.0f;
        contentAccessPanel->setPos(glm::vec2(width-caWidth, 0));

		glm::vec2 invSize = inventoryView ? inventoryView->getSize() : glm::vec2();
        if (secondUI == nullptr && inventoryView) {
            inventoryView->setPos(glm::vec2(
                glm::min(width / 2 - invSize.x / 2, width - caWidth - 10 - invSize.x),
                height / 2 - invSize.y / 2
            ));
        } else {
            glm::vec2 secondUISize = secondUI->getSize();
            float invwidth = glm::max(invSize.x, secondUISize.x);
            int interval = invSize.y > 0.0 ? 5 : 0;
            float totalHeight = invSize.y + secondUISize.y + interval;
            if (inventoryView) {
                inventoryView->setPos(glm::vec2(
                    glm::min(width / 2 - invwidth / 2, width - caWidth - 10 - invwidth),
                    height / 2 + totalHeight / 2 - invSize.y
                ));
            }
            secondUI->setPos(glm::vec2(
                glm::min(width / 2 - invwidth / 2, width - caWidth - 10 - invwidth),
                height / 2 - totalHeight / 2
            ));
        }
	}
	grabbedItemView->setPos(glm::vec2(Events::cursor));

	hotbarView->setPos(glm::vec2(width / 2, height - 65));
    hotbarView->setSelected(player->getChosenSlot());
}

void Hud::showOverlay(UIDocument* doc, bool playerInventory) {
    if (isInventoryOpen()) closeInventory();

    secondUI = doc->getRoot();
    if (playerInventory) openInventory();
    else inventoryOpen = true;
    add(HudElement(HudElementMode::InventoryBound, doc, secondUI, false));
}

void Hud::openInventory(glm::ivec3 block, UIDocument* doc, std::shared_ptr<Inventory> blockinv, bool playerInventory) {
	if (isInventoryOpen()) closeInventory();

    auto level = levelFrontend->getLevel();
    blockUI = std::dynamic_pointer_cast<InventoryView>(doc->getRoot());
    if (blockUI == nullptr) {
		LOG_ERROR("Block UI root element must be 'inventory'");
        throw std::runtime_error("Block UI root element must be 'inventory'");
    }

    secondUI = blockUI;

    if (playerInventory) openInventory();
    else inventoryOpen = true;

    if (blockinv == nullptr) blockinv = level->inventories->createVirtual(blockUI->getSlotsCount());
    level->chunks->getChunkByVoxel(block.x, block.y, block.z)->setUnsaved(true);
    blockUI->bind(blockinv, levelFrontend, interaction.get());
	blockPos = block;
	currentblockid = level->chunks->getVoxel(block.x, block.y, block.z)->id;
    add(HudElement(HudElementMode::InventoryBound, doc, blockUI, false));
}

void Hud::openInventory() {
    inventoryOpen = true;

    auto inventory = player->getInventory();
    auto inventoryDocument = assets->getLayout(BUILTIN_CONTENT_NAMESPACE + ":inventory");
    inventoryView = std::dynamic_pointer_cast<InventoryView>(inventoryDocument->getRoot());
    inventoryView->bind(inventory, levelFrontend, interaction.get());
    add(HudElement(HudElementMode::InventoryBound, inventoryDocument, inventoryView, false));
}

void Hud::closeInventory() {
    inventoryOpen = false;
    ItemStack& grabbed = interaction->getGrabbedItem();
    grabbed.clear();
    inventoryView = nullptr;

	blockUI = nullptr;
    secondUI = nullptr;
}

void Hud::openPermanent(UIDocument* doc) {
    auto root = doc->getRoot();
    remove(root);

    auto invview = std::dynamic_pointer_cast<InventoryView>(root);
    if (invview) {
        auto inventory = player->getInventory();
        invview->bind(inventory, levelFrontend, interaction.get());
    }
    add(HudElement(HudElementMode::Permanent, doc, doc->getRoot(), false));
}

bool Hud::isInventoryOpen() const {
	return inventoryOpen;
}

bool Hud::isPause() const {
	return pause;
}

void Hud::setPause(bool pause) {
    if (this->pause == pause) return;
    this->pause = pause;

    if (inventoryOpen) closeInventory();

    auto menu = guiController->getMenu();
    if (pause) {
        menu->setPage("pause");
    } else {
        menu->reset();
    }

    darkOverlay->setVisible(pause);
    menu->setVisible(pause);
}

void Hud::add(HudElement element) {
    guiController->add(element.getNode());
    auto invview = std::dynamic_pointer_cast<InventoryView>(element.getNode());
    auto document = element.getDocument();
    if (document) {
        auto inventory = invview ? invview->getInventory() : nullptr;
        std::vector<std::unique_ptr<dynamic::Value>> args;
        args.push_back(dynamic::Value::of(inventory ? inventory.get()->getId() : 0));
        for (int i = 0; i < 3; ++i) {
            args.push_back(dynamic::Value::of(blockPos[i]));
        }
        if (invview) {
            scripting::on_ui_open(
                element.getDocument(),
                std::move(args)
            );
        } else {
            scripting::on_ui_open(
                element.getDocument(),
                std::move(args)
            );
        }
    }
    elements.push_back(element);
}

void Hud::onRemove(HudElement& element) {
    auto document = element.getDocument();
    if (document) {
        Inventory* inventory = nullptr;
        auto invview = std::dynamic_pointer_cast<InventoryView>(element.getNode());
        if (invview) inventory = invview->getInventory().get();
        scripting::on_ui_close(document, inventory);
        if (invview) invview->unbind();
    }
    guiController->remove(element.getNode());
}

void Hud::remove(std::shared_ptr<gui::UINode> node) {
    for (auto& element : elements) {
        if (element.getNode() == node) {
            element.setRemoved();
            onRemove(element);
        }
    }
    cleanup();
}

Player* Hud::getPlayer() const {
    return player;
}

std::shared_ptr<Inventory> Hud::getBlockInventory() {
    if (blockUI == nullptr) return nullptr;
    return blockUI->getInventory();
}
