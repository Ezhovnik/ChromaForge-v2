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

inline gui::Label* create_label(gui::wstringsupplier supplier) {
	gui::Label* label = new gui::Label(L"-");
	label->textSupplier(supplier);
	return label;
}

HudRenderer::HudRenderer(Engine* engine, Level* level) : assets(engine->getAssets()), level(level), guiController(engine->getGUI()) {
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
		return L"fps: " + this->fpsString;
	})));
    panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"meshes: " + std::to_wstring(Mesh::meshesCount);
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"occlusion: " + std::wstring(this->occlusion ? L"on" : L"off");
	})));
    panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"chunks: " + std::to_wstring(this->level->chunks->chunksCount) + L" visible: " + std::to_wstring(this->level->chunks->visibleCount);
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		std::wstringstream stream;
        auto player = this->level->player;
		stream << std::hex << player->selectedVoxel.states;
		return L"block-selected: " + std::to_wstring(player->selectedVoxel.id) + L" " + stream.str();
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"seed: " + std::to_wstring(this->level->world->seed);
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

// void HudRenderer::drawInventory(const GfxContext& context, Player* player) {
// 	Texture* blocks = assets->getTexture("blocks_tex");
//     if (blocks == nullptr) {
//         LOG_CRITICAL("The texture 'bloks_tex' could not be found in the assets");
//         throw std::runtime_error("The texture 'bloks_tex' could not be found in the assets");
//     }

//     const Viewport& viewport = context.getViewport();
// 	const uint width = viewport.getWidth();
// 	const uint height = viewport.getHeight();

// 	uint size = 48;
// 	uint step = 64;
// 	uint inv_cols = 10;
// 	uint inv_rows = 8;
// 	uint inv_w = step * inv_cols + size;
// 	uint inv_h = step * inv_rows + size;
// 	int inv_x = (width - (inv_w)) / 2;
// 	int inv_y = (height - (inv_h)) / 2;
// 	int xs = (width - inv_w + step) / 2;
// 	int ys = (height - inv_h + step) / 2;
// 	if (width > inv_w * 3){
// 		inv_x = (width + (inv_w)) / 2;
// 		inv_y = (height - (inv_h)) / 2;
// 		xs = (width + inv_w + step) / 2;
// 		ys = (height - inv_h + step) / 2;
// 	}
// 	glm::vec4 tint = glm::vec4(1.0f);
// 	int mx = Events::x;
// 	int my = Events::y;
// 	uint count = inv_cols * inv_rows;

// 	batch->texture(nullptr);
// 	batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.3f);
// 	batch->rect(0, 0, width, height);
// 	batch->rect(inv_x - 4, inv_y - 4, inv_w+8, inv_h+8,
// 					0.95f, 0.95f, 0.95f, 0.85f, 0.85f, 0.85f,
// 					0.7f, 0.7f, 0.7f,
// 					0.55f, 0.55f, 0.55f, 0.45f, 0.45f, 0.45f, 4);
// 	batch->rect(inv_x, inv_y, inv_w, inv_h,
// 					0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
// 					0.75f, 0.75f, 0.75f,
// 					0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 4);

// 	batch->color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
// 	for (uint i = 0; i < count; ++i) {
// 		int x = xs + step * (i % (inv_cols));
// 		int y = ys + step * (i / (inv_cols));
// 		batch->rect(x-2, y-2, size+4, size+4,
// 					0.45f, 0.45f, 0.45f, 0.55f, 0.55f, 0.55f,
// 					0.7f, 0.7f, 0.7f,
// 					0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f, 2);
// 		batch->rect(x, y, size, size,
// 					0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 0.65f,
// 					0.65f, 0.65f, 0.65f,
// 					0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 2);
// 	}

// 	batch->texture(blocks);
// 	for (uint i = 0; i < count; i++) {
// 		Block* cblock = Block::blocks[i + 1];
// 		if (cblock == nullptr) break;
// 		int x = xs + step * (i % inv_cols);
// 		int y = ys + step * (i / inv_cols);
// 		if (mx > x && mx < x + (int)size && my > y && my < y + (int)size) {
// 			tint.r *= 1.2f;
// 			tint.g *= 1.2f;
// 			tint.b *= 1.2f;
// 			if (Events::justClicked(mousecode::BUTTON_1)) player->choosenBlock = i + 1;
// 		} else {
//             tint = glm::vec4(1.0f);
//         }
		
// 		if (cblock->model == BlockModel::Cube){
// 			batch->blockSprite(x, y, size, size, 16, cblock->textureFaces, tint);
// 		} else if (cblock->model == BlockModel::X){
// 			batch->sprite(x, y, size, size, 16, cblock->textureFaces[3], tint);
// 		}
// 	}
// }

void HudRenderer::drawContentAccess(const GfxContext& context, Player* player) {
	const Content* content = level->content;
	const ContentIndices* contentIds = content->indices;

	const Viewport& viewport = context.getViewport();
	const uint width = viewport.getWidth();

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

	Texture* blocks = assets->getTexture("blocks_tex");
    if (blocks == nullptr) {
        LOG_CRITICAL("The texture 'blocks_tex' could not be found in the assets");
        throw std::runtime_error("The texture 'blocks_tex' could not be found in the assets");
    }

	batch->texture(blocks);
	for (uint i = 0; i < count - 1; ++i) {
		Block* cblock = contentIds->getBlockDef(i + 1);
		if (cblock == nullptr) break;
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
		
		if (cblock->model == BlockModel::Cube){
			batch->blockSprite(x, y, icon_size, icon_size, 16, cblock->textureFaces, tint);
		} else if (cblock->model == BlockModel::X){
			batch->sprite(x, y, icon_size, icon_size, 16, cblock->textureFaces[3], tint);
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

    debugPanel->visible(level->player->debug);

    uicamera->fov = height;

	ShaderProgram* uishader = assets->getShader("ui");
    if (uishader == nullptr) {
        LOG_CRITICAL("The shader 'ui' could not be found in the assets");
        throw std::runtime_error("The shader 'ui' could not be found in the assets");
    }

	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjView());

	Texture* blocks = assets->getTexture("blocks_tex");
    if (blocks == nullptr) {
        LOG_CRITICAL("The texture 'blocks_tex' could not be found in the assets");
        throw std::runtime_error("The texture 'blocks_tex' could not be found in the assets");
    }

	batch->texture(nullptr);
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
	batch->texture(blocks);
	{
		Block* cblock = contentIds->getBlockDef(player->choosenBlock);
		assert(cblock != nullptr);
		if (cblock->model == BlockModel::Cube){
			batch->blockSprite(width - 56, uicamera->fov - 56, 48, 48, 16, cblock->textureFaces, glm::vec4(1.0f));
		} else if (cblock->model == BlockModel::X){
			batch->sprite(width - 56, uicamera->fov - 56, 48, 48, 16, cblock->textureFaces[3], glm::vec4(1.0f));
		}
	}

	if (pause || inventoryOpen) {
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
