#ifndef WORLD_RENDERER_CPP_
#define WORLD_RENDERER_CPP_

#include <vector>
#include <algorithm>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "window/Window.h"
#include "window/Camera.h"
#include "graphics/Mesh.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "graphics/LineBatch.h"
#include "voxels/Chunks.h"
#include "voxels/Chunk.h"
#include "Assets.h"

float _camera_cx;
float _camera_cz;
Chunks* _chunks;

Mesh* crosshairMesh;
LineBatch* lineBatch;

// Вершины прицела-указателя
const float crosshair_vertices[] = {
    -0.01f, -0.01f,
    0.01f, 0.01f,

    -0.01f, 0.01f,
    0.01f, -0.01f,
};

// Атрибуты вершин прицела-указателя
const int crosshair_attrs[] = {
    2, 0
};

constexpr glm::vec4 CLEAR_COLOR = {0.7f, 0.71f, 0.73f, 1.0f};
constexpr float GAMMA_VALUE = 1.6f;
constexpr glm::vec3 SKY_LIGHT_COLOR = {1.8f, 1.8f, 1.8f};
constexpr glm::vec3 FOG_COLOR = {0.7f, 0.71f, 0.73f};
constexpr float LINE_WIDTH = 2.0f;
constexpr float CROSSHAIR_SCALE_BASE = 1000.0f;

// Инициализирует рендерер мира
void init_renderer(){
	crosshairMesh = new Mesh(crosshair_vertices, 4, crosshair_attrs);
	lineBatch = new LineBatch(4096);
}

// Освобождает ресурсы, занятые рендерером
void finalize_renderer(){
	delete crosshairMesh;
	delete lineBatch;
}

// Отрисовывает один чанк
void draw_chunk(size_t index, Camera* camera, ShaderProgram* shader, bool occlusion){
	Chunk* chunk = _chunks->chunks[index];
	Mesh* mesh = _chunks->meshes[index];
	if (mesh == nullptr) return;

	// Простой фрустум-каллинг (отсечение чанков позади камеры в 2D плоскости XZ)
	if (occlusion){
		const float cameraX = camera->position.x;
		const float cameraZ = camera->position.z;

		const float camDirX = camera->dir.x;
		const float camDirZ = camera->dir.z;

		bool unoccluded = false;
		do {
			if ((chunk->chunk_x * CHUNK_WIDTH-cameraX)*camDirX + (chunk->chunk_z*CHUNK_DEPTH-cameraZ)*camDirZ >= 0.0){
				unoccluded = true; 
                break;
			}
			if (((chunk->chunk_x + 1) * CHUNK_WIDTH-cameraX)*camDirX + (chunk->chunk_z*CHUNK_DEPTH-cameraZ)*camDirZ >= 0.0){
				unoccluded = true; 
                break;
			}
			if (((chunk->chunk_x + 1) * CHUNK_WIDTH-cameraX)*camDirX + ((chunk->chunk_z+1)*CHUNK_DEPTH-cameraZ)*camDirZ >= 0.0){
				unoccluded = true; 
                break;
			}
			if ((chunk->chunk_x * CHUNK_WIDTH-cameraX)*camDirX + ((chunk->chunk_z+1)*CHUNK_DEPTH-cameraZ)*camDirZ >= 0.0){
				unoccluded = true; 
                break;
			}
		} while (false);
		if (!unoccluded) return;
	}

	glm::mat4 model = glm::translate(
        glm::mat4(1.0f), 
        glm::vec3(
            chunk->chunk_x * CHUNK_WIDTH + 0.5f, 
            chunk->chunk_y * CHUNK_HEIGHT + 0.5f, 
            chunk->chunk_z * CHUNK_DEPTH + 0.5f
        ));

	shader->uniformMatrix("u_model", model);
	mesh->draw(GL_TRIANGLES);
}

// Компаратор для сортировки чанков по удаленности от камеры.
bool chunks_comparator(size_t i, size_t j) {
	Chunk* a = _chunks->chunks[i];
	Chunk* b = _chunks->chunks[j];

    float distA = (a->chunk_x + 0.5f - _camera_cx) * (a->chunk_x + 0.5f - _camera_cx) + (a->chunk_z + 0.5f - _camera_cz) * (a->chunk_z + 0.5f - _camera_cz);
	float distB = (b->chunk_x + 0.5f - _camera_cx) * (b->chunk_x + 0.5f - _camera_cx) + (b->chunk_z + 0.5f - _camera_cz) * (b->chunk_z + 0.5f - _camera_cz);

    return distA > distB;
}


