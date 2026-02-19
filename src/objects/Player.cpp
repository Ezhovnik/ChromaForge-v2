#include "Player.h"

#include "../physics/Hitbox.h"
#include "../window/Camera.h"
#include "../world/Level.h"
#include "../physics/PhysicsSolver.h"

namespace PlayerConsts {
    constexpr float CROUCH_SPEED_MUL = 0.35f;
    constexpr float CROUCH_SHIFT_Y = -0.2f;
    constexpr float RUN_SPEED_MUL = 1.5f;
    constexpr float FLIGHT_SPEED_MUL = 5.0f;
    constexpr float JUMP_FORCE = 8.0f;
    constexpr float GROUND_DAMPING = 10.0f;
    constexpr float AIR_DAMPING = 7.0f;
    constexpr float CHEAT_SPEED_MUL = 5.0f;
}

Player::Player(glm::vec3 position, float speed) : speed(speed), choosenBlock(1) {
	camera = new Camera(position, glm::radians(90.0f));
	hitbox = new Hitbox(position, glm::vec3(0.2f, 0.9f, 0.2f));
}

Player::~Player(){
	delete hitbox;
}

void Player::update(Level* level, PlayerInput& input, float delta) {
	bool crouch = input.crouch && hitbox->grounded && !input.sprint;
	float speed = this->speed;

	if (flight) speed *= PlayerConsts::FLIGHT_SPEED_MUL;

	if (input.cheat) speed *= PlayerConsts::CHEAT_SPEED_MUL;

	if (crouch) speed *= PlayerConsts::CROUCH_SPEED_MUL;
	else if (input.sprint) speed *= PlayerConsts::RUN_SPEED_MUL;

	glm::vec3 dir(0,0,0);
	if (input.moveForward){
		dir.x += camera->dir.x;
		dir.z += camera->dir.z;
	}
	if (input.moveBack){
		dir.x -= camera->dir.x;
		dir.z -= camera->dir.z;
	}
	if (input.moveRight){
		dir.x += camera->right.x;
		dir.z += camera->right.z;
	}
	if (input.moveLeft){
		dir.x -= camera->right.x;
		dir.z -= camera->right.z;
	}
	if (length(dir) > 0.0f){
		dir = normalize(dir);
		hitbox->velocity.x += dir.x * speed * delta * 9;
		hitbox->velocity.z += dir.z * speed * delta * 9;
	}

	float vel = std::max(glm::length(hitbox->velocity * 0.25f), 1.0f);
	int substeps = int(delta * vel * 1000);
	substeps = std::min(100, std::max(1, substeps));
	level->physics->step(level->chunks, hitbox, delta, substeps, crouch, flight ? 0.0f : 1.0f, !noclip);
	if (flight && hitbox->grounded) flight = false;

	if (input.jump && hitbox->grounded) hitbox->velocity.y = PlayerConsts::JUMP_FORCE;


	if ((input.flight && !noclip) ||
		(input.noclip && flight == noclip)){
		flight = !flight;
		if (flight){
			hitbox->grounded = false;
		}
	}
	if (input.noclip) {
		noclip = !noclip;
	}

	hitbox->linear_damping = PlayerConsts::GROUND_DAMPING;
	if (flight){
		hitbox->linear_damping = PlayerConsts::AIR_DAMPING;
		hitbox->velocity.y *= 1.0f - delta * 9;
		if (input.jump) hitbox->velocity.y += speed * delta * 9;
		if (input.crouch) hitbox->velocity.y -= speed * delta * 9;
		
	}
	if (!hitbox->grounded) hitbox->linear_damping = PlayerConsts::AIR_DAMPING;

	input.noclip = false;
	input.flight = false;
}

void Player::teleport(glm::vec3 position) {
    hitbox->position = position;
}

float Player::getSpeed() const {
	return speed;
}
