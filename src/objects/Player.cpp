#include <objects/Player.h>

#include <utility>
#include <algorithm>

#include <physics/Hitbox.h>
#include <window/Camera.h>
#include <world/Level.h>
#include <physics/PhysicsSolver.h>
#include <items/Inventory.h>
#include <math/rand.h>
#include <voxels/Chunks.h>
#include <voxels/voxel.h>
#include <content/ContentReport.h>
#include <objects/Entities.h>
#include <core_content_defs.h>
#include <objects/rigging.h>
#include <data/dv_util.h>

namespace PlayerConsts {
    constexpr float CROUCH_SPEED_MUL = 0.35f; ///< Множитель скорости при приседании
    constexpr float RUN_SPEED_MUL = 1.5f; ///< Множитель скорости при беге
    constexpr float FLIGHT_SPEED_MUL = 5.0f; ///< Множитель скорости в режиме полёта
    constexpr float JUMP_FORCE = 8.0f; ///< Сила прыжка
    constexpr float GROUND_DAMPING = 10.0f; ///< Затухание скорости на земле
    constexpr float AIR_DAMPING = 7.0f; ///< Затухание скорости в воздухе
    constexpr float CHEAT_SPEED_MUL = 5.0f; ///< Множитель скорости в режиме читов
}

Player::Player(
	Level* level,
	glm::vec3 position,
	float speed,
	std::shared_ptr<Inventory> inventory,
	entityid_t eid
) : level(level),
	speed(speed),
	chosenSlot(0),
	position(position),
	camera(level->getCamera(CHROMAFORGE_CONTENT_NAMESPACE + ":first-person")),
    spCamera(level->getCamera(CHROMAFORGE_CONTENT_NAMESPACE + ":third-person-front")),
    tpCamera(level->getCamera(CHROMAFORGE_CONTENT_NAMESPACE + ":third-person-back")),
	currentCamera(camera),
	inventory(std::move(inventory)),
	eid(eid)
{
	camera->setFov(glm::radians(90.0f));
    spCamera->setFov(glm::radians(90.0f));
    tpCamera->setFov(glm::radians(90.0f));
}

Player::~Player() {
}

void Player::updateEntity() {
    if (eid == 0) {
        auto& def = level->content->entities.require(CHROMAFORGE_CONTENT_NAMESPACE + ":player");
        eid = level->entities->spawn(def, getPosition());
	} else if (auto entity = level->entities->get(eid)) {
        position = entity->getTransform().pos;
    } else {
        // TODO: Check if chunk loaded
    }
}

Hitbox* Player::getHitbox() {
    if (auto entity = level->entities->get(eid)) {
        return &entity->getRigidbody().hitbox;
    }
    return nullptr;
}

void Player::updateInput(PlayerInput& input, float delta) {
    auto hitbox = getHitbox();
    if (hitbox == nullptr) return;

	bool crouch = input.crouch && hitbox->grounded && !input.sprint;
	float speed = this->speed;

	// Применяем модификаторы скорости
	if (flight) {
		speed *= PlayerConsts::FLIGHT_SPEED_MUL;
	}
	if (input.cheat) {
		speed *= PlayerConsts::CHEAT_SPEED_MUL;
	}

	hitbox->crouching = crouch;
	if (crouch) {
		speed *= PlayerConsts::CROUCH_SPEED_MUL;
	} else if (input.sprint) {
		speed *= PlayerConsts::RUN_SPEED_MUL;
	}

	// Вычисляем направление движения на основе ввода и ориентации камеры
	glm::vec3 dir(0, 0, 0);
	if (input.moveForward){
		dir += camera->dir;
	}
	if (input.moveBack){
		dir -= camera->dir;
	}
	if (input.moveRight){
		dir += camera->right;
	}
	if (input.moveLeft){
		dir -= camera->right;
	}
	// Если есть движение, нормализуем и придаём импульс
	if (length(dir) > 0.0f) {
		dir = normalize(dir);
		hitbox->velocity += dir * speed * delta * 9.0f;
	}

	hitbox->linearDamping = PlayerConsts::GROUND_DAMPING;
    hitbox->verticalDamping = flight;
	hitbox->gravityScale = flight ? 0.0f : 1.0f;
    if (flight) {
        hitbox->linearDamping = PlayerConsts::AIR_DAMPING;
        if (input.jump) {
            hitbox->velocity.y += speed * delta * 9;
        }
        if (input.crouch) {
            hitbox->velocity.y -= speed * delta * 9;
        }
    }
    if (!hitbox->grounded) {
        hitbox->linearDamping = PlayerConsts::AIR_DAMPING;
	}
	if (input.jump && hitbox->grounded) {
		hitbox->velocity.y = PlayerConsts::JUMP_FORCE;
	}
	if ((input.flight && !noclip) || (input.noclip && flight == noclip)){
		flight = !flight;
		if (flight) hitbox->velocity.y += 1.0f;
	}

	hitbox->type = noclip ? BodyType::Kinematic : BodyType::Dynamic;
	if (input.noclip) noclip = !noclip;

	input.noclip = false;
	input.flight = false;
}

