#ifndef OBJECTS_PLAYER_H_
#define OBJECTS_PLAYER_H_

#include <glm/glm.hpp>

#include "../voxels/voxel.h"

class Camera;
class Hitbox;
class Level;

struct PlayerInput {
	bool zoom;
	bool moveForward;
	bool moveBack;
	bool moveRight;
	bool moveLeft;
	bool sprint;
	bool crouch;
	bool cheat;
	bool jump;
	bool noclip;
	bool flight;
    bool attack;
    bool build;
    bool pickBlock;
};

class Player {
private:
	float speed;
public:
	Camera* camera;
	Hitbox* hitbox;

	bool flight = false;
    bool noclip = false;
    bool debug = false;

    int choosenBlock;

	float camX = 0.0f;
	float camY = 0.0f;

    voxel selectedVoxel {0, 0};

	Player(glm::vec3 position, float speed);
	~Player();

    void teleport(glm::vec3 position);

	float getSpeed() const;
	void update(Level* level, PlayerInput& input, float delta);
};

#endif // OBJECTS_PLAYER_H_
