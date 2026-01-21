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
#include "../world/LevelEvents.h"
#include "../objects/Player.h"
#include "../assets/Assets.h"
#include "../objects/player_control.h"
#include "../logger/Logger.h"
#include "../graphics/ChunksRenderer.h"

inline constexpr glm::vec4 CLEAR_COLOR = {0.7f, 0.71f, 0.73f, 1.0f};
inline constexpr float GAMMA_VALUE = 1.6f;
inline constexpr glm::vec3 SKY_LIGHT_COLOR = {1.8f, 1.8f, 1.8f};
inline constexpr glm::vec3 FOG_COLOR = {0.7f, 0.71f, 0.73f};
inline constexpr float FOG_FACTOR = 0.025f;
inline constexpr float MAX_TORCH_LIGHT = 15.0f;
inline constexpr float TORCH_LIGHT_DIST = 6.0f;

WorldRenderer::WorldRenderer(Level* level, Assets* assets) : assets(assets), level(level) {
	lineBatch = new LineBatch(4096);
	batch3D = new Batch3D(1024);
	renderer = new ChunksRenderer(level);
    level->events->listen(CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		renderer->unload(chunk);
	});
}

WorldRenderer::~WorldRenderer() {
	delete batch3D;
	delete lineBatch;
	delete renderer;
}

// Отрисовывает один чанк
bool WorldRenderer::drawChunk(size_t index, Camera* camera, ShaderProgram* shader, bool occlusion){
	std::shared_ptr<Chunk> chunk = level->chunks->chunks[index];
	if (!chunk->isLighted()) return false;

    std::shared_ptr<Mesh> mesh = renderer->getOrRender(chunk.get());
	if (mesh == nullptr) return false;

	// Простой фрустум-каллинг (отсечение чанков позади камеры в 2D плоскости XZ)
	if (occlusion){
		float y = camera->position.y + camera->front.y * CHUNK_HEIGHT * 0.5f;
		if (y < 0.0f) y = 0.0f;
		if (y > CHUNK_HEIGHT) y = CHUNK_HEIGHT;
		glm::vec3 v = glm::vec3(chunk->chunk_x * CHUNK_WIDTH, y, chunk->chunk_z * CHUNK_DEPTH) - camera->position;
		if (v.x * v.x + v.z * v.z > (CHUNK_WIDTH * 3) * (CHUNK_WIDTH * 3) && dot(camera->front, v) < 0.0f) return true;
	}

	glm::mat4 model = glm::translate(
        glm::mat4(1.0f), 
        glm::vec3(
            chunk->chunk_x * CHUNK_WIDTH, 
            0.0f, 
            chunk->chunk_z * CHUNK_DEPTH + 1.0f
        ));

	shader->uniformMatrix("u_model", model);

	mesh->draw();

    return false;
}

void WorldRenderer::draw(Camera* camera, bool occlusion, float fogFactor, float fogCurve){
    Chunks* chunks = level->chunks;

	glClearColor(CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, CLEAR_COLOR.a); // Цвет фона (светло-серый с голубым оттенком)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	shader->uniform3f("u_skyLightColor", SKY_LIGHT_COLOR.r, SKY_LIGHT_COLOR.g, SKY_LIGHT_COLOR.b);
	shader->uniform3f("u_fogColor", FOG_COLOR.r, FOG_COLOR.g, FOG_COLOR.b);
	shader->uniform1f("u_fogFactor", fogFactor);
    shader->uniform1f("u_fogCurve", fogCurve);
    shader->uniform3f("u_cameraPos", camera->position.x, camera->position.y, camera->position.z);

    Block* choosen_block = Block::blocks[level->player->choosenBlock].get();
	shader->uniform3f("u_torchlightColor",
			choosen_block->emission[0] / MAX_TORCH_LIGHT,
			choosen_block->emission[1] / MAX_TORCH_LIGHT,
			choosen_block->emission[2] / MAX_TORCH_LIGHT);
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

	std::sort(indices.begin(), indices.end(), [this, chunks, px, pz](size_t i, size_t j) {
		std::shared_ptr<Chunk> a = chunks->chunks[i];
		std::shared_ptr<Chunk> b = chunks->chunks[j];
		return ((a->chunk_x + 0.5f - px) * (a->chunk_x + 0.5f - px) + (a->chunk_z + 0.5f - pz) * (a->chunk_z + 0.5f - pz) >
				(b->chunk_x + 0.5f - px) * (b->chunk_x + 0.5f - px) + (b->chunk_z + 0.5f - pz) * (b->chunk_z + 0.5f - pz));
    });

    // Отрисовываем все видимые чанки
    int occludedChunks = 0;
	for (size_t i = 0; i < indices.size(); i++){
		occludedChunks += drawChunk(indices[i], camera, shader, occlusion);
	}

    shader->uniformMatrix("u_model", glm::mat4(1.0f));
    // batch3D->begin();
    // batch3D->render();

    // Рендеринг линий
    if (level->playerController->selectedBlockId != -1 && !level->player->noclip){
		Block* selectedBlock = Block::blocks[level->playerController->selectedBlockId].get();;
		glm::vec3 pos = level->playerController->selectedBlockPosition;

        linesShader->use();
	    linesShader->uniformMatrix("u_projview", camera->getProjection() * camera->getView());
        glLineWidth(2.0f);

		if (selectedBlock->model == Block_models::CUBE){
			lineBatch->box(pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f, 1.005f, 1.005f, 1.005f, 0, 0, 0, 0.5f);
		} else if (selectedBlock->model == Block_models::X){
			lineBatch->box(pos.x + 0.4f, pos.y + 0.3f, pos.z + 0.4f, 0.805f, 0.805f, 0.805f, 0, 0, 0, 0.5f);
		}

        lineBatch->render();
	}

    if (level->player->debug) {
		linesShader->use();
		linesShader->uniformMatrix("u_projview", camera->getProjection()*camera->getView());

		glm::vec3 point = glm::vec3(camera->position.x+camera->front.x,
						camera->position.y+camera->front.y,
						camera->position.z+camera->front.z);

		glDisable(GL_DEPTH_TEST);

		glLineWidth(4.0f);
		lineBatch->line(point.x, point.y, point.z,
						point.x+0.1f, point.y, point.z,
						0, 0, 0, 1);
		lineBatch->line(point.x, point.y, point.z,
						point.x, point.y, point.z+0.1f,
						0, 0, 0, 1);
		lineBatch->line(point.x, point.y, point.z,
						point.x, point.y+0.1f, point.z,
						0, 0, 0, 1);
		lineBatch->render();

		glLineWidth(2.0f);
		lineBatch->line(point.x, point.y, point.z,
						point.x+0.1f, point.y, point.z,
						1, 0, 0, 1);
		lineBatch->line(point.x, point.y, point.z,
						point.x, point.y, point.z+0.1f,
						0, 0, 1, 1);
		lineBatch->line(point.x, point.y, point.z,
						point.x, point.y+0.1f, point.z,
						0, 1, 0, 1);
		lineBatch->render();

		glEnable(GL_DEPTH_TEST);
	}
}
