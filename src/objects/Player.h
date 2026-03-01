#ifndef OBJECTS_PLAYER_H_
#define OBJECTS_PLAYER_H_

#include <memory>

#include <glm/glm.hpp>

#include "../voxels/voxel.h"

class Camera;
class Hitbox;
class Level;

struct PlayerInput {
	bool zoom;
	bool cameraMode;
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
	itemid_t chosenItem;
public:
	std::shared_ptr<Camera> camera, spCamera, tpCamera;
    std::shared_ptr<Camera> currentCamera;
	std::unique_ptr<Hitbox> hitbox;

	bool flight = false;
    bool noclip = false;
    bool debug = false;

	glm::vec2 cam = {};

    voxel selectedVoxel {0, 0};

	Player(glm::vec3 position, float speed);
	~Player();

    void teleport(glm::vec3 position);

	float getSpeed() const;

	void setChosenItem(itemid_t id);
	itemid_t getChosenItem() const;

	void update(Level* level, PlayerInput& input, float delta);
};

#endif // OBJECTS_PLAYER_H_
