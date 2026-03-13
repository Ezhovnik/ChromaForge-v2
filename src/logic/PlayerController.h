#ifndef LOGIC_PLAYERCONTROLLER_H_
#define LOGIC_PLAYERCONTROLLER_H_

#include <memory>

#include <glm/glm.hpp>

#include "../settings.h"
#include "../objects/Player.h"
#include "../interfaces/Object.h"

class Camera;
class BlocksController;
class Level;
class Chunks;

class CameraControl {
	std::shared_ptr<Player> player;
	std::shared_ptr<Camera> camera, currentViewCamera;
	const CameraSettings& settings;

	glm::vec3 offset;
	float shake = 0.0f;
	float shakeTimer = 0.0f;
	glm::vec3 interpVel {0.0f};
public:
	CameraControl(std::shared_ptr<Player> player, const CameraSettings& settings);

	void updateMouse(PlayerInput& input);
	void update(PlayerInput& input, float delta, Chunks* chunks);
	void refresh();
};

class PlayerController {
private:
	Level* level;
	std::shared_ptr<Player> player;
	PlayerInput input;
	CameraControl camControl;
	BlocksController* blocksController;

	void updateKeyboard();
	void updateCamera(float delta, bool movement);
	void resetKeyboard();
	void updateControls(float delta);
	void updateInteraction();
public:
	static glm::vec3 selectedBlockPosition;
	static glm::ivec3 selectedBlockNormal;
	static glm::vec3 selectedPointPosition;
	static int selectedBlockStates;
	static int selectedBlockId;

	PlayerController(Level* level, const EngineSettings& settings, BlocksController* blocksController);

	void update(float delta, bool input, bool pause);

	Player* getPlayer();
};

#endif // LOGIC_PLAYERCONTROLLER_H_