void draw_world(Camera* camera, Assets* assets, Chunks* chunks, bool occlusion){
	glClearColor(CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, CLEAR_COLOR.a); // Цвет фона (светло-серый с голубым оттенком)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_chunks = chunks;

    // Загрузка ресурсов с проверкой
	Texture* texture = assets->getTexture("blocks");
    if (texture == nullptr) {
        std::cerr << "ERROR::The texture 'bloks' could not be found in the assets" << std::endl;
        throw std::runtime_error("The texture 'bloks' could not be found in the assets");
    }

	ShaderProgram* shader = assets->getShader("default");
    if (shader == nullptr) {
        std::cerr << "ERROR::The shader 'default' could not be found in the assets" << std::endl;
        throw std::runtime_error("The shader 'default' could not be found in the assets");
    }
	ShaderProgram* crosshairShader = assets->getShader("crosshair");
    if (crosshairShader == nullptr) {
        std::cerr << "ERROR::The shader 'crosshair' could not be found in the assets" << std::endl;
        throw std::runtime_error("The shader 'crosshair' could not be found in the assets");
    }
	ShaderProgram* linesShader = assets->getShader("lines");
    if (linesShader == nullptr) {
        std::cerr << "ERROR::The shader 'lines' could not be found in the assets" << std::endl;
        throw std::runtime_error("The shader 'lines' could not be found in the assets");
    }

    // Рендеринг чанков
	shader->use();

	shader->uniformMatrix("u_proj", camera->getProjection());
	shader->uniformMatrix("u_view", camera->getView());

	shader->uniform1f("u_gamma", GAMMA_VALUE);

	shader->uniform3f("u_skyLightColor", SKY_LIGHT_COLOR.r, SKY_LIGHT_COLOR.g, SKY_LIGHT_COLOR.b);
	shader->uniform3f("u_fogColor", FOG_COLOR.r, FOG_COLOR.g, FOG_COLOR.b);
	shader->uniform3f("u_cameraPos", camera->position.x, camera->position.y, camera->position.z);

	texture->bind();

    // Собираем индексы чанков, которые нужно отрисовать
	std::vector<size_t> indices;
	for (size_t i = 0; i < chunks->volume; i++){
		Chunk* chunk = chunks->chunks[i];
		if (chunk == nullptr) continue;
		if (chunks->meshes[i] != nullptr) indices.push_back(i);
	}

    // Вычисляем позицию камеры в координатах чанков (для сортировки)
	float px = camera->position.x / (float)CHUNK_WIDTH;
	float pz = camera->position.z / (float)CHUNK_DEPTH;

	_camera_cx = px;
	_camera_cz = pz;

    // Сортируем чанки по удаленности от камеры (от дальних к ближним)
	std::sort(indices.begin(), indices.end(), chunks_comparator);

    // Отрисовываем все видимые чанки
	for (size_t i = 0; i < indices.size(); i++){
		draw_chunk(indices[i], camera, shader, occlusion);
	}

    // Отрисовка прицела
	crosshairShader->use();
	crosshairShader->uniform1f("u_ar", (float)Window::height / (float)Window::width);
	crosshairShader->uniform1f("u_scale", 1.0f / ((float)Window::height / CROSSHAIR_SCALE_BASE));
	crosshairMesh->draw(GL_LINES);

    // Рендеринг линий
	linesShader->use();
	linesShader->uniformMatrix("u_projview", camera->getProjection()*camera->getView());

	glLineWidth(LINE_WIDTH);

    // Отрисовываем отладочные линии (оси X и Z у ног игрока)
    // Красная линия: ось X
	lineBatch->line(camera->position.x, camera->position.y - 0.5f, camera->position.z, camera->position.x + 0.1f, camera->position.y - 0.5f, camera->position.z, 1, 0, 0, 1);
	// Синяя линия: ось Z
    lineBatch->line(camera->position.x, camera->position.y - 0.5f, camera->position.z, camera->position.x, camera->position.y - 0.5f, camera->position.z + 0.1f, 0, 0, 1, 1);

    lineBatch->render();
}

#endif // WORLD_RENDERER_CPP_
