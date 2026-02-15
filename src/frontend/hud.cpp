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
#include "world_render.h"
#include "../util/timeutil.h"
#include "../util/stringutil.h"

inline gui::Label* create_label(gui::wstringsupplier supplier) {
	gui::Label* label = new gui::Label(L"-");
	label->textSupplier(supplier);
	return label;
}

HudRenderer::HudRenderer(Engine* engine, Level* level, const ContentGfxCache* cache, WorldRenderer* renderer) : assets(engine->getAssets()), level(level), guiController(engine->getGUI()), cache(cache), renderer(renderer) {
	auto menu = guiController->getMenu();
    batch = new Batch2D(1024);

	uicamera = new Camera(glm::vec3(), 1);
	uicamera->perspective = false;
	uicamera->flipped = true;

    gui::Panel* panel = new gui::Panel(glm::vec2(250, 200), glm::vec4(5.0f), 1.0f);
    debugPanel = std::shared_ptr<gui::UINode>(panel);
	panel->listenInterval(1.0f, [this]() {
		fpsString = std::to_wstring(fpsMax)+L" / "+std::to_wstring(fpsMin);
		fpsMin = fps;
		fpsMax = fps;
	});

	panel->setCoord(glm::vec2(10, 10));
	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"FPS: " + this->fpsString;
	})));
    panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"Meshes: " + std::to_wstring(Mesh::meshesCount);
	})));
    panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"Chunks: " + std::to_wstring(this->level->chunks->chunksCount) + L" (visible: " + std::to_wstring(this->level->chunks->visibleCount) + L")";
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		std::wstringstream stream;
        auto player = this->level->player;
		stream << std::hex << player->selectedVoxel.states;
		return L"Block-selected: " + std::to_wstring(player->selectedVoxel.id) + L" " + stream.str();
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"Seed: " + std::to_wstring(this->level->world->seed);
	})));
	for (int ax = 0; ax < 3; ax++){
		gui::Panel* sub = new gui::Panel(glm::vec2(10, 27), glm::vec4(0.0f));
		sub->orientation(gui::Orientation::horizontal);
		gui::Label* label = new gui::Label(std::wstring({static_cast<wchar_t>(L'x' + ax)}) + L": ");
		label->margin(glm::vec4(2, 3, 2, 3));
		sub->add(label);
		sub->color(glm::vec4(0.0f));

		gui::TextBox* box = new gui::TextBox(L"");
		box->textSupplier([this, ax]() {
			Hitbox* hitbox = this->level->player->hitbox;
			return std::to_wstring((int)hitbox->position[ax]);
		});
		box->textConsumer([this, ax](std::wstring text) {
			try {
				glm::vec3 position = this->level->player->hitbox->position;
				position[ax] = std::stoi(text);
				this->level->player->teleport(position);
			} catch (std::invalid_argument& _){
			}
		});

		sub->add(box);
		panel->add(sub);
	}

	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		int hour, minute, second;
		timeutil::from_value(this->level->world->daytime, hour, minute, second);

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
            return engine->getSettings().chunks.occlusion;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().chunks.occlusion = checked;
        });
        checkpanel->add(checkbox);
        checkpanel->add(new gui::Label(L"Occlusion"));

        panel->add(checkpanel);
	}
	{
        gui::Panel* checkpanel = new gui::Panel(glm::vec2(400, 32), glm::vec4(5.0f), 1.0f);
        checkpanel->color(glm::vec4(0.0f));
        checkpanel->orientation(gui::Orientation::horizontal);

        gui::CheckBox* checkbox = new gui::CheckBox();
        checkbox->margin(glm::vec4(0.0f, 0.0f, 5.0f, 0.0f));
        checkbox->supplier([=]() {
            return renderer->isChunkBordersOn();
        });
        checkbox->consumer([=](bool checked) {
            renderer->setChunkBorders(checked);
        });
        checkpanel->add(checkbox);
        checkpanel->add(new gui::Label(L"Show Chunk Borders"));

        panel->add(checkpanel);
	}
	panel->refresh();

    menu->reset();
	
	guiController->add(this->debugPanel);
}

HudRenderer::~HudRenderer() {
    guiController->remove(debugPanel);
	//guiController->remove(gui->get("pages"));

	delete batch;
	delete uicamera;
}

void HudRenderer::drawDebug(int fps, bool occlusion){
	this->occlusion = occlusion;
	this->fps = fps;
	fpsMin = glm::min(fps, fpsMin);
	fpsMax = glm::max(fps, fpsMax);
}

