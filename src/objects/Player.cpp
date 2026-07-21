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
#include <debug/Logger.h>
#include <objects/Entt_Entity.h>

namespace PlayerConsts {
	constexpr int SPAWN_ATTEMPTS_PER_UPDATE = 64;
}

Player::Player(
	Level& level,
	int64_t id,
	const std::string& name,
	glm::vec3 position,
	float speed,
	std::shared_ptr<Inventory> inventory,
	entityid_t eid
) : level(level),
	id(id),
	name(name),
	speed(speed),
	chosenSlot(0),
	position(position),
	chunks(std::make_unique<Chunks>(
        3, 3, 0, 0, level.events.get(), *level.content.getIndices()
    )),
	fpCamera(level.getCamera(BUILTIN_CONTENT_NAMESPACE + ":first-person")),
    spCamera(level.getCamera(BUILTIN_CONTENT_NAMESPACE + ":third-person-front")),
    tpCamera(level.getCamera(BUILTIN_CONTENT_NAMESPACE + ":third-person-back")),
	currentCamera(fpCamera),
	inventory(std::move(inventory)),
	eid(eid)
{
	fpCamera->setFov(glm::radians(90.0f));
    spCamera->setFov(glm::radians(90.0f));
    tpCamera->setFov(glm::radians(90.0f));
}

Player::~Player() = default;

void Player::updateEntity() {
    if (eid == ENTITY_AUTO) {
        const auto& defaults = level.content.getDefaults();
        const auto& defName = defaults["player-entity"].asString();
        if (!defName.empty()) {
            auto& def = level.content.entities.require(defName);
            eid = level.entities->spawn(def, getPosition());
            if (auto entity = level.entities->get(eid)) {
                entity->setPlayer(id);
            }
        }
	} else if (auto entity = level.entities->get(eid)) {
        position = entity->getTransform().pos;
        if (auto entity = level.entities->get(eid)) {
            entity->setPlayer(id);
        }
    } else if (chunks->getChunkByVoxel(position) && eid != ENTITY_NONE) {
        LOG_WARN("Player entity despawned or deleted; will be respawned");
        eid = ENTITY_AUTO;
    }
}

Hitbox* Player::getHitbox() {
    if (auto entity = level.entities->get(eid)) {
        return &entity->getRigidbody().hitbox;
    }
    return nullptr;
}

void Player::postUpdate() {
    auto entity = level.entities->get(eid);
    if (!entity.has_value()) return;

    auto& hitbox = entity->getRigidbody().hitbox;
    position = hitbox.position;

    if (flight && hitbox.grounded && !noclip) {
        flight = false;
    }

	// Если точка возрождения не задана, пытаемся найти её
	if (spawnpoint.y <= 0.1) {
		for (int i = 0; i < PlayerConsts::SPAWN_ATTEMPTS_PER_UPDATE; ++i) {
            attemptToFindSpawnpoint();
        }
	}
}

void Player::attemptToFindSpawnpoint() {
	// Генерируем случайную позицию в окрестности текущей
	glm::vec3 newpos {
		position.x + (util::RandomGenerator::get<int>(0, RAND_MAX) % 200 - 100), // TODO: Replace util::RandomGenerator to other
		util::RandomGenerator::get<int>(0, RAND_MAX) % 80 + 100, 
		position.z + (util::RandomGenerator::get<int>(0, RAND_MAX) % 200 - 100)
	};

	// Опускаемся вниз, пока не найдём твёрдый блок под ногами
	while (newpos.y > 0 && !chunks->isObstacleBlock(newpos.x, newpos.y - 2, newpos.z)) {
		newpos.y--;
	}

	// Проверяем, что в позиции игрока нет препятствий и над головой воздух
	voxel* headvox = chunks->getVoxel(newpos.x, newpos.y + 1, newpos.z);
	if (chunks->isObstacleBlock(newpos.x, newpos.y, newpos.z) || headvox == nullptr || headvox->id != 0) return; // не удалось найти безопасное место
	spawnpoint = newpos + glm::vec3(0.5f, 0.0f, 0.5f);
	teleport(spawnpoint);
}

