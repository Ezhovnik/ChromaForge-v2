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
#include "UIDocument.h"
#include "../logic/scripting/scripting.h"
#include "../delegates.h"

static std::shared_ptr<gui::Label> create_label(wstringsupplier supplier) {
	std::shared_ptr<gui::Label> label = std::make_shared<gui::Label>(L"-");
	label->textSupplier(supplier);
	return label;
}

std::shared_ptr<gui::UINode> HudRenderer::createDebugPanel(Engine* engine) {
	auto level = levelFrontend->getLevel();

    auto panel = std::make_shared<gui::Panel>(glm::vec2(350, 200), glm::vec4(5.0f), 2.0f);
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
		sub->setOrientation(gui::Orientation::horizontal);
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
			box->setText(std::to_wstring(int(hitbox->position[ax])));
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
		bar->setSupplier([=]() {
			return WorldRenderer::skyClearness;
		});
		bar->setConsumer([=](double val) {
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
		return L"Time: " + timeString;
	})));

	{
		auto bar = std::make_shared<gui::TrackBar>(0.0f, 1.0f, 1.0f, 0.005f, 8);
		bar->setSupplier([=]() {
			return level->world->daytime;
		});
		bar->setConsumer([=](double val) {
			level->world->daytime = val;
		});
		panel->add(bar);
	}
	{
        auto checkbox = std::make_shared<gui::FullCheckBox>(L"Frustum-Culling", glm::vec2(400, 24));
        checkbox->setSupplier([=]() {
            return engine->getSettings().graphics.frustumCulling;
        });
        checkbox->setConsumer([=](bool checked) {
            engine->getSettings().graphics.frustumCulling = checked;
        });
        panel->add(checkbox);
	}
	{
        auto checkbox = std::make_shared<gui::FullCheckBox>(L"Show Chunk Borders", glm::vec2(400, 24));
        checkbox->setSupplier([=]() {
            return WorldRenderer::drawChunkBorders;
        });
        checkbox->setConsumer([=](bool checked) {
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

    SlotLayout slotLayout(-1, glm::vec2(), false, true, 
    [=](uint, ItemStack& item) {
        auto copy = ItemStack(item);
        inventory->move(copy, indices);
    }, 
    [=](ItemStack& item, ItemStack& grabbed) {
        inventory->getSlot(player->getChosenSlot()).set(item);
    });

    InventoryBuilder builder;
    builder.addGrid(8, itemsCount - 1, glm::vec2(), 8, true, slotLayout);
    auto view = builder.build();
    view->bind(accessInventory, levelFrontend, interaction.get());
    return view;
}

std::shared_ptr<InventoryView> HudRenderer::createHotbar() {
    auto level = levelFrontend->getLevel();
    auto player = level->player;
    auto inventory = player->getInventory();

    SlotLayout slotLayout(-1, glm::vec2(), false, false, nullptr, nullptr);
    InventoryBuilder builder;
    builder.addGrid(10, 10, glm::vec2(), 4, true, slotLayout);
    auto view = builder.build();
    view->setOrigin(glm::vec2(view->getSize().x/2, 0));
    view->bind(inventory, levelFrontend, interaction.get());
	view->setInteractive(false);
    return view;
}

HudRenderer::HudRenderer(Engine* engine, LevelFrontend* levelFrontend) : assets(engine->getAssets()), guiController(engine->getGUI()), levelFrontend(levelFrontend) {
	auto menu = guiController->getMenu();

    interaction = std::make_unique<InventoryInteraction>();
    grabbedItemView = std::make_shared<SlotView>(SlotLayout(-1, glm::vec2(), false, false, nullptr, nullptr));
	grabbedItemView->bind(interaction->getGrabbedItem(), levelFrontend, interaction.get());
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

    debugPanel = createDebugPanel(engine);
	menu->reset();

	debugPanel->setZIndex(2);

	guiController->addBack(darkOverlay);
    guiController->addBack(hotbarView);
	guiController->add(debugPanel);
    guiController->add(contentAccessPanel);
    guiController->add(grabbedItemView);
}

HudRenderer::~HudRenderer() {
    guiController->remove(grabbedItemView);
    if (inventoryView) guiController->remove(inventoryView);
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

	if (!hudVisible && inventoryOpen) closeInventory();
	if (pause && menu->getCurrent().panel == nullptr) setPause(false);

	if (Events::justPressed(keycode::ESCAPE) && !guiController->isFocusCaught()) {
		if (pause) {
			setPause(false);
		} else if (inventoryOpen) {
			closeInventory();
		} else {
			setPause(true);
		}
	}

	if (hudVisible && Events::justActive(BIND_HUD_INVENTORY) && !pause) {
		if (inventoryOpen) closeInventory();
		else openInventory();
	}

	if ((pause || inventoryOpen) == Events::_cursor_locked) Events::toggleCursor();

	glm::vec2 invSize = contentAccessPanel->getSize();
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

	if (!pause && Events::_cursor_locked && !level->player->debug) {
		GfxContext crosshair_context = context.sub();
        crosshair_context.blendMode(BlendMode::Inversion);
        batch->texture(assets->getTexture("gui/crosshair"));
        int chsize = 16;
        batch->rect((width - chsize) / 2, (height - chsize) / 2, chsize, chsize, 0, 0, 1, 1, 1, 1, 1, 1);
        batch->render();
	}

	if (level->player->debug) {
		batch->texture(nullptr);
        const int dmwidth = 256;
        const float dmscale = 4000.0f;
        static float deltameter[dmwidth]{};
        static int index = 0;
        index = index + 1 % dmwidth;
        deltameter[index % dmwidth] = glm::min(0.2f, 1.0f / fps) * dmscale;
        batch->setLineWidth(1.0f);
        for (int i = index + 1; i < index + dmwidth; ++i) {
            int j = i % dmwidth;
            batch->line(width - dmwidth + i - index, height - deltameter[j], width - dmwidth + i - index, height, 1.0f, 1.0f, 1.0f, 0.2f);
        }
    }

	if (inventoryOpen) {
		float caWidth = contentAccess->getSize().x;
        contentAccessPanel->setCoord(glm::vec2(width-caWidth, 0));

		glm::vec2 invSize = inventoryView->getSize();
        inventoryView->setCoord(glm::vec2(
            glm::min(width / 2 - invSize.x / 2, width - caWidth - 10 - invSize.x), 
            height / 2 - invSize.y / 2
        ));
	}
	grabbedItemView->setCoord(glm::vec2(Events::cursor));
	batch->render();
}

void HudRenderer::openInventory() {
    auto level = levelFrontend->getLevel();
    auto player = level->player;
    auto inventory = player->getInventory();

    inventoryOpen = true;

    inventoryDocument = assets->getLayout(BUILTIN_CONTENT_NAMESPACE":inventory");
    inventoryView = std::dynamic_pointer_cast<InventoryView>(inventoryDocument->getRoot());
    inventoryView->bind(inventory, levelFrontend, interaction.get());
    scripting::on_ui_open(inventoryDocument, inventory.get());

    guiController->add(inventoryView);
}

void HudRenderer::closeInventory() {
	scripting::on_ui_close(inventoryDocument, inventoryView->getInventory().get());
    inventoryOpen = false;
    ItemStack& grabbed = interaction->getGrabbedItem();
    grabbed.clear();
	guiController->remove(inventoryView);
    inventoryView = nullptr;
    inventoryDocument = nullptr;
}

bool HudRenderer::isInventoryOpen() const {
	return inventoryOpen;
}

bool HudRenderer::isPause() const {
	return pause;
}

void HudRenderer::setPause(bool pause) {
    if (this->pause == pause) return;
    this->pause = pause;

    auto menu = guiController->getMenu();
    if (pause) menu->setPage("pause");
    else menu->reset();

    darkOverlay->setVisible(pause);
    menu->setVisible(pause);
}
