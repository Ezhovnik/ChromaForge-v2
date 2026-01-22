#include "hud.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../typedefs.h"
#include "../util/stringutil.h"
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
#include "gui/controls.h"
#include "gui/panels.h"
#include "gui/UINode.h"
#include "gui/GUI.h"

inline gui::Label* create_label(gui::wstringsupplier supplier) {
	gui::Label* label = new gui::Label(L"-");
	label->textSupplier(supplier);
	return label;
}

HudRenderer::HudRenderer(gui::GUI* gui, Level* level, Assets* assets) : level(level), assets(assets), guiController(gui) {
	batch = new Batch2D(1024);
	uicamera = new Camera(glm::vec3(), Window::height);
	uicamera->perspective = false;
	uicamera->flipped = true;

    gui->interval(1.0f, [this]() {
		fpsString = std::to_wstring(fpsMax) + L" / " + std::to_wstring(fpsMin);
		fpsMin = fps;
		fpsMax = fps;
	});

	gui::Panel* panel = new gui::Panel(glm::vec2(200, 200), glm::vec4(5.0f), 1.0f);
	panel->setCoord(glm::vec2(10, 10));
	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"chunks: "+std::to_wstring(this->level->chunks->chunksCount);
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"fps: "+this->fpsString;
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"occlusion: " + std::wstring(this->occlusion ? L"on" : L"off");
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		std::wstringstream stream;
		stream << std::hex << this->level->player->selectedVoxel.states;
		return L"block-selected: " + std::to_wstring(this->level->player->selectedVoxel.id) + L" " + stream.str();
	})));
	panel->add(std::shared_ptr<gui::Label>(create_label([this](){
		return L"meshes: " + std::to_wstring(Mesh::meshesCount);
	})));
	for (int ax = 0; ax < 3; ax++){
		gui::Panel* sub = new gui::Panel(glm::vec2(10, 27), glm::vec4(0.0f));
		sub->orientation(gui::Orientation::horizontal);
		gui::Label* label = new gui::Label(std::wstring({static_cast<wchar_t>(L'x' + ax)}) + L": ");
		label->margin(glm::vec4(2, 3, 2, 3));
		sub->add(std::shared_ptr<gui::UINode>(label));
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

		sub->add(std::shared_ptr<gui::UINode>(box));
		panel->add(std::shared_ptr<gui::UINode>(sub));
	}
	panel->refresh();
	debugPanel = panel;

	pauseMenu = new gui::Panel(glm::vec2(350, 200));
	pauseMenu->color(glm::vec4(0.0f));
	{
		gui::Button* button = new gui::Button(L"Continue", glm::vec4(12.0f, 10.0f, 12.0f, 12.0f));
		button->listenAction([this](gui::GUI*){this->pause = false;});
		pauseMenu->add(std::shared_ptr<gui::UINode>(button));
	}
	{
		gui::Button* button = new gui::Button(L"Save and Quit", glm::vec4(12.0f, 10.0f, 12.0f, 12.0f));
		button->listenAction([this](gui::GUI*){Window::setShouldClose(true);});
		pauseMenu->add(std::shared_ptr<gui::UINode>(button));
	}
	guiController->add(std::shared_ptr<gui::UINode>(debugPanel));
	guiController->add(std::shared_ptr<gui::UINode>(pauseMenu));
}

HudRenderer::~HudRenderer() {
	delete batch;
	delete uicamera;
	delete guiController;
}

void HudRenderer::drawDebug(int fps, bool occlusion){
	this->occlusion = occlusion;
	this->fps = fps;
	fpsMin = glm::min(fps, fpsMin);
	fpsMax = glm::max(fps, fpsMax);
}

