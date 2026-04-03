#include "PlayerController.h"

#include <algorithm>

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
#include "../input_bindings.h"
#include "BlocksController.h"
#include "scripting/scripting.h"
#include "../items/Item.h"
#include "../items/ItemStack.h"
#include "../items/Inventory.h"
#include "../settings.h"

namespace CameraConsts {
	inline constexpr float SHAKE_OFFSET = 0.025f;
	inline constexpr float SHAKE_OFFSET_Y = 0.031f;
	inline constexpr float SHAKE_SPEED = 1.75f;
	inline constexpr float SHAKE_DELTA_K = 10.0f;
	inline constexpr float MAX_PITCH = 89.9f;
	inline constexpr float CROUCH_SHIFT_Y = -0.2f;
}

namespace ZoomConsts {
	inline constexpr float SPEED = 16.0f;
	inline constexpr float CROUCH = 0.9f;
	inline constexpr float RUN = 1.1f;
	inline constexpr float INPUT = 0.1f;
}

inline constexpr float STEPS_SPEED = 1.75f;

CameraControl::CameraControl(
	const std::shared_ptr<Player>& player, 
	const CameraSettings& settings
) : player(player), 
	camera(player->camera), 
	settings(settings), 
	offset(0.0f, 0.7f, 0.0f) {}

void CameraControl::refresh() {
	camera->position = player->hitbox->position + offset;
}

void CameraControl::updateMouse(PlayerInput& input) {
	float sensitivity = input.zoom ? settings.sensitivity.get() / 4.0f : settings.sensitivity.get();
	glm::vec3& cam = player->cam;

    auto cam_delta = glm::degrees(Events::delta / (float)Window::height * sensitivity);
    cam.x -= cam_delta.x;
    cam.y -= cam_delta.y;

	if (cam.y < -CameraConsts::MAX_PITCH) cam.y = -CameraConsts::MAX_PITCH;
	else if (cam.y > CameraConsts::MAX_PITCH) cam.y = CameraConsts::MAX_PITCH;

	if (cam.x > 180.0f) cam.x -= 360.0f;
	else if (cam.x < -180.0f) cam.x += 360.0f;

	camera->rotation = glm::mat4(1.0f);
	camera->rotate(glm::radians(cam.y), glm::radians(cam.x), glm::radians(cam.z));
}

glm::vec3 CameraControl::updateCameraShaking(float delta) {
    glm::vec3 offset {};
    auto hitbox = player->hitbox.get();
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
    return offset;
}

void CameraControl::updateFovEffects(const PlayerInput& input, float delta) {
    auto hitbox = player->hitbox.get();
    bool crouch = input.crouch && hitbox->grounded && !input.sprint;

    float dt = fmin(1.0f, delta * ZoomConsts::SPEED);
    float zoomValue = 1.0f;
    if (crouch){
        offset += glm::vec3(0.0f, CameraConsts::CROUCH_SHIFT_Y, 0.0f);
        zoomValue = ZoomConsts::CROUCH;
    } else if (input.sprint){
        zoomValue = ZoomConsts::RUN;
    }
    if (input.zoom) zoomValue *= ZoomConsts::INPUT;
    camera->zoom = zoomValue * dt + camera->zoom * (1.0f - dt);
}

void CameraControl::switchCamera() {
    const std::vector<std::shared_ptr<Camera>> playerCameras {
        camera, player->tpCamera, player->spCamera
    };

    auto index = std::distance(
        playerCameras.begin(),
        std::find_if(
            playerCameras.begin(), 
            playerCameras.end(), 
            [=](auto ptr) {
                return ptr.get() == player->currentCamera.get();
            }
        )
    );
    if (static_cast<size_t>(index) != playerCameras.size()) {
        index = (index + 1) % playerCameras.size();
        player->currentCamera = playerCameras.at(index);
    }
}

void CameraControl::update(const PlayerInput& input, float delta, Chunks* chunks) {
	offset = glm::vec3(0.0f, 0.7f, 0.0f);

    if (settings.shaking.get() && !input.cheat) offset += updateCameraShaking(delta);
    if (settings.fovEffects.get())updateFovEffects(input, delta);
    if (input.cameraMode) switchCamera();

    auto spCamera = player->spCamera;
    auto tpCamera = player->tpCamera;

    if (player->currentCamera == spCamera) {
        spCamera->position = chunks->rayCastToObstacle(camera->position, camera->front, 3.0f) - 0.2f * camera->front;
        spCamera->dir = -camera->dir;
        spCamera->front = -camera->front;
    }

    else if (player->currentCamera == tpCamera) {
        tpCamera->position = chunks->rayCastToObstacle(camera->position, -camera->front, 3.0f) + 0.2f * camera->front;
        tpCamera->dir = camera->dir;
        tpCamera->front = camera->front;
    }
}

