#ifndef SRC_PLAYER_CONTROL_H_
#define SRC_PLAYER_CONTROL_H_

#include <glm/glm.hpp>

#include "../settings.h"

class Level;
class Player;

struct PlayerInput {
	bool zoom;
	bool moveForward;
	bool moveBack;
	bool moveRight;
	bool moveLeft;
	bool sprint;
	bool shift;
	bool cheat;
	bool jump;
	bool noclip;
	bool flight;
    bool breakBlock;
    bool setBlock;
    bool selectBlock;
};

class PlayerController {
private:
	Level* level;
    Player* player;
    PlayerInput input;
    const CameraSettings& camSettings;
public:
	glm::vec3 selectedBlockPosition;
    glm::vec3 cameraOffset {0.0f, 0.7f, 0.0f};
	int selectedBlockId = -1;

	PlayerController(Level* level, const EngineSettings& settings);

	void updateKeyboard();
	void resetKeyboard();
	void updateCameraControl();
	void updateControls(float deltaTime);
	void updateInteraction();
	void refreshCamera();
};

#endif // SRC_PLAYER_CONTROL_H_
