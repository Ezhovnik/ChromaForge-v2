#include "PlayerController.h"

#include "../objects/Player.h"
#include "../physics/PhysicsSolver.h"
#include "../physics/Hitbox.h"
#include "../lighting/Lighting.h"
#include "../world/Level.h"
#include "../content/Content.h"
#include "../voxels/Block.h"
#include "../voxels/voxel.h"
#include "../voxels/Chunks.h"
#include "../window/Camera.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../core_defs.h"

namespace CameraConsts {
	constexpr float SHAKE_OFFSET = 0.025f;
	constexpr float SHAKE_OFFSET_Y = 0.031f;
	constexpr float SHAKE_SPEED = 1.75f;
	constexpr float SHAKE_DELTA_K = 10.0f;
}

namespace ZoomConsts {
	constexpr float SPEED = 16.0f;
	constexpr float CROUCH = 0.9f;
	constexpr float RUN = 1.1f;
	constexpr float INPUT = 0.1f;
}

constexpr float MAX_PITCH = glm::radians(89.0f);
constexpr float CROUCH_SHIFT_Y = -0.2f;

CameraControl::CameraControl(Player* player, const CameraSettings& settings) : player(player), camera(player->camera), settings(settings), offset(0.0f, 0.7f, 0.0f) {
}

void CameraControl::refresh() {
	camera->position = player->hitbox->position + offset;
}

void CameraControl::updateMouse(PlayerInput& input) {
	float sensitivity = settings.sensitivity;

	float rotX = -Events::deltaX / Window::height * sensitivity;
	float rotY = -Events::deltaY / Window::height * sensitivity;

	if (input.zoom){
		rotX /= 4;
		rotY /= 4;
	}

	float& camX = player->camX;
	float& camY = player->camY;
	camX += rotX;
	camY += rotY;

	if (camY < -MAX_PITCH) camY = -MAX_PITCH;
	if (camY > MAX_PITCH) camY = MAX_PITCH;

	camera->rotation = glm::mat4(1.0f);
	camera->rotate(camY, camX, 0);
}

void CameraControl::update(PlayerInput& input, float delta) {
	Hitbox* hitbox = player->hitbox;

	offset = glm::vec3(0.0f, 0.7f, 0.0f);

	if (settings.shaking && !input.cheat) {
		const float k = CameraConsts::SHAKE_DELTA_K;
		const float oh = CameraConsts::SHAKE_OFFSET;
		const float ov = CameraConsts::SHAKE_OFFSET_Y;
		const glm::vec3& vel = hitbox->velocity;

		interpVel = interpVel * (1.0f - delta * 5) + vel * delta * 0.1f;
		if (hitbox->grounded && interpVel.y < 0.0f) interpVel.y *= -30.0f;
		shake = shake * (1.0f - delta * k);
		if (hitbox->grounded) {
			float f = glm::length(glm::vec2(vel.x, vel.z));
			shakeTimer += delta * f * CameraConsts::SHAKE_SPEED;
			shake += f * delta * k;
		}
		offset += camera->right * glm::sin(shakeTimer) * oh * shake;
		offset += camera->up * glm::abs(glm::cos(shakeTimer)) * ov * shake;
		offset -= glm::min(interpVel * 0.05f, 1.0f);
	}

	if (settings.fovEvents){
		bool crouch = input.crouch && hitbox->grounded && !input.sprint;

		float dt = fmin(1.0f, delta * ZoomConsts::SPEED);
		float zoomValue = 1.0f;
		if (crouch){
			offset += glm::vec3(0.f, CROUCH_SHIFT_Y, 0.f);
			zoomValue = ZoomConsts::CROUCH;
		} else if (input.sprint){
			zoomValue = ZoomConsts::RUN;
		}
		if (input.zoom) zoomValue *= ZoomConsts::INPUT;
		camera->zoom = zoomValue * dt + camera->zoom * (1.0f - dt);
	}
}

glm::vec3 PlayerController::selectedBlockPosition;
int PlayerController::selectedBlockId = -1;
glm::vec3 PlayerController::selectedPointPosition;
glm::ivec3 PlayerController::selectedBlockNormal;
int PlayerController::selectedBlockStates = 0;

PlayerController::PlayerController(Level* level, const EngineSettings& settings) : level(level), player(level->player), camControl(level->player, settings.camera) {
}

