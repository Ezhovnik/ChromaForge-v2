#ifndef SRC_PLAYER_CONTROL_H_
#define SRC_PLAYER_CONTROL_H_

#include <glm/glm.hpp>

class PhysicsSolver;
class Chunks;
class Player;
class Level;

class PlayerController {
	Level* level;
public:
	glm::vec3 selectedBlockPosition;
	int selectedBlockId = -1;

	PlayerController(Level* level);

	void update_controls(float delta);
	void update_interaction();
};

#endif // SRC_PLAYER_CONTROL_H_
