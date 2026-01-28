#include "world_render.h"

#include <vector>
#include <algorithm>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../window/Window.h"
#include "../window/Camera.h"
#include "../graphics/Mesh.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Texture.h"
#include "../graphics/LineBatch.h"
#include "../graphics/Batch3D.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/Block.h"
#include "../world/World.h"
#include "../world/Level.h"
#include "../objects/Player.h"
#include "../assets/Assets.h"
#include "../objects/player_control.h"
#include "../logger/Logger.h"
#include "../graphics/ChunksRenderer.h"
#include "../world/LevelEvents.h"
#include "../math/FrustumCulling.h"

inline constexpr glm::vec3 CLEAR_COLOR = {0.7f, 0.71f, 0.73f};
inline constexpr float GAMMA_VALUE = 1.6f;
inline constexpr glm::vec3 SKY_LIGHT_COLOR = {1.8f, 1.8f, 1.8f};
inline constexpr glm::vec3 FOG_COLOR = {0.7f, 0.71f, 0.73f};
inline constexpr float MAX_TORCH_LIGHT = 15.0f;
inline constexpr float TORCH_LIGHT_DIST = 6.0f;

WorldRenderer::WorldRenderer(Level* level, Assets* assets) : assets(assets), level(level) {
	lineBatch = new LineBatch(4096);
	batch3D = new Batch3D(1024);
	renderer = new ChunksRenderer(level);
    frustumCulling = new Frustum();

    level->events->listen(CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		renderer->unload(chunk);
	});
}

WorldRenderer::~WorldRenderer() {
	delete batch3D;
	delete lineBatch;
	delete renderer;
    delete frustumCulling;
}

// Отрисовывает один чанк
bool WorldRenderer::drawChunk(size_t index, Camera* camera, ShaderProgram* shader, bool occlusion){
	std::shared_ptr<Chunk> chunk = level->chunks->chunks[index];
	if (!chunk->isLighted()) return false;

	std::shared_ptr<Mesh> mesh = renderer->getOrRender(chunk.get());
	if (mesh == nullptr) return true;

	// Простой фрустум-каллинг (отсечение чанков позади камеры в 2D плоскости XZ)
	if (occlusion){
		glm::vec3 min(chunk->chunk_x * CHUNK_WIDTH, chunk->bottom, chunk->chunk_z * CHUNK_DEPTH);
		glm::vec3 max(chunk->chunk_x * CHUNK_WIDTH + CHUNK_WIDTH, chunk->top, chunk->chunk_z * CHUNK_DEPTH + CHUNK_DEPTH);

		if (!frustumCulling->IsBoxVisible(min, max)) return false;
	}

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(chunk->chunk_x*CHUNK_WIDTH, 0.0f, chunk->chunk_z*CHUNK_DEPTH+1));

	shader->uniformMatrix("u_model", model);

    glDisable(GL_MULTISAMPLE);
	mesh->draw();
    glEnable(GL_MULTISAMPLE);

    return true;
}

