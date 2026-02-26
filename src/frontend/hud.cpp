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

inline std::shared_ptr<gui::Label> create_label(gui::wstringsupplier supplier) {
	std::shared_ptr<gui::Label> label = std::make_shared<gui::Label>(L"-");
	label->textSupplier(supplier);
	return label;
}

void HudRenderer::createDebugPanel(Engine* engine) {
	auto level = levelFrontend->getLevel();

    gui::Panel* panel = new gui::Panel(glm::vec2(350, 200), glm::vec4(5.0f), 1.0f);
    debugPanel = std::shared_ptr<gui::UINode>(panel);
	panel->listenInterval(1.0f, [this]() {
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
		auto indices = level->content->indices;
		auto def = indices->getBlockDef(player->selectedVoxel.id);

		std::wstringstream stream;
		stream << std::hex << player->selectedVoxel.states;
		if (def) stream << L" (" << util::str2wstr_utf8(def->name) << L")";

		return L"Selected-block " + std::to_wstring(player->selectedVoxel.id) + L" " + stream.str();
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([=](){
		return L"Seed: " + std::to_wstring(level->world->seed);
	})));
	for (int ax = 0; ax < 3; ax++){
		gui::Panel* sub = new gui::Panel(glm::vec2(10, 27), glm::vec4(0.0f));
		sub->orientation(gui::Orientation::horizontal);
		gui::Label* label = new gui::Label(std::wstring({static_cast<wchar_t>(L'x' + ax)}) + L": ");
		label->margin(glm::vec4(2, 3, 2, 3));
		sub->add(label);
		sub->color(glm::vec4(0.0f));

		gui::TextBox* box = new gui::TextBox(L"");
		box->textSupplier([=]() {
			Hitbox* hitbox = level->player->hitbox;
			return std::to_wstring(hitbox->position[ax]);
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
		gui::TrackBar* bar = new gui::TrackBar(0.0f, 1.0f, 0.0f, 0.005f, 8);
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
		gui::TrackBar* bar = new gui::TrackBar(0.0f, 1.0f, 1.0f, 0.005f, 8);
		bar->supplier([=]() {
			return level->world->daytime;
		});
		bar->consumer([=](double val) {
			level->world->daytime = val;
		});
		panel->add(bar);
	}
	{
        gui::Panel* checkpanel = new gui::Panel(glm::vec2(400, 32), glm::vec4(5.0f), 1.0f);
        checkpanel->color(glm::vec4(0.0f));
        checkpanel->orientation(gui::Orientation::horizontal);

        gui::CheckBox* checkbox = new gui::CheckBox();
        checkbox->margin(glm::vec4(0.0f, 0.0f, 5.0f, 0.0f));
        checkbox->supplier([=]() {
            return engine->getSettings().graphics.frustumCulling;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().graphics.frustumCulling = checked;
        });
        checkpanel->add(checkbox);
        checkpanel->add(new gui::Label(L"Frustum-Culling"));

        panel->add(checkpanel);
	}
	{
        gui::Panel* checkpanel = new gui::Panel(glm::vec2(400, 32), glm::vec4(5.0f), 1.0f);
        checkpanel->color(glm::vec4(0.0f));
        checkpanel->orientation(gui::Orientation::horizontal);

        gui::CheckBox* checkbox = new gui::CheckBox();
        checkbox->margin(glm::vec4(0.0f, 0.0f, 5.0f, 0.0f));
        checkbox->supplier([=]() {
            return WorldRenderer::drawChunkBorders;
        });
        checkbox->consumer([=](bool checked) {
            WorldRenderer::drawChunkBorders = checked;
        });
        checkpanel->add(checkbox);
        checkpanel->add(new gui::Label(L"Show Chunk Borders"));

        panel->add(checkpanel);
	}
	panel->refresh();
}

HudRenderer::HudRenderer(Engine* engine, LevelFrontend* levelFrontend) : assets(engine->getAssets()), guiController(engine->getGUI()), levelFrontend(levelFrontend) {
	auto menu = guiController->getMenu();

	auto level = levelFrontend->getLevel();
	auto content = level->content;
    auto indices = content->indices;
    std::vector<blockid_t> blocks;
    for (blockid_t id = 1; id < indices->countBlockDefs(); ++id) {
        const Block* def = indices->getBlockDef(id);
        if (def->hidden) continue;
        blocks.push_back(id);
    }
    contentAccess.reset(new InventoryView(8, indices, blocks, levelFrontend));
	contentAccess->setSlotConsumer([=](blockid_t id) {level->player->chosenBlock = id;});

	uicamera = new Camera(glm::vec3(), 1);
	uicamera->perspective = false;
	uicamera->flipped = true;

    gui::Panel* panel = new gui::Panel(glm::vec2(350, 200), glm::vec4(5.0f), 1.0f);
    debugPanel = std::shared_ptr<gui::UINode>(panel);
	panel->listenInterval(1.0f, [this]() {
		fpsString = std::to_wstring(fpsMax)+L" / "+std::to_wstring(fpsMin);
		fpsMin = fps;
		fpsMax = fps;
	});

	createDebugPanel(engine);
    menu->reset();
	guiController->add(this->debugPanel);
}

HudRenderer::~HudRenderer() {
    guiController->remove(debugPanel);
	delete uicamera;
}

void HudRenderer::drawDebug(int fps){
	this->fps = fps;
	fpsMin = glm::min(fps, fpsMin);
	fpsMax = glm::max(fps, fpsMax);
}

void HudRenderer::update() {
	auto menu = guiController->getMenu();
	if (pause && menu->current().panel == nullptr) pause = false;

	if (Events::justPressed(keycode::ESCAPE) && !guiController->isFocusCaught()) {
		if (pause) {
			pause = false;
			menu->reset();
		} else if (inventoryOpen) {
			inventoryOpen = false;
		} else {
			pause = true;
			menu->set("pause");
		}
	}

	if (Events::justActive(BIND_HUD_INVENTORY) && !pause) inventoryOpen = !inventoryOpen;

	if ((pause || inventoryOpen) == Events::_cursor_locked) Events::toggleCursor();
}

void HudRenderer::draw(const GfxContext& context) {
	auto level = levelFrontend->getLevel();
    const Content* content = level->content;
	const ContentIndices* contentIds = content->indices;

    const Viewport& viewport = context.getViewport();
	const uint width = viewport.getWidth();
	const uint height = viewport.getHeight();

    debugPanel->visible(level->player->debug);

    uicamera->setFov(height);

	ShaderProgram* uiShader = assets->getShader("ui");
    if (uiShader == nullptr) {
        LOG_CRITICAL("The shader 'ui' could not be found in the assets");
        throw std::runtime_error("The shader 'ui' could not be found in the assets");
    }

	uiShader->use();
	uiShader->uniformMatrix("u_projview", uicamera->getProjView());

	Batch2D* batch = context.getBatch2D();
	batch->begin();

	batch->color = glm::vec4(1.0f);
	if (Events::_cursor_locked && !level->player->debug) {
		batch->setLineWidth(2.0f);
		batch->line(width / 2, height / 2 - 6, width / 2, height / 2 + 6, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width / 2 + 6, height / 2, width / 2 - 6, height / 2, 0.2f, 0.2f, 0.2f, 1.0f);
	}

    Player* player = level->player;

	batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.75f);
	batch->rect(width - 68, height - 68, 68, 68);

	batch->color = glm::vec4(1.0f);
	batch->render();

	auto blocksPreview = levelFrontend->getBlocksPreview();
	blocksPreview->begin(&context.getViewport());
	{
		Window::clearDepth();
		GfxContext subctx = context.sub();
		subctx.depthTest(true);
		subctx.cullFace(true);

		Block* chosen_block = contentIds->getBlockDef(player->chosenBlock);
		assert(chosen_block != nullptr);
		blocksPreview->draw(chosen_block, width - 56, uicamera->getFov() - 56, 48, glm::vec4(1.0f));
	}
	uiShader->use();
	batch->begin();

	if (pause) {
		batch->texture(nullptr);
		batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
		batch->rect(0, 0, width, height);
	}

	if (inventoryOpen) {
		contentAccess->setPosition(viewport.getWidth() - contentAccess->getWidth(), 0);
        contentAccess->activateAndDraw(&context);
	}

	batch->render();
}

bool HudRenderer::isInventoryOpen() const {
	return inventoryOpen;
}

bool HudRenderer::isPause() const {
	return pause;
}
