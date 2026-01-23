#include "player_control.h"

#include <GLFW/glfw3.h>

#include "Player.h"
#include "../physics/PhysicsSolver.h"
#include "../physics/Hitbox.h"
#include "../lighting/Lighting.h"
#include "../world/Level.h"
#include "../voxels/Block.h"
#include "../voxels/voxel.h"
#include "../voxels/Chunks.h"
#include "../window/Camera.h"
#include "../window/Events.h"
#include "../window/input.h"

namespace Player_Consts {
    constexpr float CROUCH_SPEED_MUL = 0.25f;
    constexpr float CROUCH_SHIFT_Y = -0.2f;
    constexpr float RUN_SPEED_MUL = 1.5f;
    constexpr float FLIGHT_SPEED_MUL = 5.0f;
    constexpr float JUMP_FORCE = 7.0f;
    constexpr float CROUCH_ZOOM = 0.9f;
    constexpr float RUN_ZOOM = 1.1f;
    constexpr float C_ZOOM = 0.1f;
    constexpr float GROUND_DAMPING = 10.0f;
    constexpr float AIR_DAMPING = 7.0f;
    constexpr float CAMERA_SHAKING_OFFSET = 0.025f;
    constexpr float CAMERA_SHAKING_OFFSET_Y = 0.031f;
    constexpr float CAMERA_SHAKING_SPEED = 1.6f;
    constexpr float CAMERA_SHAKING_DELTA_K = 10.0f;
    constexpr float ZOOM_SPEED = 16.0f;
    constexpr float CHEAT_SPEED_MUL = 5.0f;
    constexpr float MAX_PITCH = glm::radians(89.0f); // Ограничесние вертикального поворота
}

constexpr float MOUSE_SENSITIVITY = 1.0f;

PlayerController::PlayerController(Level* level, const EngineSettings& settings) 
	: level(level), player(level->player), camSettings(settings.camera) {
}

void PlayerController::refreshCamera() {
	level->player->camera->position = level->player->hitbox->position + cameraOffset;
}

void PlayerController::updateKeyboard() {
	input.zoom = Events::isPressed(keycode::C);
	input.moveForward = Events::isPressed(keycode::W);
	input.moveBack = Events::isPressed(keycode::S);
	input.moveLeft = Events::isPressed(keycode::A);
	input.moveRight = Events::isPressed(keycode::D);
	input.sprint = Events::isPressed(keycode::LEFT_CONTROL);
	input.shift = Events::isPressed(keycode::LEFT_SHIFT);
	input.cheat = Events::isPressed(keycode::R);
	input.jump = Events::isPressed(keycode::SPACE);

	input.noclip = Events::justPressed(keycode::N);
	input.flight = Events::justPressed(keycode::F);

	for (int i = 1; i < 10; i++){
		if (Events::justPressed(keycode::NUM_0 + i)) player->choosenBlock = i;
	}
}

void PlayerController::resetKeyboard() {
	input.zoom = false;
	input.moveForward = false;
	input.moveBack = false;
	input.moveLeft = false;
	input.moveRight = false;
	input.sprint = false;
	input.shift = false;
	input.cheat = false;
	input.jump = false;
}

