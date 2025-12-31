#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GLM для работы с матрицами
#include <glm/glm.hpp>
#include <glm/ext.hpp>

// Пользовательские классы
#include "loaders/shader_loader.h"
#include "graphics/ShaderProgram.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "loaders/texture_loader.h"
#include "graphics/Texture.h"
#include "graphics/Mesh.h"
#include "voxels/voxel.h"
#include "voxels/Chunk.h"
#include "voxels/Chunks.h"
#include "graphics/VoxelRenderer.h"
#include "graphics/LineBatch.h"
#include "files/files.h"
#include "lighting/LightSolver.h"
#include "lighting/LightMap.h"
#include "lighting/Lighting.h"
#include "voxels/Block.h"
#include "files/WorldFiles.h"
#include "physics/Hitbox.h"
#include "physics/PhysicsSolver.h"
#include "voxels/WorldGenerator.h"

// Размеры окна по умолчанию
int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

constexpr float MAX_PITCH = glm::radians(89.0f); // Ограничесние вертикального поворота
constexpr float GRAVITY = 16.0f;
constexpr glm::vec3 SPAWNPOINT = glm::vec3(32, 32, 32);
constexpr float CROUCH_SPEED_MUL = 0.25f;
constexpr float RUN_SPEED_MUL = 1.5f;
constexpr float DEFAULT_PLAYER_SPEED = 5.0f;
constexpr float JUMP_FORCE = 6.0f;
constexpr float CROUCH_SHIFT_Y = -0.2f;
constexpr float CROUCH_ZOOM = 0.9f;
constexpr float RUN_ZOOM = 1.1f;

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

// Указатели на игровые объекты
Mesh *crosshair_mesh; // Меш прицела
ShaderProgram *shader, *crosshair_shader, *lines_shader; // Шейдерные програмы
Texture *texture; // Текстурный атлас
LineBatch *lineBatch; // Буфер для пакетной отрисовки линий
Chunks* chunks; // Чанки
WorldFiles* wfile; // Объект для управления загрузкой мира

// Инициализирует блоки и их свойства
void setup_definitions() {
        // Воздух
        Block* block = new Block(0, 0);
        block->drawGroup = 1;
        block->lightPassing = true;
        block->obstacle = false;
        Block::blocks[block->id] = block;

        // Мох
        block = new Block(1, 1);
        Block::blocks[block->id] = block;

        // Земля
        block = new Block(2, 2);
        Block::blocks[block->id] = block;

        // Светокамень
        block = new Block(3, 3);
        block->emission[0] = 14;
        block->emission[1] = 12;
        block->emission[2] = 3;
        Block::blocks[block->id] = block;

        // Стекло
        block = new Block(4, 4);
        block->drawGroup = 2;
        block->lightPassing = true;
        Block::blocks[block->id] = block;

        // Доски
        block = new Block(5, 5);
        Block::blocks[block->id] = block;

        // Бревно
        block = new Block(6, 6);
        block->textureFaces[2] = 7;
        block->textureFaces[3] = 7;
        Block::blocks[block->id] = block;

        // Листва
        block = new Block(7, 8);
        block->drawGroup = 3;
        block->lightPassing = true;
        Block::blocks[block->id] = block;
}

// Инициализирует графические ресурсы (шейдеры и текстурный атлас)
bool initialize_assets() {
    // Загрузка шейдерной программы
    shader = loadShaderProgram("../res/shaders/default.vert", "../res/shaders/default.frag");
    if (shader == nullptr) {
        std::cerr << "Failed to load shader program" << std::endl;
        return false;
    }

    // Загрузка шейдерной программы прицела-указателя
    crosshair_shader = loadShaderProgram("../res/shaders/crosshair.vert", "../res/shaders/crosshair.frag");
    if (crosshair_shader == nullptr) {
        std::cerr << "Failed to load crosshair shader program" << std::endl;
        delete shader;
        return false;
    }

    // Загрузка шейдерной программы для отрисовки линий
    lines_shader = loadShaderProgram("../res/shaders/lines.vert", "../res/shaders/lines.frag");
    if (lines_shader == nullptr) {
        std::cerr << "Failed to load lines shader program" << std::endl;
        delete crosshair_shader;
        delete shader;
        return false;
    }

    // Загрузка текстурного атласа
    texture = loadTexture("../res/textures/atlas.png");
    if (texture == nullptr) {
        std::cerr << "Failed to load texture" << std::endl;
        delete lines_shader;
        delete crosshair_shader;
        delete shader;
        return false;
    }

    return true;
}