void PlayerController::updateKeyboard() {
	input.moveForward = Events::isActive(BIND_MOVE_FORWARD);
	input.moveBack = Events::isActive(BIND_MOVE_BACK);
	input.moveLeft = Events::isActive(BIND_MOVE_LEFT);
	input.moveRight = Events::isActive(BIND_MOVE_RIGHT);
	input.sprint = Events::isActive(BIND_MOVE_SPRINT);
	input.crouch = Events::isActive(BIND_MOVE_CROUCH);
	input.cheat = Events::isActive(BIND_MOVE_CHEAT);
	input.jump = Events::isActive(BIND_MOVE_JUMP);
	input.zoom = Events::isActive(BIND_CAM_ZOOM);

	input.noclip = Events::justActive(BIND_PLAYER_NOCLIP);
	input.flight = Events::justActive(BIND_PLAYER_FLIGHT);

	input.attack = Events::justActive(BIND_PLAYER_ATTACK);
	input.build = Events::justActive(BIND_PLAYER_BUILD);
	input.pickBlock = Events::justActive(BIND_PLAYER_PICK);

	for (int i = 1; i < 10; ++i){
		if (Events::justPressed(keycode::NUM_0 + i)) player->choosenBlock = i;
	}
}

void PlayerController::updateCamera(float delta, bool movement) {
	if (movement) camControl.updateMouse(input);
	camControl.update(input, delta);
}

void PlayerController::resetKeyboard() {
	input.zoom = false;
	input.moveForward = false;
	input.moveBack = false;
	input.moveLeft = false;
	input.moveRight = false;
	input.sprint = false;
	input.crouch = false;
	input.cheat = false;
	input.jump = false;
	input.attack = false;
	input.build = false;
	input.pickBlock = false;
}

void PlayerController::updateControls(float delta){
	player->update(level, input, delta);
}

void PlayerController::updateInteraction(){
	const ContentIndices* contentIds = level->content->indices;
	Chunks* chunks = level->chunks;
	Player* player = level->player;
	Lighting* lighting = level->lighting;
	Camera* camera = player->camera;

	glm::vec3 end;
	glm::ivec3 iend;
	glm::ivec3 norm;

	voxel* vox = chunks->rayCast(camera->position, camera->front, 10.0f, end, norm, iend);
	if (vox != nullptr){
		player->selectedVoxel = *vox;
		selectedBlockId = vox->id;
		selectedBlockStates = vox->states;
		selectedBlockPosition = iend;
		selectedPointPosition = end;
		selectedBlockNormal = norm;
		int x = iend.x;
		int y = iend.y;
		int z = iend.z;
		uint8_t states = 0;

		Block* def = contentIds->getBlockDef(player->choosenBlock);
		if (def->rotatable){
			const std::string& name = def->rotations.name;
			if (name == "pipe") {
				if (norm.x < 0.0f) states = BLOCK_DIR_WEST;
				else if (norm.x > 0.0f) states = BLOCK_DIR_EAST;
				else if (norm.y > 0.0f) states = BLOCK_DIR_UP;
				else if (norm.y < 0.0f) states = BLOCK_DIR_DOWN;
				else if (norm.z > 0.0f) states = BLOCK_DIR_NORTH;
				else if (norm.z < 0.0f) states = BLOCK_DIR_SOUTH;
			} else if (name == "pane") {
				glm::vec3 vec = camera->dir;
				if (abs(vec.x) > abs(vec.z)){
					if (vec.x > 0.0f) states = BLOCK_DIR_EAST;
					if (vec.x < 0.0f) states = BLOCK_DIR_WEST;
				}
				if (abs(vec.x) < abs(vec.z)){
					if (vec.z > 0.0f) states = BLOCK_DIR_SOUTH;
					if (vec.z < 0.0f) states = BLOCK_DIR_NORTH;
				}
			}
		}
		
		Block* block = contentIds->getBlockDef(vox->id);
		if (input.attack && block->breakable){
			chunks->setVoxel(x, y, z, 0, 0);
			lighting->onBlockSet(x, y, z, 0);
		}
		if (input.build){
			if (block->model != BlockModel::X){
				x = iend.x + norm.x;
				y = iend.y + norm.y;
				z = iend.z + norm.z;
			}
			vox = chunks->getVoxel(x, y, z);
			if (vox && (block = contentIds->getBlockDef(vox->id))->replaceable) {
				if (!level->physics->isBlockInside(x, y, z, player->hitbox) || !def->obstacle){
					chunks->setVoxel(x, y, z, player->choosenBlock, states);
					lighting->onBlockSet(x, y, z, player->choosenBlock);
				}
			}
		}
		if (input.pickBlock) player->choosenBlock = chunks->getVoxel(x, y, z)->id;
	} else {
		selectedBlockStates = 0;
		selectedBlockId = -1;
	}
}

void PlayerController::update(float delta, bool input, bool pause) {
	if (!pause) {
		if (input) updateKeyboard();
		else resetKeyboard();

        updateCamera(delta, input);
		updateControls(delta);

	}
	camControl.refresh();

	if (input) {
		updateInteraction();
	} else {
		selectedBlockId = -1;
		selectedBlockStates = 0;
	}
}