int PlayerController::selectedBlockId = -1;
glm::vec3 PlayerController::selectedPointPosition;
glm::ivec3 PlayerController::selectedBlockNormal;
int PlayerController::selectedBlockRotation = 0;

PlayerController::PlayerController(
	Level* level, 
	const EngineSettings& settings, 
	BlocksController* blocksController
) : level(level), 
	player(level->getObject<Player>(0)), 
	camControl(player, settings.camera), 
	blocksController(blocksController) {}

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
	camControl.update(input, delta, level->chunks.get());
}

void PlayerController::onBlockInteraction(glm::ivec3 pos, const Block* def, BlockInteraction type) {
    for (const auto& callback : blockInteractionCallbacks) {
        callback(player.get(), pos, def, type);
    }
}

void PlayerController::onFootstep() {
    auto hitbox = player->hitbox.get();
    glm::vec3 pos = hitbox->position;
    glm::vec3 half = hitbox->halfsize;

    for (int offsetZ = -1; offsetZ <= 1; ++offsetZ) {
        for (int offsetX = -1; offsetX <= 1; ++offsetX) {
            int x = std::floor(pos.x + half.x * offsetX);
            int y = std::floor(pos.y - half.y * 1.1f);
            int z = std::floor(pos.z + half.z * offsetZ);
            auto vox = level->chunks->getVoxel(x, y, z);
            if (vox) {
                auto def = level->content->getIndices()->getBlockDef(vox->id);
                if (!def->obstacle) continue;
                onBlockInteraction(
                    glm::ivec3(x, y, z), def,
                    BlockInteraction::Step
                );
                return;
            }
        }
    }
}

void PlayerController::updateFootsteps(float delta) {
    auto hitbox = player->hitbox.get();

    if (hitbox->grounded) {
        const glm::vec3& vel = hitbox->velocity;
        float f = glm::length(glm::vec2(vel.x, vel.z));
        stepsTimer += delta * f * STEPS_SPEED;
        if (stepsTimer >= PI) {
            stepsTimer = fmod(stepsTimer, PI);
            onFootstep();
        }
    } else {
        stepsTimer = PI;
    }
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
	player->updateInput(level, input, delta);
}

static int determine_rotation(Block* def, const glm::ivec3& norm, glm::vec3& camDir) {
    if (def && def->rotatable){
        const std::string& name = def->rotations.name;
        if (name == "pipe") {
            if (norm.x < 0.0f) return BLOCK_DIR_WEST;
            else if (norm.x > 0.0f) return BLOCK_DIR_EAST;
            else if (norm.y > 0.0f) return BLOCK_DIR_UP;
            else if (norm.y < 0.0f) return BLOCK_DIR_DOWN;
            else if (norm.z > 0.0f) return BLOCK_DIR_NORTH;
            else if (norm.z < 0.0f) return BLOCK_DIR_SOUTH;
        } 
        else if (name == "pane") {
            if (abs(camDir.x) > abs(camDir.z)){
                if (camDir.x > 0.0f) return BLOCK_DIR_EAST;
                if (camDir.x < 0.0f) return BLOCK_DIR_WEST;
            }
            if (abs(camDir.x) < abs(camDir.z)){
                if (camDir.z > 0.0f) return BLOCK_DIR_SOUTH;
                if (camDir.z < 0.0f) return BLOCK_DIR_NORTH;
            }
        }
    }
    return 0;
}

static void pick_block(const ContentIndices* indices, Chunks* chunks, Player* player, int x, int y, int z) {
    Block* block = indices->getBlockDef(chunks->getVoxel(x, y, z)->id);
    itemid_t id = block->rt.pickingItem;
    auto inventory = player->getInventory();
    size_t slotid = inventory->findSlotByItem(id, 0, 10);
    if (slotid == Inventory::npos) {
        slotid = player->getChosenSlot();
    } else {
        player->setChosenSlot(slotid);
    }
    ItemStack& stack = inventory->getSlot(slotid);
    if (stack.getItemId() != id) stack.set(ItemStack(id, 1));
}

