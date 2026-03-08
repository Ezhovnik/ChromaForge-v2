#include "hud.h"

#include <sstream>
#include <assert.h>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../assets/Assets.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Batch2D.h"
#include "../graphics/Font.h"
#include "../graphics/Mesh.h"
#include "../graphics/Atlas.h"
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
#include "../logger/Logger.h"
#include "gui/controls.h"
#include "gui/panels.h"
#include "gui/UINode.h"
#include "gui/GUI.h"
#include "../engine.h"
#include "screens.h"
#include "../core_defs.h"
#include "../window/input.h"
#include "menu.h"
#include "../content/Content.h"
#include "../math/voxmaths.h"
#include "ContentGfxCache.h"
#include "WorldRenderer.h"
#include "../util/timeutil.h"
#include "../util/stringutil.h"
#include "../graphics/Batch3D.h"
#include "BlocksPreview.h"
#include "InventoryView.h"
#include "LevelFrontend.h"
#include "../items/Item.h"
#include "../items/Inventory.h"

static std::shared_ptr<gui::Label> create_label(gui::wstringsupplier supplier) {
	std::shared_ptr<gui::Label> label = std::make_shared<gui::Label>(L"-");
	label->textSupplier(supplier);
	return label;
}

std::shared_ptr<gui::UINode> HudRenderer::createDebugPanel(Engine* engine) {
	auto level = levelFrontend->getLevel();

    auto panel = std::make_shared<gui::Panel>(glm::vec2(350, 200), glm::vec4(5.0f), 1.0f);
    panel->listenInterval(0.5f, [this]() {
		fpsString = std::to_wstring(fpsMax) + L" / "+std::to_wstring(fpsMin);
		fpsMin = fps;
		fpsMax = fps;
	});

	panel->setCoord(glm::vec2(10, 10));
	panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		return L"FPS: " + this->fpsString;
	})));
    panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		return L"Meshes: " + std::to_wstring(Mesh::meshesCount);
	})));
    panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		return L"Chunks: " + std::to_wstring(level->chunks->chunksCount) + L" (visible: " + std::to_wstring(level->chunks->visibleCount) + L")";
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		auto player = level->player;
		auto indices = level->content->getIndices();
		auto def = indices->getBlockDef(player->selectedVoxel.id);

		std::wstringstream stream;
		stream << std::hex << player->selectedVoxel.states;
		if (def) stream << L" (" << util::str2wstr_utf8(def->name) << L")";

		return L"Selected-block " + std::to_wstring(player->selectedVoxel.id) + L" " + stream.str();
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		return L"Seed: " + std::to_wstring(level->world->getSeed());
	})));
	for (int ax = 0; ax < 3; ++ax){
		auto sub = std::make_shared<gui::Panel>(glm::vec2(10, 27), glm::vec4(0.0f));
		sub->orientation(gui::Orientation::horizontal);
		auto label = std::make_shared<gui::Label>(std::wstring({static_cast<wchar_t>(L'x' + ax)}) + L": ");
		label->setMargin(glm::vec4(2, 3, 2, 3));
		sub->add(label);
		sub->setColor(glm::vec4(0.0f));

		auto box = std::make_shared<gui::TextBox>(L"");
		box->textSupplier([=]() {
			Hitbox* hitbox = level->player->hitbox.get();
			return util::double2wstr(hitbox->position[ax], 2);
		});
		box->textConsumer([=](std::wstring text) {
			try {
				glm::vec3 position = level->player->hitbox->position;
				position[ax] = std::stoi(text);
				level->player->teleport(position);
			} catch (std::out_of_range& _) {
			} catch (std::invalid_argument& _){
			}
		});
		box->setOnEditStart([=](){
			Hitbox* hitbox = level->player->hitbox.get();
			box->text(std::to_wstring(int(hitbox->position[ax])));
		});

		sub->add(box);
		panel->add(sub);
	}

	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		std::wstringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << WorldRenderer::skyClearness;
		return L"Sky clearness: " + ss.str();
	})));

	{
		auto bar = std::make_shared<gui::TrackBar>(0.0f, 1.0f, 0.0f, 0.005f, 8);
		bar->supplier([=]() {
			return WorldRenderer::skyClearness;
		});
		bar->consumer([=](double val) {
			WorldRenderer::skyClearness = val;
		});
		panel->add(bar);
	}

	panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		int hour, minute, second;
		timeutil::from_value(level->world->daytime, hour, minute, second);

		std::wstring timeString = 
					util::lfill(std::to_wstring(hour), 2, L'0') + L":" +
					util::lfill(std::to_wstring(minute), 2, L'0');
		return L"Time: "+timeString;
	})));

	{
		auto bar = std::make_shared<gui::TrackBar>(0.0f, 1.0f, 1.0f, 0.005f, 8);
		bar->supplier([=]() {
			return level->world->daytime;
		});
		bar->consumer([=](double val) {
			level->world->daytime = val;
		});
		panel->add(bar);
	}
	{
        auto checkbox = std::make_shared<gui::FullCheckBox>(L"Frustum-Culling", glm::vec2(400, 32));
        checkbox->supplier([=]() {
            return engine->getSettings().graphics.frustumCulling;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().graphics.frustumCulling = checked;
        });
        panel->add(checkbox);
	}
	{
        auto checkbox = std::make_shared<gui::FullCheckBox>(L"Show Chunk Borders", glm::vec2(400, 32));
        checkbox->supplier([=]() {
            return WorldRenderer::drawChunkBorders;
        });
        checkbox->consumer([=](bool checked) {
            WorldRenderer::drawChunkBorders = checked;
        });
		panel->add(checkbox);
	}

	panel->refresh();
	return panel;
}

