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
#include "voxels/ChunksController.h"
#include "Assets.h"
#include "objects/Player.h"
#include "declarations.h"
#include "world_render.h"

// Размеры окна по умолчанию
int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

constexpr float MAX_PITCH = glm::radians(89.0f); // Ограничесние вертикального поворота
constexpr float GRAVITY = 16.0f;
constexpr glm::vec3 SPAWNPOINT = {-320, 255, 32};
constexpr float CROUCH_SPEED_MUL = 0.25f;
constexpr float RUN_SPEED_MUL = 1.5f;
constexpr float FLIGHT_SPEED_MUL = 5.0f;
constexpr float DEFAULT_PLAYER_SPEED = 5.0f;
constexpr float JUMP_FORCE = 6.0f;
constexpr float CROUCH_SHIFT_Y = -0.2f;
constexpr float CROUCH_ZOOM = 0.9f;
constexpr float RUN_ZOOM = 1.1f;
constexpr float C_ZOOM = 0.1f;
constexpr float DEFAULT_AIR_DAMPING = 0.1f;
constexpr float PLAYER_NOT_ONGROUND_DAMPING = 10.0f;
constexpr float CAMERA_SHAKING_OFFSET = 0.025f;
constexpr float CAMERA_SHAKING_OFFSET_Y = 0.031f;
constexpr float CAMERA_SHAKING_SPEED = 1.6f;
constexpr float CAMERA_SHAKING_DELTA_K = 10.0f;
constexpr float ZOOM_SPEED = 16.0f;
constexpr float MOUSE_SENSITIVITY = 1.0f;

void write_world(WorldFiles* wfile, Chunks* chunks){
	for (uint i = 0; i < chunks->volume; ++i){
		Chunk* chunk = chunks->chunks[i];
		if (chunk == nullptr) continue;
		wfile->put((const char*)chunk->voxels, chunk->chunk_x, chunk->chunk_z);
	}

	wfile->write();
}

void close_world(WorldFiles* wfile, Chunks* chunks){
	delete chunks;
	delete wfile;
}

void update_controls(PhysicsSolver* physics, Chunks* chunks, Player* player, float delta){
	if (Events::justPressed(GLFW_KEY_TAB)) Events::toggleCursor();

	for (int i = 1; i < 9; ++i){
		if (Events::justPressed(GLFW_KEY_0 + i)) player->choosenBlock = i;
	}

	// Controls
	Camera* camera = player->camera;
	Hitbox* hitbox = player->hitbox;
	bool sprint = Events::isPressed(GLFW_KEY_LEFT_CONTROL);
	bool shift = Events::isPressed(GLFW_KEY_LEFT_SHIFT) && hitbox->grounded && !sprint;
	bool zoom = Events::isPressed(GLFW_KEY_C);

	float speed = player->speed;
	if (player->flight) speed *= FLIGHT_SPEED_MUL;
	int substeps = (int)(delta * 1000);
	substeps = (substeps <= 0 ? 1 : (substeps > 100 ? 100 : substeps));
	physics->step(chunks, hitbox, delta, substeps, shift, player->flight ? 0.0f : 1.0f);
	camera->position.x = hitbox->position.x;
	camera->position.y = hitbox->position.y + 0.5f;
	camera->position.z = hitbox->position.z;

	if (player->flight && hitbox->grounded) player->flight = false;
	
	player->interpVel = player->interpVel * (1.0f - delta * 5) + hitbox->velocity * delta * 0.1f;
	if (hitbox->grounded && player->interpVel.y < 0.0f) player->interpVel.y *= -30.0f;

	float factor = hitbox->grounded ? glm::length(glm::vec2(hitbox->velocity.x, hitbox->velocity.z)) : 0.0f;
	player->cameraShakingTimer += delta * factor * CAMERA_SHAKING_SPEED;
	float shakeTimer = player->cameraShakingTimer;
	player->cameraShaking = player->cameraShaking * (1.0f - delta * CAMERA_SHAKING_DELTA_K) + factor * delta * CAMERA_SHAKING_DELTA_K;
	camera->position += camera->right * glm::sin(shakeTimer) * CAMERA_SHAKING_OFFSET * player->cameraShaking;
	camera->position += camera->up * glm::abs(glm::cos(shakeTimer)) * CAMERA_SHAKING_OFFSET_Y * player->cameraShaking;
	camera->position -= min(player->interpVel * 0.05f, 1.0f);

	if (Events::justPressed(GLFW_KEY_F)){
		player->flight = !player->flight;
		if (player->flight){
			hitbox->velocity.y += 1;
			hitbox->grounded = false;
		}
	}

	float dt = glm::min(1.0f, delta * ZOOM_SPEED);
	if (dt > 1.0f) dt = 1.0f;
	float zoomValue = 1.0f;
	if (shift){
		speed *= CROUCH_SPEED_MUL;
		camera->position.y += CROUCH_SHIFT_Y;
		zoomValue = CROUCH_ZOOM;
	} else if (sprint){
		speed *= RUN_SPEED_MUL;
		zoomValue = RUN_ZOOM;
	}
	if (zoom) zoomValue *= C_ZOOM;
	camera->zoom = zoomValue * dt + camera->zoom * (1.0f - dt);

	if (Events::isPressed(GLFW_KEY_SPACE) && hitbox->grounded) hitbox->velocity.y = JUMP_FORCE;

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

	hitbox->linear_damping = DEFAULT_AIR_DAMPING;
	if (player->flight){
		hitbox->linear_damping = PLAYER_NOT_ONGROUND_DAMPING;
		hitbox->velocity.y *= 1.0f - delta * 9;
		if (Events::isPressed(GLFW_KEY_SPACE)) hitbox->velocity.y += speed * delta * 9;
		
		if (Events::isPressed(GLFW_KEY_LEFT_SHIFT)) hitbox->velocity.y -= speed * delta * 9;
	}
	if (glm::length(moveDirection) > 0.0f){
		moveDirection = glm::normalize(moveDirection);

		if (!hitbox->grounded) hitbox->linear_damping = PLAYER_NOT_ONGROUND_DAMPING;

		hitbox->velocity.x += moveDirection.x * speed * delta * 9;
		hitbox->velocity.z += moveDirection.z * speed * delta * 9;
	}

	if (Events::_cursor_locked){
		player->camY += -Events::deltaY / Window::height * 2 * MOUSE_SENSITIVITY;
		player->camX += -Events::deltaX / Window::height * 2 * MOUSE_SENSITIVITY;

		if (player->camY < -MAX_PITCH) player->camY = -MAX_PITCH;
		if (player->camY > MAX_PITCH) player->camY = MAX_PITCH;

		camera->rotation = glm::mat4(1.0f);
		camera->rotate(player->camY, player->camX, 0);
	}
}