void PlayerController::updateInteraction(){
	const ContentIndices* contentIds = level->content->getIndices();
	Chunks* chunks = level->chunks.get();
	Lighting* lighting = level->lighting.get();
	Camera* camera = player->camera.get();

	auto inventory = player->getInventory();
    const ItemStack& stack = inventory->getSlot(player->getChosenSlot());
    Item* item = contentIds->getItemDef(stack.getItemId());

	glm::vec3 end;
	glm::ivec3 iend;
	glm::ivec3 norm;

	voxel* vox = chunks->rayCast(camera->position, camera->front, 10.0f, end, norm, iend);
	if (vox != nullptr) {
        blockstate selectedState = vox->state;
		player->selectedVoxel = *vox;
		selectedBlockId = vox->id;
		selectedBlockRotation = vox->state.rotation;
        player->actualSelectedBlockPosition = iend;
        if (selectedState.segment) {
            iend = chunks->seekOrigin(
                iend, contentIds->getBlockDef(selectedBlockId), selectedState
            );
        } 
		player->selectedBlockPosition = iend;
		selectedPointPosition = end;
		selectedBlockNormal = norm;
		int x = iend.x;
		int y = iend.y;
		int z = iend.z;

		Block* def = contentIds->getBlockDef(item->rt.placingBlock);
		blockstate state {};
        state.rotation = determine_rotation(def, norm, camera->dir);

		if (input.attack && !input.crouch && item->rt.funcsset.on_block_break_by) {
			if (scripting::on_item_break_block(player.get(), item, x, y, z)) return;
		}

		Block* target = contentIds->getBlockDef(vox->id);
		if (input.attack && target->breakable) {
			onBlockInteraction(
                glm::ivec3(x, y, z), target,
                BlockInteraction::Destruction
            );
			blocksController->breakBlock(player.get(), target, x, y, z);
		}

		if (input.build && !input.crouch) {
			bool preventDefault = false;
            if (item->rt.funcsset.on_use_on_block) {
                preventDefault = scripting::on_item_use_on_block(player.get(), item, x, y, z);
            } else if (item->rt.funcsset.on_use) {
                preventDefault = scripting::on_item_use(player.get(), item);
            }
            if (preventDefault) {
                return;
            }
        }

		if (def && input.build) {
            iend = player->actualSelectedBlockPosition;
			if (target->rt.funcsset.oninteract && !input.crouch) {
                if (scripting::on_block_interact(player.get(), target, x, y, z)) return;
            }
			if (!target->replaceable) {
				x = iend.x + norm.x;
				y = iend.y + norm.y;
				z = iend.z + norm.z;
			} else if (def->rotations.name == "pipe") {
                state.rotation = BLOCK_DIR_UP;
                x = iend.x;
                y = iend.y;
                z = iend.z;
            }
			vox = chunks->getVoxel(x, y, z);
			blockid_t chosenBlock = def->rt.id;
			if (vox && (target = contentIds->getBlockDef(vox->id))->replaceable) {
				if (!level->physics->isBlockInside(x, y, z, def, state, player->hitbox.get()) || !def->obstacle) {
					Block* def = contentIds->getBlockDef(chosenBlock);
					if (def->grounded && !chunks->isSolidBlock(x, y - 1, z)) chosenBlock = 0;
					if (chosenBlock != vox->id && chosenBlock) {
						onBlockInteraction(
                            glm::ivec3(x, y, z), def,
                            BlockInteraction::Placing
                        );
						chunks->setVoxel(x, y, z, chosenBlock, state);
						lighting->onBlockSet(x, y, z, chosenBlock);
						if (def->rt.funcsset.onplaced) scripting::on_block_placed(player.get(), def, x, y, z);
						blocksController->updateSides(x, y, z);
					}
				}
			}
		}

		if (input.pickBlock) pick_block(contentIds, chunks, player.get(), x, y, z);
	} else {
		selectedBlockRotation = 0;
		selectedBlockId = -1;
        player->selectedVoxel.id = BLOCK_VOID;
        if (input.build) {
            if (item->rt.funcsset.on_use) {
                scripting::on_item_use(player.get(), item);
            }
        }
    }
}

void PlayerController::update(float delta, bool input, bool pause) {
	if (!pause) {
		if (input) updateKeyboard();
		else resetKeyboard();

		updateFootsteps(delta);
        updateCamera(delta, input);
		updateControls(delta);
	}
	camControl.refresh();

	if (input) {
		updateInteraction();
	} else {
		selectedBlockId = -1;
		selectedBlockRotation = 0;
	}
}

Player* PlayerController::getPlayer() {
    return player.get();
}

void PlayerController::listenBlockInteraction(
    const on_block_interaction& callback
) {
    blockInteractionCallbacks.push_back(callback);
}
