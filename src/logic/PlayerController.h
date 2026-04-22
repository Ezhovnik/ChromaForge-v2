#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include <objects/Player.h>
#include <interfaces/Object.h>

class Camera;
class BlocksController;
class Level;
class Chunks;
class Block;
struct EngineSettings;
struct CameraSettings;
struct Hitbox;

class CameraControl {
private:
	std::shared_ptr<Player> player;
    std::shared_ptr<Camera> camera;
    const CameraSettings& settings;
    glm::vec3 offset;
    float shake = 0.0f;
    float shakeTimer = 0.0f;
    glm::vec3 interpVel {0.0f};

	glm::vec3 updateCameraShaking(const Hitbox& hitbox, float delta);

	void updateFovEffects(const Hitbox& hitbox, PlayerInput input, float delta);

	void switchCamera();
public:
	CameraControl(
		const std::shared_ptr<Player>& player,
		const CameraSettings& settings
	);

	void updateMouse(PlayerInput& input);
	void update(PlayerInput input, float delta, Chunks* chunks);
	void refresh();
};

class PlayerController {
private:
	Level* level;
	std::shared_ptr<Player> player;
	PlayerInput input {};
	CameraControl camControl;
	BlocksController* blocksController;

	void updateKeyboard();
	void resetKeyboard();
	void updatePlayer(float deltaTime);
	void updateEntityInteraction(entityid_t eid, bool lclick, bool rclick);
	void updateInteraction();

    float stepsTimer = 0.0f;
    void onFootstep(const Hitbox& hitbox);
    void updateFootsteps(float delta);

	void processRightClick(const Block& def, const Block& target);

	voxel* updateSelection(float maxDistance);
public:

	PlayerController(Level* level, const EngineSettings& settings, BlocksController* blocksController);

	void update(float delta, bool input_flag, bool pause);
	void postUpdate(float delta, bool input_flag, bool pause);

	Player* getPlayer();
};
