#include "hud_render.h"

#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Assets.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Batch2D.h"
#include "graphics/Font.h"
#include "graphics/Mesh.h"
#include "window/Camera.h"
#include "window/Window.h"
#include "window/Events.h"
#include "voxels/Chunks.h"
#include "voxels/Block.h"
#include "world/World.h"
#include "world/Level.h"
#include "objects/Player.h"
#include "logger/Logger.h"

HudRenderer::HudRenderer() {
	batch = new Batch2D(1024);

	uicamera = new Camera(glm::vec3(), Window::height / 1.0f);
	uicamera->perspective = false;
	uicamera->flipped = true;
}

HudRenderer::~HudRenderer() {
	delete batch;
	delete uicamera;
}

void HudRenderer::drawDebug(Level* level, Assets* assets, int fps, bool occlusion){
	Chunks* chunks = level->chunks;
	Player* player = level->player;

	Font* font = assets->getFont("normal");
    if (font == nullptr) {
        LOG_CRITICAL("The font 'normal' could not be found in the assets");
        throw std::runtime_error("The font 'normal' could not be found in the assets");
    }

	ShaderProgram* uishader = assets->getShader("ui");
    if (uishader == nullptr) {
        LOG_CRITICAL("The shader 'ui' could not be found in the assets");
        throw std::runtime_error("The shader 'ui' could not be found in the assets");
    }
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection() * uicamera->getView());
	batch->color = glm::vec4(1.0f);
	batch->begin();

	font->draw(batch, L"chunks: "+std::to_wstring(chunks->chunksCount), 16, 16, FONT_STYLES::OUTLINE);
	font->draw(batch, std::to_wstring((int)player->camera->position.x), 10, 30, FONT_STYLES::OUTLINE);
	font->draw(batch, std::to_wstring((int)player->camera->position.y), 50, 30, FONT_STYLES::OUTLINE);
	font->draw(batch, std::to_wstring((int)player->camera->position.z), 90, 30, FONT_STYLES::OUTLINE);
	font->draw(batch, L"fps:", 16, 42, FONT_STYLES::OUTLINE);
	font->draw(batch, std::to_wstring(fps), 44, 42, FONT_STYLES::OUTLINE);
	font->draw(batch, L"occlusion: "+std::to_wstring(occlusion), 16, 54, FONT_STYLES::OUTLINE);

    std::wstringstream stream;
	stream << std::hex << player->selectedVoxel.states;
	font->draw(batch, L"block-selected: "+std::to_wstring(player->selectedVoxel.id)+L" "+stream.str(), 16, 78, FONT_STYLES::OUTLINE);
	font->draw(batch, L"meshes: " + std::to_wstring(Mesh::meshesCount), 16, 102, FONT_STYLES::OUTLINE);
}