void HudRenderer::drawContentAccess(const GfxContext& context, Player* player) {
	const Content* content = level->content;
	const ContentIndices* contentIds = content->indices;

	const Viewport& viewport = context.getViewport();
	const uint width = viewport.getWidth();

	Atlas* atlas = assets->getAtlas("blocks");

	uint count = contentIds->countBlockDefs();
	uint icon_size = 48;
	uint interval = 4;
	uint inv_cols = 8;
	uint inv_rows = ceildiv(count - 1, inv_cols);
	int pad_x = interval;
	int pad_y = interval;
	uint inv_w = inv_cols * icon_size + (inv_cols - 1) * interval + pad_x * 2;
	uint inv_h = inv_rows * icon_size + (inv_rows - 1) * interval + pad_x * 2;
	int inv_x = width - inv_w;
	int inv_y = 0;
	int xs = inv_x + pad_x;
	int ys = inv_y + pad_y;

	glm::vec4 tint = glm::vec4(1.0f);
	int mx = Events::x;
	int my = Events::y;

	batch->texture(nullptr);
	batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
	batch->rect(inv_x, inv_y, inv_w, inv_h);

	batch->texture(atlas->getTexture());
	for (uint i = 0; i < count - 1; ++i) {
		Block* choosen_block = contentIds->getBlockDef(i + 1);
		if (choosen_block == nullptr) break;
		int x = xs + (icon_size + interval) * (i % inv_cols);
		int y = ys + (icon_size + interval) * (i / inv_cols);
		if (mx > x && mx < x + (int)icon_size && my > y && my < y + (int)icon_size) {
			tint.r *= 1.2f;
			tint.g *= 1.2f;
			tint.b *= 1.2f;
			if (Events::justClicked(mousecode::BUTTON_1)) player->choosenBlock = i + 1;
		} else {
			tint = glm::vec4(1.0f);
		}
		
		if (choosen_block->model == BlockModel::Cube){
			batch->blockSprite(x, y, icon_size, icon_size, &cache->getRegion(choosen_block->id, 0), tint);
		} else if (choosen_block->model == BlockModel::X){
			batch->sprite(x, y, icon_size, icon_size, cache->getRegion(choosen_block->id, 3), tint);
		}
	}
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
    const Content* content = level->content;
	const ContentIndices* contentIds = content->indices;

    const Viewport& viewport = context.getViewport();
	const uint width = viewport.getWidth();
	const uint height = viewport.getHeight();

	Atlas* atlas = assets->getAtlas("blocks");

    debugPanel->visible(level->player->debug);

    uicamera->fov = height;

	ShaderProgram* uishader = assets->getShader("ui");
    if (uishader == nullptr) {
        LOG_CRITICAL("The shader 'ui' could not be found in the assets");
        throw std::runtime_error("The shader 'ui' could not be found in the assets");
    }

	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjView());

	batch->begin();

	batch->color = glm::vec4(1.0f);
	if (Events::_cursor_locked && !level->player->debug) {
		batch->setLineWidth(2.0f);
		batch->line(width / 2, height / 2 - 6, width / 2, height / 2 + 6, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width / 2 + 6, height / 2, width / 2 - 6, height / 2, 0.2f, 0.2f, 0.2f, 1.0f);
	}

    Player* player = level->player;

	batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
	batch->rect(width - 68, height - 68, 68, 68);

	batch->color = glm::vec4(1.0f);
	batch->texture(atlas->getTexture());
	{
		Block* choosen_block = contentIds->getBlockDef(player->choosenBlock);
		assert(choosen_block != nullptr);
		if (choosen_block->model == BlockModel::Cube){
			batch->blockSprite(width-56, uicamera->fov - 56, 48, 48, &cache->getRegion(choosen_block->id, 0), glm::vec4(1.0f));
		} else if (choosen_block->model == BlockModel::X){
			batch->sprite(width-56, uicamera->fov - 56, 48, 48, cache->getRegion(choosen_block->id, 3), glm::vec4(1.0f));
		}
	}

	if (pause) {
		batch->texture(nullptr);
		batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
		batch->rect(0, 0, width, height);
	}

	if (inventoryOpen) drawContentAccess(context, player);

	batch->render();
}

bool HudRenderer::isInventoryOpen() const {
	return inventoryOpen;
}

bool HudRenderer::isPause() const {
	return pause;
}