void WorldRenderer::draw(Camera* camera, bool occlusion, float fogFactor, float fogCurve){
    Chunks* chunks = level->chunks;

    Window::setBgColor(CLEAR_COLOR); // Цвет фона (светло-серый с голубым оттенком)
	Window::clear();

    Window::viewport(0, 0, Window::width, Window::height);

    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

    // Загрузка ресурсов с проверкой
	Texture* texture = assets->getTexture("blocks");
    if (texture == nullptr) {
        LOG_CRITICAL("The texture 'bloks' could not be found in the assets");
        throw std::runtime_error("The texture 'bloks' could not be found in the assets");
    }

	ShaderProgram* shader = assets->getShader("default");
    if (shader == nullptr) {
        LOG_CRITICAL("The shader 'default' could not be found in the assets");
        throw std::runtime_error("The shader 'default' could not be found in the assets");
    }
	ShaderProgram* linesShader = assets->getShader("lines");
    if (linesShader == nullptr) {
        LOG_CRITICAL("The shader 'lines' could not be found in the assets");
        throw std::runtime_error("The shader 'lines' could not be found in the assets");
    }

    // Рендеринг чанков
	shader->use();

	shader->uniformMatrix("u_proj", camera->getProjection());
	shader->uniformMatrix("u_view", camera->getView());

	shader->uniform1f("u_gamma", GAMMA_VALUE);

	shader->uniform3f("u_skyLightColor", SKY_LIGHT_COLOR);
	shader->uniform3f("u_fogColor", FOG_COLOR);
	shader->uniform1f("u_fogFactor", fogFactor);
    shader->uniform1f("u_fogCurve", fogCurve);
    shader->uniform3f("u_cameraPos", camera->position);

    Block* choosen_block = Block::blocks[level->player->choosenBlock];
    if (!level->player->noclip) {
        float multiplier = 0.6f;
        shader->uniform3f("u_torchlightColor",
                choosen_block->emission[0] / MAX_TORCH_LIGHT * multiplier,
                choosen_block->emission[1] / MAX_TORCH_LIGHT * multiplier,
                choosen_block->emission[2] / MAX_TORCH_LIGHT * multiplier);
    } else {
        shader->uniform3f("u_torchlightColor", 0, 0, 0);
    }
	shader->uniform1f("u_torchlightDistance", TORCH_LIGHT_DIST);

	texture->bind();

    // Собираем индексы чанков, которые нужно отрисовать
	std::vector<size_t> indices;
	for (size_t i = 0; i < chunks->volume; ++i){
		std::shared_ptr<Chunk> chunk = chunks->chunks[i];
		if (chunk == nullptr) continue;
		indices.push_back(i);
	}

    // Вычисляем позицию камеры в координатах чанков (для сортировки)
	float px = camera->position.x / (float)CHUNK_WIDTH;
	float pz = camera->position.z / (float)CHUNK_DEPTH;

    // Сортируем чанки по удаленности от камеры (от дальних к ближним)
	std::sort(indices.begin(), indices.end(), [this, chunks, px, pz](size_t i, size_t j) {
		std::shared_ptr<Chunk> a = chunks->chunks[i];
		std::shared_ptr<Chunk> b = chunks->chunks[j];
		return ((a->chunk_x + 0.5f - px)*(a->chunk_x + 0.5f - px) + (a->chunk_z + 0.5f - pz)*(a->chunk_z + 0.5f - pz) >
				(b->chunk_x + 0.5f - px)*(b->chunk_x + 0.5f - px) + (b->chunk_z + 0.5f - pz)*(b->chunk_z + 0.5f - pz));
	});

    // Отрисовываем все видимые чанки
    frustumCulling->update(camera->getProjView());
    chunks->visibleCount = 0;
	for (size_t i = 0; i < indices.size(); i++){
		chunks->visibleCount += drawChunk(indices[i], camera, shader, occlusion);
	}

    shader->uniformMatrix("u_model", glm::mat4(1.0f));

    // Рендеринг линий
    if (level->playerController->selectedBlockId != -1 && !level->player->noclip){
		Block* selectedBlock = Block::blocks[level->playerController->selectedBlockId];
		glm::vec3 pos = level->playerController->selectedBlockPosition;

        linesShader->use();
        linesShader->uniformMatrix("u_projview", camera->getProjView());
        glLineWidth(2.0f);

		if (selectedBlock->model == BlockModel::Cube){
			lineBatch->box(pos.x+0.5f, pos.y+0.5f, pos.z+0.5f, 1.005f,1.005f,1.005f, 0,0,0,0.5f);
		} else if (selectedBlock->model == BlockModel::X){
			lineBatch->box(pos.x+0.4f, pos.y+0.3f, pos.z+0.4f, 0.805f,0.805f,0.805f, 0,0,0,0.5f);
		}

        lineBatch->render();
	}

    if (level->player->debug) {
        float length = 40.0f;

		linesShader->use();
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(Window::width >> 1, -static_cast<int>(Window::height) >> 1, 0.f));
		linesShader->uniformMatrix("u_projview", glm::ortho(0.0f, static_cast<float>(Window::width), -static_cast<float>(Window::height), 0.f, -length, length) * model * glm::inverse(camera->rotation));

		glDisable(GL_DEPTH_TEST);

		glLineWidth(4.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 0.0f, 1.0f);
		lineBatch->render();

        glEnable(GL_DEPTH_TEST);

		glLineWidth(2.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		lineBatch->line(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, length, 0.0f, 0.0f, 1.0f, 1.0f);
		lineBatch->render();
	}
}