void update_interaction(Chunks* chunks, PhysicsSolver* physics, Player* player, Lighting* lighting){
	Camera* camera = player->camera;
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
            lighting->onBlockSet(x, y, z, 0);
        }
        if (Events::justClicked(GLFW_MOUSE_BUTTON_2)){ // На ПКМ ставим блок
            int x = (int)(hitVoxelCoord.x) + (int)(hitNormal.x);
            int y = (int)(hitVoxelCoord.y) + (int)(hitNormal.y);
            int z = (int)(hitVoxelCoord.z) + (int)(hitNormal.z);
            if (!physics->isBlockInside(x,y,z, player->hitbox)){
                chunks->setVoxel(x, y, z, player->choosenBlock);
                lighting->onBlockSet(x, y, z, player->choosenBlock);
            }
        }
    }
}

int main() {
    setup_definitions();

    Window::initialize(WINDOW_WIDTH, WINDOW_HEIGHT, "ChromaForge"); // Инициализация окна
    Events::initialize(); // Инициализация системы событий

    std::cout << "INFO::Loading Assets" << std::endl;
    Assets* assets = new Assets();
    if(!initialize_assets(assets)) {
        std::cerr << "ERROR::Failed to load assets" << std::endl;
        delete assets;
        Window::terminate();
        return -1;
    }
    std::cout << "INFO::Assets uploaded successfully" << std::endl;

    std::cout << "INFO::Preparing world" << std::endl;
    Camera* camera = new Camera(SPAWNPOINT, glm::radians(90.0f));
    WorldFiles* wfile = new WorldFiles("../saves/world/", REGION_VOLUME * (CHUNK_VOLUME * 2 + 8));
    Chunks* chunks = new Chunks(32, 1, 32, 0, 0, 0);

    Player* player = new Player(glm::vec3(camera->position), DEFAULT_PLAYER_SPEED, camera);
    wfile->readPlayer(player);
	camera->rotation = glm::mat4(1.0f);
	camera->rotate(player->camY, player->camX, 0);
    std::cout << "INFO::The world is prepared" << std::endl;

    std::cout << "INFO::Preparing systems" << std::endl;
    VoxelRenderer renderer(1024*1024);
	PhysicsSolver physics(glm::vec3(0, -GRAVITY, 0));
	Lighting lighting(chunks);

    init_renderer();

	ChunksController chunksController(chunks, &lighting);

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

    bool occlusion = false;

    glfwSwapInterval(1);

    std::cout << "INFO::Systems is prepared" << std::endl;
    std::cout << "INFO::Initialization is finished" << std::endl;

    // Главный игровой цикл
    while (!Window::isShouldClose()) {
        frame++;

        // Расчет времени между кадрами
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Обработка ввода
        if (Events::justPressed(GLFW_KEY_ESCAPE)) Window::setShouldClose(true);

		if (Events::justPressed(GLFW_KEY_O)) occlusion = !occlusion;

		update_controls(&physics, chunks, player, deltaTime);
		update_interaction(chunks, &physics, player, &lighting);

		chunks->setCenter(wfile, camera->position.x, 0, camera->position.z);
		chunksController._buildMeshes(&renderer, frame);

		int freeLoaders = chunksController.countFreeLoaders();
		for (int i = 0; i < freeLoaders; ++i) {
			chunksController.loadVisible(wfile);
        }

		draw_world(camera, assets, chunks, occlusion);

        Window::swapBuffers(); // Обмен буферов
        Events::pollEvents(); // Обработка событий
    }

    // Сохранение мира
    std::cout << "INFO::World saving" << std::endl;
    wfile->writePlayer(player);
	write_world(wfile, chunks);
	close_world(wfile, chunks);
    std::cout << "INFO::The world has been successfully saved" << std::endl;

    // Завершение работы
    std::cout << "INFO::Shutting down" << std::endl;
    delete player;
	delete assets;
	finalize_renderer();
	Events::finalize();
	Window::terminate();

    return 0;
}