void HudRenderer::draw(Level* level, Assets* assets){
	uicamera->fov = Window::height;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	ShaderProgram* uishader = assets->getShader("ui");
    if (uishader == nullptr) {
        LOG_CRITICAL("The shader 'ui' could not be found in the assets");
        throw std::runtime_error("The shader 'ui' could not be found in the assets");
    }

	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());

	// Chosen block preview
	Texture* blocks = assets->getTexture("blocks");
    if (blocks == nullptr) {
        LOG_CRITICAL("The texture 'bloks' could not be found in the assets");
        throw std::runtime_error("The texture 'bloks' could not be found in the assets");
    }
	Texture* sprite = assets->getTexture("slot");
    if (sprite == nullptr) {
        LOG_CRITICAL("The texture 'slot' could not be found in the assets");
        throw std::runtime_error("The texture 'slot' could not be found in the assets");
    }

	batch->texture(nullptr);
	batch->color = glm::vec4(1.0f);
	if (Events::_cursor_locked && !level->player->debug) {
		glLineWidth(2);
		batch->line(Window::width/2, Window::height/2-6, Window::width/2, Window::height/2+6, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(Window::width/2+6, Window::height/2, Window::width/2-6, Window::height/2, 0.2f, 0.2f, 0.2f, 1.0f);
		// batch->line(Window::width/2-5, Window::height/2-5, Window::width/2+5, Window::height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
		// batch->line(Window::width/2+5, Window::height/2-5, Window::width/2-5, Window::height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
	}

	batch->rect(Window::width/2-128-4, Window::height-80-4, 256+8, 64+8,
						0.95f, 0.95f, 0.95f, 0.85f, 0.85f, 0.85f,
						0.7f, 0.7f, 0.7f,
						0.55f, 0.55f, 0.55f, 0.45f, 0.45f, 0.45f, 4);
	batch->rect(Window::width/2-128, Window::height - 80, 256, 64,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 4);
	batch->rect(Window::width/2-32+2, Window::height - 80+2, 60, 60,
						0.45f, 0.45f, 0.45f, 0.55f, 0.55f, 0.55f,
						0.7f, 0.7f, 0.7f,
						0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f, 2);
	batch->rect(Window::width/2-32+4, Window::height - 80+4, 56, 56,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 2);

	batch->texture(blocks);
	Player* player = level->player;
	{
		Block* choosen_block = Block::blocks[player->choosenBlock].get();
		if (choosen_block->model == Block_models::CUBE){
			batch->blockSprite(Window::width/2-24, uicamera->fov - 72, 48, 48, 16, choosen_block->textureFaces, glm::vec4(1.0f));
		} else if (choosen_block->model == Block_models::X){
			batch->sprite(Window::width/2-24, uicamera->fov - 72, 48, 48, 16, choosen_block->textureFaces[3], glm::vec4(1.0f));
		}
	}

	if (!Events::_cursor_locked) {
		uint size = 48;
		uint step = 64;
		uint inv_wm = step*10;
		uint inv_hm = step*8;
		uint inv_w = inv_wm - (step - size);
		uint inv_h = inv_hm - (step - size);
		int inv_x = (Window::width - (inv_w)) / 2;
		int inv_y = (Window::height - (inv_h)) / 2;
		int xs = (Window::width - inv_w + step)/2;
		int ys = (Window::height - inv_h + step)/2;
		if (Window::width > inv_w*3){
			inv_x = (Window::width + (inv_w)) / 2;
			inv_y = (Window::height - (inv_h)) / 2;
			xs = (Window::width + inv_w + step)/2;
			ys = (Window::height - inv_h + step)/2;
		}
		int x = 0;
		int y = 0;
		glm::vec4 tint = glm::vec4(1.0f);
		int mx = Events::x;
		int my = Events::y;
		uint count = (inv_w / step) * (inv_h / step) + 1;

		batch->texture(nullptr);
		batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.3f);
		batch->rect(0, 0, Window::width, Window::height);
		batch->rect(inv_x - 4, inv_y - 4, inv_w+8, inv_h+8,
						0.95f, 0.95f, 0.95f, 0.85f, 0.85f, 0.85f,
						0.7f, 0.7f, 0.7f,
						0.55f, 0.55f, 0.55f, 0.45f, 0.45f, 0.45f, 4);
		batch->rect(inv_x, inv_y, inv_w, inv_h,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 4);

		batch->color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
		for (uint i = 1; i < count; i++) {
			x = xs + step * ((i-1) % (inv_w / step));
			y = ys + step * ((i-1) / (inv_w / step));

			batch->rect(x-2, y-2, size+4, size+4,
						0.45f, 0.45f, 0.45f, 0.55f, 0.55f, 0.55f,
						0.7f, 0.7f, 0.7f,
						0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f, 2);
			batch->rect(x, y, size, size,
						0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 0.65f,
						0.65f, 0.65f, 0.65f,
						0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 2);
		}

		//front
		batch->texture(blocks);
		for (uint i = 1; i < count; i++) {
			Block* cblock = Block::blocks[i].get();
			if (cblock == nullptr) break;
			x = xs + step * ((i-1) % (inv_w / step));
			y = ys + step * ((i-1) / (inv_w / step));
			if (mx > x && mx < x + (int)size && my > y && my < y + (int)size) {
				tint.r *= 1.2f;
				tint.g *= 1.2f;
				tint.b *= 1.2f;
				if (Events::justClicked(GLFW_MOUSE_BUTTON_LEFT)) player->choosenBlock = i;
			} else {
				tint = glm::vec4(1.0f);
			}
			
			if (cblock->model == Block_models::CUBE){
				batch->blockSprite(x, y, size, size, 16, cblock->textureFaces, tint);
			} else if (cblock->model == Block_models::X){
				batch->sprite(x, y, size, size, 16, cblock->textureFaces[3], tint);
			}
		}
	}
}