void Player::postUpdate() {
    auto entity = level->entities->get(eid);
    if (!entity.has_value()) return;

    auto& hitbox = entity->getRigidbody().hitbox;
    position = hitbox.position;

    if (flight && hitbox.grounded) {
        flight = false;
    }

	// Если точка возрождения не задана, пытаемся найти её
	if (spawnpoint.y <= 0.1) attemptToFindSpawnpoint();

	auto& skeleton = entity->getSkeleton();

    skeleton.visible = currentCamera != camera;

    auto body = skeleton.config->find("body");
    auto head = skeleton.config->find("head");

	if (body) {
		skeleton.pose.matrices[body->getIndex()] = glm::rotate(
			glm::mat4(1.0f), glm::radians(cam.x), glm::vec3(0, 1, 0)
		);
	}
	if (head) {
		skeleton.pose.matrices[head->getIndex()] = glm::rotate(
			glm::mat4(1.0f), glm::radians(cam.y), glm::vec3(1, 0, 0)
		);
	}
}

void Player::attemptToFindSpawnpoint() {
	// Генерируем случайную позицию в окрестности текущей
	glm::vec3 newpos {
		position.x + (RandomGenerator::get<int>(0, RAND_MAX) % 200 - 100), 
		RandomGenerator::get<int>(0, RAND_MAX) % 80 + 100, 
		position.z + (RandomGenerator::get<int>(0, RAND_MAX) % 200 - 100)
	};

	// Опускаемся вниз, пока не найдём твёрдый блок под ногами
	while (newpos.y > 0 && !level->chunks->isObstacleBlock(newpos.x, newpos.y - 2, newpos.z)) {
		newpos.y--;
	}

	// Проверяем, что в позиции игрока нет препятствий и над головой воздух
	voxel* headvox = level->chunks->getVoxel(newpos.x, newpos.y + 1, newpos.z);
	if (level->chunks->isObstacleBlock(newpos.x, newpos.y, newpos.z) || headvox == nullptr || headvox->id != 0) return; // не удалось найти безопасное место
	spawnpoint = newpos + glm::vec3(0.5f, 0.0f, 0.5f);
	teleport(spawnpoint);
}

dv::value Player::serialize() const {
    auto root = dv::object();

    root["position"] = dv::to_value(position);
    root["rotation"] = dv::to_value(cam);
    root["spawnpoint"] = dv::to_value(spawnpoint);

    root["flight"] = flight;
    root["noclip"] = noclip;
    root["chosen-slot"] = chosenSlot;
    root["entity"] = eid;
    root["inventory"] = inventory->serialize();
	auto found = std::find(level->cameras.begin(), level->cameras.end(), currentCamera);
    if (found != level->cameras.end()) {
        root["camera"] = level->content->getIndices(ResourceType::Camera).getName(found - level->cameras.begin());
    }
    return root;
}

void Player::deserialize(const dv::value& src) {
    const auto& posarr = src["position"];
    dv::get_vec(posarr, position);
	camera->position = position;

	const auto& rotarr = src["rotation"];
    dv::get_vec(rotarr, cam);

	const auto& sparr = src["spawnpoint"];
    setSpawnPoint(glm::vec3(sparr[0].asNumber(), sparr[1].asNumber(), sparr[2].asNumber()));

	flight = src["flight"].asBoolean();
    noclip = src["noclip"].asBoolean();
    setChosenSlot(src["chosen-slot"].asInteger());
    eid = src["entity"].asNumber();

    if (src.has("inventory")) {
        getInventory()->deserialize(src["inventory"]);
    }

	if (src.has("camera")) {
        std::string name = src["camera"].asString();
        if (auto camera = level->getCamera(name)) {
            currentCamera = camera;
        }
    }
}

void Player::convert(dv::value& data, const ContentReport* report) {
    if (data.has("players")) {
        auto& players = data["players"];
        for (uint i = 0; i < players.size(); i++) {
            auto& playerData = players[i];
            if (playerData.has("inventory")) {
                Inventory::convert(playerData["inventory"], report);
			}
        }
    } else if (data.has("inventory")) {
        Inventory::convert(data["inventory"], report);
    }
}

void Player::teleport(glm::vec3 position) {
    this->position = position;
    if (auto hitbox = getHitbox()) {
        hitbox->position = position;
    }
}

float Player::getSpeed() const {
	return speed;
}

void Player::setChosenSlot(int index) {
    chosenSlot = index;
}

int Player::getChosenSlot() const {
    return chosenSlot;
}

entityid_t Player::getEntity() const {
    return eid;
}

void Player::setEntity(entityid_t eid) {
    this->eid = eid;
}

std::shared_ptr<Inventory> Player::getInventory() const {
    return inventory;
}

void Player::setSpawnPoint(glm::vec3 spawnpoint) {
	this->spawnpoint = spawnpoint;
}

glm::vec3 Player::getSpawnPoint() const {
	return spawnpoint;
}

bool Player::isFlight() const {
    return flight;
}

void Player::setFlight(bool flag) {
    this->flight = flag;
}

bool Player::isNoclip() const {
    return noclip;
}

void Player::setNoclip(bool flag) {
    this->noclip = flag;
}

void Player::updateSelectedEntity() {
    selectedEid = selection.entity;
}

entityid_t Player::getSelectedEntity() const {
    return selectedEid;
}