void HudRenderer::drawInventory(Player* player) {
	Texture* blocks = assets->getTexture("blocks_tex");

	uint size = 48;
	uint step = 64;

	uint inv_cols = 10;
	uint inv_rows = 8;

	uint inv_w = step*inv_cols + size;
	uint inv_h = step*inv_rows + size;

    const uint width = Window::width;
	const uint height = Window::height;

	int inv_x = (width - (inv_w)) / 2;
	int inv_y = (height - (inv_h)) / 2;
	int xs = (width - inv_w + step) / 2;
	int ys = (height - inv_h + step) / 2;
	if (width > inv_w * 3){
		inv_x = (width + (inv_w)) / 2;
		inv_y = (height - (inv_h)) / 2;
		xs = (width + inv_w + step) / 2;
		ys = (height - inv_h + step) / 2;
	}
	glm::vec4 tint = glm::vec4(1.0f);
	int mx = Events::x;
	int my = Events::y;
	uint count = inv_cols * inv_rows;

	// back
	batch->texture(nullptr);
	batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.3f);
	batch->rect(inv_x - 4, inv_y - 4, inv_w+8, inv_h+8,
					0.95f, 0.95f, 0.95f, 0.85f, 0.85f, 0.85f,
					0.7f, 0.7f, 0.7f,
					0.55f, 0.55f, 0.55f, 0.45f, 0.45f, 0.45f, 4);
	batch->rect(inv_x, inv_y, inv_w, inv_h,
					0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
					0.75f, 0.75f, 0.75f,
					0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 4);

	batch->color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
	for (uint i = 0; i < count; i++) {
		int x = xs + step * (i % (inv_cols));
		int y = ys + step * (i / (inv_cols));
		batch->rect(x-2, y-2, size+4, size+4,
					0.45f, 0.45f, 0.45f, 0.55f, 0.55f, 0.55f,
					0.7f, 0.7f, 0.7f,
					0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f, 2);
		batch->rect(x, y, size, size,
					0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 0.65f,
					0.65f, 0.65f, 0.65f,
					0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 2);
	}

	// front
	batch->texture(blocks);
	for (uint i = 0; i < count; i++) {
		Block* cblock = Block::blocks[i+1].get();
		if (cblock == nullptr) break;
		int x = xs + step * (i % inv_cols);
		int y = ys + step * (i / inv_cols);
		if (mx > x && mx < x + (int)size && my > y && my < y + (int)size) {
			tint.r *= 1.2f;
			tint.g *= 1.2f;
			tint.b *= 1.2f;
			if (Events::justClicked(GLFW_MOUSE_BUTTON_LEFT)) player->choosenBlock = i + 1;
		} else
		{
			tint = glm::vec4(1.0f);
		}
		
		if (cblock->model == Block_models::CUBE) batch->blockSprite(x, y, size, size, 16, cblock->textureFaces, tint);
		else if (cblock->model == Block_models::X) batch->sprite(x, y, size, size, 16, cblock->textureFaces[3], tint);
	}
}

void HudRenderer::draw(){
    const uint width = Window::width;
	const uint height = Window::height;

	debugPanel->visible(level->player->debug);
	pauseMenu->visible(pause);
	pauseMenu->setCoord(glm::vec2(width / 2.0f, height / 2.0f) - pauseMenu->size() / 2.0f);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	uicamera->fov = height;

	ShaderProgram* uishader = assets->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection() * uicamera->getView());

	// Chosen block preview
	Texture* blocks = assets->getTexture("blocks_tex");

	batch->texture(nullptr);
	batch->color = glm::vec4(1.0f);
	if (Events::_cursor_locked && !level->player->debug) {
		glLineWidth(2);
		batch->line(width / 2, height / 2 - 6, width / 2, height / 2 + 6, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width / 2 + 6, height / 2, width / 2 - 6, height / 2, 0.2f, 0.2f, 0.2f, 1.0f);
	}

	batch->rect(width / 2 - 128 - 4, height - 80 - 4, 256 + 8, 64 + 8,
					0.95f, 0.95f, 0.95f, 0.85f, 0.85f, 0.85f,
					0.7f, 0.7f, 0.7f,
					0.55f, 0.55f, 0.55f, 0.45f, 0.45f, 0.45f, 4
                );
	batch->rect(width / 2 - 128, height - 80, 256, 64,
					0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
					0.75f, 0.75f, 0.75f,
					0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 4
                );
	batch->rect(width / 2 - 32 + 2, height - 80 + 2, 60, 60,
					0.45f, 0.45f, 0.45f, 0.55f, 0.55f, 0.55f,
					0.7f, 0.7f, 0.7f,
					0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f, 2
                );
	batch->rect(width / 2 - 32 + 4, height - 80 + 4, 56, 56,
					0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
					0.75f, 0.75f, 0.75f,
					0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 2
                );

	batch->texture(blocks);
	Player* player = level->player;
	{
		Block* cblock = Block::blocks[player->choosenBlock].get();
		if (cblock->model == Block_models::CUBE){
			batch->blockSprite(width / 2 - 24, uicamera->fov - 72, 48, 48, 16, cblock->textureFaces, glm::vec4(1.0f));
		} else if (cblock->model == Block_models::X){
			batch->sprite(width / 2 - 24, uicamera->fov - 72, 48, 48, 16, cblock->textureFaces[3], glm::vec4(1.0f));
		}
	}

	if (Events::justPressed(keycode::ESCAPE) && !guiController->isFocusCaught()) {
		if (pause) pause = false;
		else if (inventoryOpen) inventoryOpen = false;
		else pause = true;
	}
	if (Events::justPressed(keycode::E) && !pause) inventoryOpen = !inventoryOpen;

	if ((pause || inventoryOpen) == Events::_cursor_locked) Events::toggleCursor();

	if (pause || inventoryOpen) {
		batch->texture(nullptr);
		batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
		batch->rect(0, 0, width, height);
	}

	if (inventoryOpen) drawInventory(player);

	batch->render();
}

bool HudRenderer::isInventoryOpen() const {
	return inventoryOpen;
}

bool HudRenderer::isPause() const {
	return pause;
}