dv::value Player::serialize() const {
    auto root = dv::object();

	root["id"] = id;
	root["name"] = name;

    root["position"] = dv::to_value(position);
    root["rotation"] = dv::to_value(rotation);
    root["spawnpoint"] = dv::to_value(spawnpoint);

    root["interaction-distance"] = interactionDistance;
    root["flight"] = flight;
    root["noclip"] = noclip;
    root["suspended"] = suspended;
	root["infinite-items"] = infiniteItems;
	root["instant-destruction"] = instantDestruction;
    root["loading-chunks"] = loadingChunks;
    root["chosen-slot"] = chosenSlot;
    root["entity"] = eid;
    root["inventory"] = inventory->serialize();
	auto found = std::find(level.cameras.begin(), level.cameras.end(), currentCamera);
    if (found != level.cameras.end()) {
        root["camera"] = level.content.getIndices(ResourceType::Camera).getName(found - level.cameras.begin());
    }
    return root;
}

void Player::deserialize(const dv::value& src) {
	src.at("id").get(id);
	src.at("name").get(name);

    const auto& posarr = src["position"];
    dv::get_vec(posarr, position);
	fpCamera->position = position;

	const auto& rotarr = src["rotation"];
    dv::get_vec(rotarr, rotation);

	const auto& sparr = src["spawnpoint"];
    setSpawnPoint(glm::vec3(sparr[0].asNumber(), sparr[1].asNumber(), sparr[2].asNumber()));

	flight = src["flight"].asBoolean();
    noclip = src["noclip"].asBoolean();
    src.at("suspended").get(suspended);
	src.at("infinite-items").get(infiniteItems);
	src.at("instant-destruction").get(instantDestruction);
    src.at("loading-chunks").get(loadingChunks);
    src.at("interaction-distance").get(interactionDistance);
    setChosenSlot(src["chosen-slot"].asInteger());
    eid = src["entity"].asNumber();

    if (src.has("inventory")) {
        getInventory()->deserialize(src["inventory"]);
    }

	if (src.has("camera")) {
        std::string name = src["camera"].asString();
        if (auto camera = level.getCamera(name)) {
            currentCamera = camera;
        }
    }
}

void Player::convert(dv::value& data, const ContentReport* report) {
    if (data.has("players")) {
        auto& players = data["players"];
        for (uint i = 0; i < players.size(); ++i) {
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
    if (auto entity = level.entities->get(eid)) {
        entity->getRigidbody().hitbox.position = position;
        entity->getTransform().setPos(position);
        entity->setInterpolatedPosition(position);
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

const std::shared_ptr<Inventory>& Player::getInventory() const {
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

bool Player::isInfiniteItems() const {
    return infiniteItems;
}

void Player::setInfiniteItems(bool flag) {
    infiniteItems = flag;
}

void Player::updateSelectedEntity() {
    selectedEid = selection.entity;
}

entityid_t Player::getSelectedEntity() const {
    return selectedEid;
}

bool Player::isInstantDestruction() const {
    return instantDestruction;
}

void Player::setInstantDestruction(bool flag) {
    instantDestruction = flag;
}

void Player::setName(const std::string& name) {
    this->name = name;
}

const std::string& Player::getName() const {
    return name;
}

bool Player::isLoadingChunks() const {
    return loadingChunks;
}

void Player::setLoadingChunks(bool flag) {
    loadingChunks = flag;
}

bool Player::isSuspended() const {
    return suspended;
}

void Player::setSuspended(bool flag) {
    suspended = flag;
}

float Player::getInteractionDistance() const {
    return interactionDistance;
}

void Player::setInteractionDistance(float distance) {
    interactionDistance = std::max(1.0f, std::min(200.0f, distance));
}

glm::vec3 Player::getRotation(bool interpolated) const {
    if (interpolated) {
        return rotationInterpolation.getCurrent();
    }
    return rotation;
}

void Player::setRotation(const glm::vec3& rotation) {
    this->rotation = rotation;
    rotationInterpolation.refresh(rotation);
}