std::shared_ptr<InventoryView> HudRenderer::createContentAccess() {
    auto level = levelFrontend->getLevel();
    auto content = level->content;
    auto indices = content->getIndices();
    auto player = level->player;
    auto inventory = player->getInventory();

    int itemsCount = indices->countItemDefs();
    auto accessInventory = std::make_shared<Inventory>(0, itemsCount);
    for (int id = 1; id < itemsCount; ++id) {
        accessInventory->getSlot(id - 1).set(ItemStack(id, 1));
    }

    SlotLayout slotLayout(glm::vec2(), false, true, 
    [=](ItemStack& item) {
        auto copy = ItemStack(item);
        inventory->move(copy, indices);
    }, 
    [=](ItemStack& item, ItemStack& grabbed) {
        inventory->getSlot(player->getChosenSlot()).set(item);
    });

    InventoryBuilder builder;
    builder.addGrid(8, itemsCount - 1, glm::vec2(), 8, true, slotLayout);
    auto layout = builder.build();
    auto contentAccess = std::make_shared<InventoryView>(
        content, 
		levelFrontend, 
		interaction.get(), 
		accessInventory, 
		std::move(layout)
    );
    contentAccess->build();
    return contentAccess;
}

std::shared_ptr<InventoryView> HudRenderer::createHotbar() {
    auto level = levelFrontend->getLevel();
    auto player = level->player;
    auto inventory = player->getInventory();
    auto content = level->content;

    SlotLayout slotLayout(glm::vec2(), false, false, nullptr, nullptr);
    InventoryBuilder builder;
    builder.addGrid(10, 10, glm::vec2(), 4, true, slotLayout);
    auto layout = builder.build();

    layout->setOrigin(glm::vec2(layout->getSize().x / 2, 0));
    auto view = std::make_shared<InventoryView>(
        content, 
		levelFrontend, 
		interaction.get(), 
		inventory, 
		std::move(layout)
    );
    view->build();
    view->setInteractive(false);
    return view;
}

std::shared_ptr<InventoryView> HudRenderer::createInventory() {
    auto level = levelFrontend->getLevel();
    auto player = level->player;
    auto inventory = player->getInventory();
    auto content = level->content;

    SlotLayout slotLayout(glm::vec2(), true, false, [=](ItemStack& stack) {
        stack.clear();
    }, nullptr);

    InventoryBuilder builder;
    builder.addGrid(10, inventory->size(), glm::vec2(), 4, true, slotLayout);
    auto layout = builder.build();

    auto view = std::make_shared<InventoryView>(
        content,
        levelFrontend,
        interaction.get(),
        inventory,
        std::move(layout)
    );
    view->build();
    return view;
}

HudRenderer::HudRenderer(Engine* engine, LevelFrontend* levelFrontend) : assets(engine->getAssets()), guiController(engine->getGUI()), levelFrontend(levelFrontend) {
	auto menu = guiController->getMenu();

    interaction = std::make_unique<InventoryInteraction>();
    grabbedItemView = std::make_shared<SlotView>(
        interaction->getGrabbedItem(), 
        levelFrontend,
		interaction.get(),
        levelFrontend->getLevel()->content,
        SlotLayout(glm::vec2(), false, false, nullptr, nullptr)
    );
    grabbedItemView->setColor(glm::vec4());
    grabbedItemView->setInteractive(false);

    contentAccess = createContentAccess();
    contentAccessPanel = std::make_shared<gui::Panel>(contentAccess->getSize(), glm::vec4(0.0f), 0.0f);
    contentAccessPanel->setColor(glm::vec4());
    contentAccessPanel->add(contentAccess);
    contentAccessPanel->scrollable(true);

    hotbarView = createHotbar();
    inventoryView = createInventory();

	darkOverlay = std::make_unique<gui::Panel>(glm::vec2(4000.0f));
    darkOverlay->setColor(glm::vec4(0, 0, 0, 0.5f));

	uicamera = std::make_unique<Camera>(glm::vec3(), 1);
	uicamera->perspective = false;
	uicamera->flipped = true;

    debugPanel = createDebugPanel(engine);
	menu->reset();

	guiController->addBack(darkOverlay);
    guiController->addBack(hotbarView);
	guiController->add(debugPanel);
    guiController->add(contentAccessPanel);
    guiController->add(inventoryView);
    guiController->add(grabbedItemView);
}