// Отрисовывает игровой мир
void draw_world(Camera* camera){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader->use();
	shader->uniformMatrix("u_projview", camera->getProjection() * camera->getView());
	shader->uniform1f("u_gamma", 1.6f);
	shader->uniform3f("u_skyLightColor", 0.1 * 2, 0.15 * 2, 0.2 * 2);
	texture->bind();
	glm::mat4 model;
	for (size_t i = 0; i < chunks->volume; ++i){
		Chunk* chunk = chunks->chunks[i];
		if (chunk == nullptr) continue;
		Mesh* mesh = chunks->meshes[i];
		if (mesh == nullptr) continue;
		model = glm::translate(glm::mat4(1.0f), glm::vec3(chunk->chunk_x * CHUNK_WIDTH + 0.5f, chunk->chunk_y * CHUNK_HEIGHT + 0.5f, chunk->chunk_z * CHUNK_DEPTH + 0.5f));
		shader->uniformMatrix("u_model", model);
		mesh->draw(GL_TRIANGLES);
	}

	crosshair_shader->use();
	crosshair_mesh->draw(GL_LINES);

	lines_shader->use();
	lines_shader->uniformMatrix("u_projview", camera->getProjection() * camera->getView());
	glLineWidth(2.0f);
	lineBatch->render();
}

// Освобождает графические ресурсы
void finalize_assets(){
	delete shader;
	delete texture;
	delete crosshair_mesh;
	delete crosshair_shader;
	delete lines_shader;
	delete lineBatch;
}

// Сохраняет мир в файл
void write_world(){
	for (uint i = 0; i < chunks->volume; ++i){
		Chunk* chunk = chunks->chunks[i];
		if (chunk == nullptr) continue;
		wfile->put((const char*)chunk->voxels, chunk->chunk_x, chunk->chunk_z);
	}

	wfile->write();
}

// Освобождает ресурсы мира
void close_world(){
	delete chunks;
	delete wfile;
}

