#include "Player.h"
#include "../physics/Hitbox.h"
#include "../window/Camera.h"

Player::Player(glm::vec3 position, float speed, Camera* camera) :
		speed(speed),
		camera(camera),
		choosenBlock(1),
		camX(0.0f), camY(0.0f){
	hitbox = new Hitbox(position, glm::vec3(0.2f, 0.9f, 0.2f));
}

Player::~Player(){
	delete hitbox;
}