void PlayerController::updateControls(float delta){
	Player* player = level->player;
	Camera* camera = player->camera;
	Hitbox* hitbox = player->hitbox;

	bool crouch = input.shift && hitbox->grounded && !input.sprint;
	float speed = player->speed;

	if (player->flight) speed *= Player_Consts::FLIGHT_SPEED_MUL;
	if (input.cheat) speed *= Player_Consts::CHEAT_SPEED_MUL;

    if (crouch) speed *= Player_Consts::CROUCH_SPEED_MUL;
    else if (input.sprint) speed *= Player_Consts::RUN_SPEED_MUL;

	glm::vec3 dir(0,0,0);
	if (input.moveForward){
		dir.x += camera->dir.x;
		dir.z += camera->dir.z;
	}
	if (input.moveBack){
		dir.x -= camera->dir.x;
		dir.z -= camera->dir.z;
	}
	if (input.moveRight){
		dir.x += camera->right.x;
		dir.z += camera->right.z;
	}
	if (input.moveLeft){
		dir.x -= camera->right.x;
		dir.z -= camera->right.z;
	}

	if (glm::length(dir) > 0.0f){
		dir = normalize(dir);

		hitbox->velocity.x += dir.x * speed * delta * 9;
		hitbox->velocity.z += dir.z * speed * delta * 9;
	}

	int substeps = (int)(delta * 1000);
	substeps = (substeps <= 0 ? 1 : (substeps > 100 ? 100 : substeps));
	level->physics->step(level->chunks, hitbox, delta, substeps, crouch, player->flight ? 0.0f : 1.0f, !player->noclip);
	if (player->flight && hitbox->grounded) {
		player->flight = false;
	}

	if (input.jump && hitbox->grounded) hitbox->velocity.y = Player_Consts::JUMP_FORCE;


	cameraOffset = glm::vec3(0.0f, 0.7f, 0.0f);

	if (camSettings.shaking) {
		player->interpVel = player->interpVel * (1.0f - delta * 5) + hitbox->velocity * delta * 0.1f;
		if (hitbox->grounded && player->interpVel.y < 0.0f) player->interpVel.y *= -30.0f;
		float factor = hitbox->grounded ? glm::length(glm::vec2(hitbox->velocity.x, hitbox->velocity.z)) : 0.0f;
		player->cameraShakingTimer += delta * factor * Player_Consts::CAMERA_SHAKING_SPEED;
		float shakeTimer = player->cameraShakingTimer;
		player->cameraShaking = player->cameraShaking * (1.0f - delta * Player_Consts::CAMERA_SHAKING_DELTA_K) + factor * delta * Player_Consts::CAMERA_SHAKING_DELTA_K;
		cameraOffset += camera->right * glm::sin(shakeTimer) * Player_Consts::CAMERA_SHAKING_OFFSET * player->cameraShaking;
		cameraOffset += camera->up * glm::abs(glm::cos(shakeTimer)) * Player_Consts::CAMERA_SHAKING_OFFSET_Y * player->cameraShaking;
		cameraOffset -= glm::min(player->interpVel * 0.05f, 1.0f);
	}

	if ((input.flight && !player->noclip) ||
		(input.noclip && player->flight == player->noclip)){
		player->flight = !player->flight;
		if (player->flight) hitbox->grounded = false;
	}
	if (input.noclip) player->noclip = !player->noclip;

	if (camSettings.fovEvents){
		float dt = glm::min(1.0f, delta * Player_Consts::ZOOM_SPEED);
		float zoomValue = 1.0f;
		if (crouch) {
			cameraOffset += Player_Consts::CROUCH_SHIFT_Y;
			zoomValue = Player_Consts::CROUCH_ZOOM;
		} else if (input.sprint){
			zoomValue = Player_Consts::RUN_ZOOM;
		}
		if (input.zoom) zoomValue *= Player_Consts::C_ZOOM;
    
		camera->zoom = zoomValue * dt + camera->zoom * (1.0f - dt);
	}

	hitbox->linear_damping = Player_Consts::GROUND_DAMPING;
	if (player->flight){
		hitbox->linear_damping = Player_Consts::AIR_DAMPING;
		hitbox->velocity.y *= 1.0f - delta * 9;
		if (input.jump) hitbox->velocity.y += speed * delta * 9;
		if (input.shift) hitbox->velocity.y -= speed * delta * 9;
	}

	if (!hitbox->grounded) hitbox->linear_damping = Player_Consts::AIR_DAMPING;

	input.noclip = false;
	input.flight = false;
}

void PlayerController::updateCameraControl() {
	Camera* camera = player->camera;
	float rotX = -Events::deltaX / Window::height * 2 * MOUSE_SENSITIVITY;
	float rotY = -Events::deltaY / Window::height * 2 * MOUSE_SENSITIVITY;
	if (input.zoom){
		rotX /= 4;
		rotY /= 4;
	}
	player->camX += rotX;
	player->camY += rotY;

	if (player->camY < -Player_Consts::MAX_PITCH) player->camY = -Player_Consts::MAX_PITCH;
	if (player->camY > Player_Consts::MAX_PITCH) player->camY = Player_Consts::MAX_PITCH;

	camera->rotation = glm::mat4(1.0f);
	camera->rotate(player->camY, player->camX, 0);
}

void PlayerController::updateInteraction(){
	Chunks* chunks = level->chunks;
	Player* player = level->player;
	Lighting* lighting = level->lighting;
	Camera* camera = player->camera;
	glm::vec3 end;
	glm::vec3 norm;
	glm::vec3 iend;
	voxel* vox = chunks->rayCast(camera->position, camera->front, 10.0f, end, norm, iend);
	if (vox != nullptr){
        player->selectedVoxel = *vox;
		selectedBlockId = vox->id;
		selectedBlockPosition = iend;

        int x = (int)iend.x;
		int y = (int)iend.y;
		int z = (int)iend.z;

		uint8_t states = 0;
        if (Block::blocks[player->choosenBlock].get()->rotatable){
			if (abs(norm.x) > abs(norm.z)){
				if (abs(norm.x) > abs(norm.y)) states = BLOCK_DIR_X;
				if (abs(norm.x) < abs(norm.y)) states = BLOCK_DIR_Y;
			}
			if (abs(norm.x) < abs(norm.z)){
				if (abs(norm.z) > abs(norm.y)) states = BLOCK_DIR_Z;
				if (abs(norm.z) < abs(norm.y)) states = BLOCK_DIR_Y;
			}
		}
		
		Block* block = Block::blocks[vox->id].get();
		if (Events::justClicked(mousecode::BUTTON_1) && block->breakable && !player->noclip){
			chunks->setVoxel(x, y, z, 0, 0);
			lighting->onBlockSet(x, y ,z, 0);
		}
		if (Events::justClicked(mousecode::BUTTON_2) && !player->noclip){
			if (block->model != BlockModel::X){
				x = (int)(iend.x) + (int)(norm.x);
                y = (int)(iend.y) + (int)(norm.y);
                z = (int)(iend.z) + (int)(norm.z);
			}
			if (!level->physics->isBlockInside(x,y,z, player->hitbox)){
				chunks->setVoxel(x, y, z, player->choosenBlock, states);
				lighting->onBlockSet(x,y,z, player->choosenBlock);
			}
		}
		if (Events::justClicked(mousecode::BUTTON_3)) player->choosenBlock = chunks->getVoxel(x,y,z)->id;
	} else {
		selectedBlockId = -1;
	}
}
