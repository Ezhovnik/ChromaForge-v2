#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include <objects/Player.h>
#include <util/Clock.h>

class Camera;
class BlocksController;
class Level;
class Chunks;
class Block;
struct EngineSettings;
struct CameraSettings;
struct Hitbox;
class Input;

class CameraControl {
private:
	Player& player;
    std::shared_ptr<Camera> camera;
    const CameraSettings& settings;
    glm::vec3 offset;
    float shake = 0.0f;
    float shakeTimer = 0.0f;
    glm::vec3 interpVel {0.0f};

	glm::vec3 updateCameraShaking(const Hitbox& hitbox, float delta);

	void updateFov(
        const Hitbox& hitbox, PlayerInput input, float delta, bool effects
    );

	void switchCamera();
public:
	CameraControl(
		Player& player,
		const CameraSettings& settings
	);

	void updateMouse(PlayerInput& input, int windowHeight);
	void update(PlayerInput input, float delta, const Chunks& chunks);
	void refreshPosition();
    void refreshRotation();
};

class PlayerController {
private:
	Level& level;
	Player& player;
	PlayerInput input {};
	CameraControl camControl;
	BlocksController& blocksController;

	float interactionTimer = 0.0f;

	void updateKeyboard(const Input& inputEvents);
	void resetKeyboard();
	void updatePlayer(float deltaTime);
	void updateEntityInteraction(entityid_t eid, bool lclick, bool rclick);
	void updateInteraction(const Input& inputEvents, float deltaTime);

    float stepsTimer = 0.0f;
    void onFootstep(const Hitbox& hitbox);
    void updateFootsteps(float delta);

	void processRightClick(const Block& def, const Block& target);

	voxel* updateSelection(float maxDistance);
public:

	PlayerController(
		const EngineSettings& settings,
		Level& level,
		Player& player,
		BlocksController& blocksController
	);

	void update(float delta, const Input* inputEvents);
	void postUpdate(
		float delta, int windowHeight, const Input* inputEvents, bool pause
	);

	Player* getPlayer();
};
