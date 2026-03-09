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
#include "BlocksController.h"
#include "scripting/scripting.h"
#include "../items/Item.h"
#include "../items/ItemStack.h"
#include "../items/Inventory.h"

namespace CameraConsts {
	constexpr float SHAKE_OFFSET = 0.025f;
	constexpr float SHAKE_OFFSET_Y = 0.031f;
	constexpr float SHAKE_SPEED = 1.75f;
	constexpr float SHAKE_DELTA_K = 10.0f;
	constexpr float MAX_PITCH = 89.9f;
}

namespace ZoomConsts {
	constexpr float SPEED = 16.0f;
	constexpr float CROUCH = 0.9f;
	constexpr float RUN = 1.1f;
	constexpr float INPUT = 0.1f;
}

constexpr float CROUCH_SHIFT_Y = -0.2f;

CameraControl::CameraControl(Player* player, const CameraSettings& settings) : player(player), camera(player->camera), settings(settings), offset(0.0f, 0.7f, 0.0f), currentViewCamera(player->currentCamera) {
}

void CameraControl::refresh() {
	camera->position = player->hitbox->position + offset;
}

void CameraControl::updateMouse(PlayerInput& input) {
	float sensitivity = input.zoom ? settings.sensitivity / 4.f : settings.sensitivity;
	glm::vec2 &cam = player->cam;
    cam -= glm::degrees(Events::delta / (float)Window::height * sensitivity);

	if (cam.y < -CameraConsts::MAX_PITCH) cam.y = -CameraConsts::MAX_PITCH;
	else if (cam.y > CameraConsts::MAX_PITCH) cam.y = CameraConsts::MAX_PITCH;

	if (cam.x > 180.0f) cam.x -= 360.0f;
	else if (cam.x < -180.0f) cam.x += 360.0f;

	camera->rotation = glm::mat4(1.0f);
	camera->rotate(glm::radians(cam.y), glm::radians(cam.x), 0);
}

void CameraControl::update(PlayerInput& input, float delta, Chunks* chunks) {
	Hitbox* hitbox = player->hitbox.get();

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

	auto spCamera = player->spCamera;
    auto tpCamera = player->tpCamera;

	if (input.cameraMode) {
		if (player->currentCamera == camera) player->currentCamera = spCamera;
		else if (player->currentCamera == player->spCamera) player->currentCamera = tpCamera;
		else if (player->currentCamera == player->tpCamera) player->currentCamera = camera;
	}

	if (player->currentCamera == spCamera) {
		spCamera->position = chunks->rayCastToObstacle(camera->position, camera->front, 3.0f) - 0.2f * (camera->front);
		spCamera->dir = -camera->dir;
		spCamera->front = -camera->front;
	} else if (player->currentCamera == tpCamera) {
		tpCamera->position = chunks->rayCastToObstacle(camera->position, -camera->front, 3.0f) + 0.2f * (camera->front);
		tpCamera->dir = camera->dir;
		tpCamera->front = camera->front;
	}
}

glm::vec3 PlayerController::selectedBlockPosition;
int PlayerController::selectedBlockId = -1;
glm::vec3 PlayerController::selectedPointPosition;
glm::ivec3 PlayerController::selectedBlockNormal;
int PlayerController::selectedBlockStates = 0;

PlayerController::PlayerController(Level* level, const EngineSettings& settings, BlocksController* blocksController) : level(level), player(level->player), camControl(level->player, settings.camera), blocksController(blocksController) {
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
	input.cameraMode = Events::justActive(BIND_CAM_MODE);

	input.attack = Events::justActive(BIND_PLAYER_ATTACK);
	input.build = Events::justActive(BIND_PLAYER_BUILD);
	input.pickBlock = Events::justActive(BIND_PLAYER_PICK);
}

void PlayerController::updateCamera(float delta, bool movement) {
	if (movement) camControl.updateMouse(input);
	camControl.update(input, delta, level->chunks);
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
	input.cameraMode = false;
}

void PlayerController::updateControls(float delta){
	player->update(level, input, delta);
}

void PlayerController::updateInteraction(){
	const ContentIndices* contentIds = level->content->getIndices();
	Chunks* chunks = level->chunks;
	Player* player = level->player;
	Lighting* lighting = level->lighting;
	Camera* camera = player->camera.get();

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

		auto inventory = player->getInventory();
        ItemStack& stack = inventory->getSlot(player->getChosenSlot());
        Item* item = contentIds->getItemDef(stack.getItemId());
		Block* def = contentIds->getBlockDef(item->rt.placingBlock);
		if (def && def->rotatable) {
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

		if (input.attack && !input.crouch && item->rt.funcsset.on_block_break_by) {
			if (scripting::on_item_break_block(player, item, x, y, z)) return;
		}
		
		Block* block = contentIds->getBlockDef(vox->id);
		if (input.attack && block->breakable) blocksController->breakBlock(player, block, x, y, z);

		if (input.build && !input.crouch && item->rt.funcsset.on_use_on_block) {
			if (scripting::on_item_use_on_block(player, item, x, y, z)) return;
        }

		if (def && input.build) {
			if (block->rt.funcsset.oninteract && !input.crouch) {
                if (scripting::on_block_interact(player, block, x, y, z)) return;
            }
			if (!block->replaceable){
				x = iend.x + norm.x;
				y = iend.y + norm.y;
				z = iend.z + norm.z;
			} else if (def->rotations.name == "pipe") {
                states = BLOCK_DIR_UP;
            }
			vox = chunks->getVoxel(x, y, z);
			blockid_t chosenBlock = def->rt.id;
			if (vox && (block = contentIds->getBlockDef(vox->id))->replaceable) {
				if (!level->physics->isBlockInside(x, y, z, player->hitbox.get()) || !def->obstacle){
					Block* def = contentIds->getBlockDef(chosenBlock);
					if (def->grounded && !chunks->isSolidBlock(x, y - 1, z)) chosenBlock = 0;
					if (chosenBlock != vox->id && chosenBlock) {
						chunks->setVoxel(x, y, z, chosenBlock, states);
						lighting->onBlockSet(x, y, z, chosenBlock);
						if (def->rt.funcsset.onplaced) scripting::on_block_placed(player, def, x, y, z);
						blocksController->updateSides(x, y, z);
					}
				}
			}
		}

		if (input.pickBlock) {
			Block* block = contentIds->getBlockDef(chunks->getVoxel(x,y,z)->id);
			itemid_t id = block->rt.pickingItem;
			auto inventory = player->getInventory();
			size_t slotid = inventory->findSlotByItem(id, 0, 10);
			if (slotid == Inventory::npos) {
				slotid = player->getChosenSlot();
			} else {
				player->setChosenSlot(slotid);
			}
			ItemStack& stack = inventory->getSlot(slotid);
			if (stack.getItemId() != id) {
				stack.set(ItemStack(id, 1));
			}
		}
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