int main() {
    setup_definitions();

    Window::initialize(WINDOW_WIDTH, WINDOW_HEIGHT, "ChromaForge"); // Инициализация окна
    Events::initialize(); // Инициализация системы событий

    if(!initialize_assets()) {
        std::cerr << "Failed to load assets" << std::endl;
        Window::terminate();
        return -1;
    }

    // Создание игровых объектов
    wfile = new WorldFiles("../saves/world/", REGION_VOLUME * (CHUNK_VOLUME * 2 + 8));
	chunks = new Chunks(32,1,32, 0,0,0);
	VoxelRenderer renderer(1024 * 1024);
	lineBatch = new LineBatch(4096);
	PhysicsSolver physics(glm::vec3(0, -GRAVITY, 0));

	Lighting::initialize(chunks);

	crosshair_mesh = new Mesh(crosshair_vertices, 4, crosshair_attrs);
	Camera* camera = new Camera(SPAWNPOINT, glm::radians(90.0f));
	Hitbox* hitbox = new Hitbox(SPAWNPOINT, glm::vec3(0.2f,0.9f,0.2f));

    glClearColor(0.6f, 0.62f, 0.65f, 1.0f); // Серый цвет фона

    glEnable(GL_DEPTH_TEST); // Включение теста глубины

    // Включение отсечения задних граней для оптимизации
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_BACK);
    
    glEnable(GL_BLEND); // Включение смешивания цветов
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Режим смешивания для прозрачности

    // Переменные для отслеживания времени
    float lastTime = glfwGetTime();
    float deltaTime = 0.0f;

    // Параметры управления
    float camX = 0.0f; // Угол поворота камеры по горизонтали
    float camY = 0.0f; // Угол поворота камеры по вертикали

    float playerSpeed = DEFAULT_PLAYER_SPEED;

    int choosenBlock = 1; // Идентификатор выбранного блока

	long frame = 0;

    glfwSwapInterval(1);

    // Главный игровой цикл
    while (!Window::isShouldClose()) {
        frame++;

        // Расчет времени между кадрами
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Обработка ввода
        if (Events::justPressed(GLFW_KEY_ESCAPE)) {
            Window::setShouldClose(true); // Закрытие окна после нажатия ESC
        }
        if (Events::justPressed(GLFW_KEY_TAB)) {
            Events::toggleCursor(); // Переключение режима курсора (заблокирован/разблокирован)
        }

        // Выбор блока с клавиш
        for (int i = 1; i < 8; i++){
			if (Events::justPressed(GLFW_KEY_0 + i)){
				choosenBlock = i;
			}
		}

        // Обработка движения игрока
        bool isSprinting = Events::isPressed(GLFW_KEY_LEFT_CONTROL);
		bool isCrouching = Events::isPressed(GLFW_KEY_LEFT_SHIFT) && hitbox->grounded && !isSprinting;
		float speed = playerSpeed;

        // Расчет подшагов для физики
		int substeps = (int)(deltaTime * 1000);
		substeps = (substeps <= 0 ? 1 : (substeps > 100 ? 100 : substeps));

        // Шаг физики
		physics.step(chunks, hitbox, deltaTime, substeps, isCrouching);

        // Обновление позиции камеры на основе хитбокса игрока
		camera->position.x = hitbox->position.x;
		camera->position.y = hitbox->position.y + 0.5f;
		camera->position.z = hitbox->position.z;

        // Плавное изменение зума при приседании/спринте
		float interpolationFactor = glm::min(1.0f, deltaTime * 16);
		if (isCrouching){
			speed *= CROUCH_SPEED_MUL;
			camera->position.y -= CROUCH_SHIFT_Y;
			camera->zoom = CROUCH_ZOOM * interpolationFactor + camera->zoom * (1.0f - interpolationFactor);
		} else if (isSprinting){
			speed *= RUN_SPEED_MUL;
			camera->zoom = RUN_ZOOM * interpolationFactor + camera->zoom * (1.0f - interpolationFactor);
		} else {
			camera->zoom = interpolationFactor + camera->zoom * (1.0f - interpolationFactor);
		}

        // Прыжок
		if (Events::isPressed(GLFW_KEY_SPACE) && hitbox->grounded){
			hitbox->velocity.y = JUMP_FORCE;
		}

        // Расчёт направления движения
        glm::vec3 moveDirection(0.0f);
		if (Events::isPressed(GLFW_KEY_W)){ // Вперед
			moveDirection.x += camera->front.x;
            moveDirection.z += camera->front.z;
		}
		if (Events::isPressed(GLFW_KEY_S)){ // Назад
			moveDirection.x -= camera->front.x;
            moveDirection.z -= camera->front.z;
		}
		if (Events::isPressed(GLFW_KEY_D)){ // Вправо
			moveDirection.x += camera->right.x;
			moveDirection.z += camera->right.z;
		}
		if (Events::isPressed(GLFW_KEY_A)){ // Влево
			moveDirection.x -= camera->right.x;
			moveDirection.z -= camera->right.z;
		}

		if (glm::length(moveDirection) > 0.0f){ // Нормализация направления
            moveDirection = glm::normalize(moveDirection);
        }
        // Применение скорости к хитбоксу
		hitbox->velocity.x = moveDirection.x * speed;
		hitbox->velocity.z = moveDirection.z * speed;

        // Обновление и рендеринг чанков
        chunks->setCenter(wfile, camera->position.x, 0, camera->position.z);
		chunks->_buildMeshes(&renderer);
		chunks->loadVisible(wfile);

        // Управление поворотом камеры мышью (только в заблокированном режиме)
        if (Events::_cursor_locked) {
            // Обновление углов поворота на основе движения мыши
            camY -= Events::deltaY / Window::height * 2; // Вертикальный поворот
            camX -= Events::deltaX / Window::height * 2; // Горизонтальный поворот

            // Ограничение вертикального поворота
            if (camY < -MAX_PITCH) {
                camY = -MAX_PITCH;
            } else if (camY > MAX_PITCH) {
                camY = MAX_PITCH;
            }

            // Применение поворота к камере
            camera->rotation = glm::mat4(1.0f);
            camera->rotate(camY, camX, 0);
        }

        // Логика взаимодействия с вокселями (разрушение/установка блоков)
        {
            glm::vec3 hitPoint; // Точка попадания луча
            glm::vec3 hitNormal; // Нормаль поверхности в точке попадания
            glm::vec3 hitVoxelCoord; // Координаты вокселя в точке попадания
            voxel* vox = chunks->rayCast(camera->position, camera->front, 10.0f, hitPoint, hitNormal, hitVoxelCoord);
            if (vox != nullptr) {
                // Рисуем обводку для блока, на который смотрит камера
                lineBatch->box(
                    hitVoxelCoord.x + 0.5f, hitVoxelCoord.y + 0.5f, hitVoxelCoord.z + 0.5f,
                    1.005f, 1.005f, 1.005f,
                    0, 0, 0, 1
                );

                if (Events::justClicked(GLFW_MOUSE_BUTTON_1)){ // На ЛКМ разрушаем блок
					int x = (int)hitVoxelCoord.x;
					int y = (int)hitVoxelCoord.y;
					int z = (int)hitVoxelCoord.z;
					chunks->setVoxel(x, y, z, 0);
                    Lighting::onBlockSet(x, y, z, 0);
				}
				if (Events::justClicked(GLFW_MOUSE_BUTTON_2)){ // На ПКМ ставим блок
					int x = (int)(hitVoxelCoord.x) + (int)(hitNormal.x);
					int y = (int)(hitVoxelCoord.y) + (int)(hitNormal.y);
					int z = (int)(hitVoxelCoord.z) + (int)(hitNormal.z);
					chunks->setVoxel(x, y, z, choosenBlock);
					Lighting::onBlockSet(x, y, z, choosenBlock);
				}
            }
        }


        draw_world(camera); // Отрисовка кадра

        Window::swapBuffers(); // Обмен буферов
        Events::pollEvents(); // Обработка событий
    }

    // Сохранение мира
    write_world();
	close_world();

    // Завершение работы
	Lighting::finalize();
	finalize_assets();
    Events::finalize();
	Window::terminate();

    return 0;
}