HudRenderer::~HudRenderer() {
    guiController->remove(grabbedItemView);
    guiController->remove(inventoryView);
    guiController->remove(hotbarView);
	guiController->remove(darkOverlay);
    guiController->remove(contentAccessPanel);
	guiController->remove(debugPanel);
}

void HudRenderer::drawDebug(int fps){
	this->fps = fps;
	fpsMin = glm::min(fps, fpsMin);
	fpsMax = glm::max(fps, fpsMax);
}

void HudRenderer::update(bool hudVisible) {
	auto level = levelFrontend->getLevel();
    auto player = level->player;
	auto menu = guiController->getMenu();

	debugPanel->setVisible(player->debug && hudVisible);
	menu->setVisible(pause);

	if (!hudVisible && inventoryOpen) closeInventory();
	if (pause && menu->current().panel == nullptr) pause = false;

	if (Events::justPressed(keycode::ESCAPE) && !guiController->isFocusCaught()) {
		if (pause) {
			pause = false;
			menu->reset();
		} else if (inventoryOpen) {
			closeInventory();
		} else {
			pause = true;
			menu->set("pause");
		}
	}

	if (hudVisible && Events::justActive(BIND_HUD_INVENTORY) && !pause) {
		if (inventoryOpen) closeInventory();
		else inventoryOpen = true;
	}

	if ((pause || inventoryOpen) == Events::_cursor_locked) Events::toggleCursor();

	glm::vec2 invSize = contentAccessPanel->getSize();
    inventoryView->setVisible(inventoryOpen);
    contentAccessPanel->setVisible(inventoryOpen);
    contentAccessPanel->setSize(glm::vec2(invSize.x, Window::height));
    hotbarView->setVisible(hudVisible);

    for (int i = keycode::NUM_1; i <= keycode::NUM_9; ++i) {
        if (Events::justPressed(i)) player->setChosenSlot(i - keycode::NUM_1);
    }
    if (Events::justPressed(keycode::NUM_0)) player->setChosenSlot(9);
    if (!pause && !inventoryOpen && Events::scroll) {
        int slot = player->getChosenSlot();
        slot = (slot - Events::scroll) % 10;
        if (slot < 0) slot += 10;
        player->setChosenSlot(slot);
    }

	darkOverlay->setVisible(pause);
}

void HudRenderer::draw(const GfxContext& context) {
	auto level = levelFrontend->getLevel();

    const Viewport& viewport = context.getViewport();
	const uint width = viewport.getWidth();
	const uint height = viewport.getHeight();

    Player* player = level->player;

    uicamera->setFov(height);

	auto batch = context.getBatch2D();
	batch->begin();

	ShaderProgram* uiShader = assets->getShader("ui");
	uiShader->use();
	uiShader->uniformMatrix("u_projview", uicamera->getProjView());

	hotbarView->setCoord(glm::vec2(width/2, height-65));
    hotbarView->setSelected(player->getChosenSlot());

	batch->begin();
	if (!pause && Events::_cursor_locked && !level->player->debug) {
		batch->setLineWidth(2.0f);
		batch->line(width / 2, height / 2 - 6, width / 2, height / 2 + 6, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width / 2 + 6, height / 2, width / 2 - 6, height / 2, 0.2f, 0.2f, 0.2f, 1.0f);
	}

	if (inventoryOpen) {
		auto caLayout = contentAccess->getLayout();
        auto invLayout = inventoryView->getLayout();
        float caWidth = caLayout->getSize().x;
        glm::vec2 invSize = invLayout->getSize();

        float width = viewport.getWidth();

        inventoryView->setCoord(glm::vec2(
            glm::min(width / 2 - invSize.x / 2, width - caWidth - 10 - invSize.x), 
            height / 2 - invSize.y / 2
        ));
        contentAccessPanel->setCoord(glm::vec2(width-caWidth, 0));
	}
	grabbedItemView->setCoord(glm::vec2(Events::cursor));
	batch->render();
}

void HudRenderer::closeInventory() {
    inventoryOpen = false;
    ItemStack& grabbed = interaction->getGrabbedItem();
    grabbed.clear();
}

bool HudRenderer::isInventoryOpen() const {
	return inventoryOpen;
}

bool HudRenderer::isPause() const {
	return pause;
}
