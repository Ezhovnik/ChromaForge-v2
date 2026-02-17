#ifndef LOGIC_PLAYERCONTROLLER_H_
#define LOGIC_PLAYERCONTROLLER_H_

#include <glm/glm.hpp>

#include "../settings.h"
#include "../objects/Player.h"

class Camera;
class Player;
class Level;

class CameraControl {
	Player* player;
	Camera* camera;
	const CameraSettings& settings;

	glm::vec3 offset;
	float shake = 0.0f;
	float shakeTimer = 0.0f;
	glm::vec3 interpVel {0.0f};
public:
	CameraControl(Player* player, const CameraSettings& settings);

	void updateMouse(PlayerInput& input);
	void update(PlayerInput& input, float delta);
	void refresh();
};

class PlayerController {
private:
	Level* level;
	Player* player;
	PlayerInput input;
	CameraControl camControl;

	void updateKeyboard();
	void updateCamera(float delta, bool movement);
	void resetKeyboard();
	void updateControls(float delta);
	void updateInteraction();
public:
	static glm::vec3 selectedBlockPosition;
	static glm::vec3 selectedBlockNormal;
	static glm::vec3 selectedPointPosition;
	static int selectedBlockStates;
	static int selectedBlockId;

	PlayerController(Level* level, const EngineSettings& settings);

	void update(float delta, bool input, bool pause);
};

#endif // LOGIC_PLAYERCONTROLLER_H_
