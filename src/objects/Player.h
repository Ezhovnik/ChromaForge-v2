#ifndef OBJECTS_PLAYER_H_
#define OBJECTS_PLAYER_H_

#include <memory>

#include <glm/glm.hpp>

#include "../voxels/voxel.h"

class Camera;
class Hitbox;
class Level;
class Inventory;

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
	itemid_t chosenSlot;

	glm::vec3 spawnpoint {};
public:
	std::shared_ptr<Camera> camera, spCamera, tpCamera;
    std::shared_ptr<Camera> currentCamera;
	std::unique_ptr<Hitbox> hitbox;
	std::shared_ptr<Inventory> inventory;

	bool flight = false;
    bool noclip = false;
    bool debug = false;

	glm::vec2 cam = {};

    voxel selectedVoxel {0, 0};

	Player(glm::vec3 position, float speed);
	~Player() = default;

    void teleport(glm::vec3 position);

	float getSpeed() const;

	void attemptToFindSpawnpoint(Level* level);

	void setChosenSlot(int index);
	itemid_t getChosenSlot() const;
	std::shared_ptr<Inventory> getInventory() const;

	void setSpawnPoint(glm::vec3 point);
    glm::vec3 getSpawnPoint() const;

	void update(Level* level, PlayerInput& input, float delta);
};

#endif // OBJECTS_PLAYER_H_
