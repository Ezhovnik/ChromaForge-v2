#include "World.h"

#include <memory>

#include "Level.h"
#include "LevelEvents.h"
#include "../files/WorldFiles.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksStorage.h"
#include "../objects/Player.h"
#include "../physics/PhysicsSolver.h"
#include "../window/Camera.h"
#include "../logger/Logger.h"
#include "../content/Content.h"
#include "../content/ContentLUT.h"
#include "../items/Inventories.h"

inline constexpr glm::vec3 SPAWNPOINT = {0, 256, 0}; // Точка, где игрок появляется в мире
inline constexpr float DEFAULT_PLAYER_SPEED = 5.0f; // Начальная скорость перемещения игрока
inline constexpr int DEFAULT_PLAYER_INVENTORY_SIZE = 40;

world_load_error::world_load_error(std::string message) : std::runtime_error(message) {
}

World::World(std::string name, std::filesystem::path directory, uint64_t seed, EngineSettings& settings, const Content* content, const std::vector<ContentPack> packs) : name(name), seed(seed), settings(settings), content(content), packs(packs) {
	wfile = new WorldFiles(directory, settings.debug);
}

World::~World(){
	delete wfile;
}

void World::updateTimers(float delta) {
	daytime += delta * daytimeSpeed;
	daytime = fmod(daytime, 1.0f);
	totalTime += delta;
}

void World::write(Level* level) {
	const Content* content = level->content;
	Chunks* chunks = level->chunks;

	for (size_t i = 0; i < chunks->volume; ++i) {
		std::shared_ptr<Chunk> chunk = chunks->chunks[i];
		if (chunk == nullptr || !chunk->isLighted()) continue;
		bool lightsUnsaved = !chunk->isLoadedLights() && settings.debug.doWriteLights;
		if (!chunk->isUnsaved() && !lightsUnsaved) continue;
		wfile->put(chunk.get());
	}

	wfile->write(this, content);
	wfile->writePlayer(level->player);
}

Level* World::create(std::string name, std::filesystem::path directory, uint64_t seed, EngineSettings& settings, const Content* content, const std::vector<ContentPack>& packs) {
	LOG_INFO("Creating world");
	World* world = new World(name, directory, seed, settings, content, packs);
	LOG_INFO("World successfully created");

	LOG_INFO("Creating a level");
	auto inventory = std::make_shared<Inventory>(world->getNextInventoryId(), DEFAULT_PLAYER_INVENTORY_SIZE);
    Player* player = new Player(SPAWNPOINT, DEFAULT_PLAYER_SPEED, inventory);
    Level* level = new Level(world, content, player, settings);
    level->inventories->store(player->getInventory());
	LOG_INFO("Level successfully created");

	Logger::getInstance().flush();

	return level;
}

ContentLUT* World::checkIndices(const std::filesystem::path& directory, const Content* content) {
	std::filesystem::path indicesFile = directory/std::filesystem::path("indices.json");
	if (std::filesystem::is_regular_file(indicesFile)) return ContentLUT::create(indicesFile, content);

	return nullptr;
}

Level* World::load(std::filesystem::path directory, EngineSettings& settings, const Content* content, const std::vector<ContentPack>& packs) {
	LOG_INFO("Loading world");
	auto world = std::make_unique<World>(".", directory, 0, settings, content, packs);
	auto& wfile = world->wfile;

	if (!wfile->readWorldInfo(world.get())) {
		LOG_ERROR("Could not to find world.json");
		Logger::getInstance().flush();
		throw world_load_error("Could not to find world.json");
	}

	LOG_INFO("Creating a level");
	auto inventory = std::make_shared<Inventory>(world->getNextInventoryId(), DEFAULT_PLAYER_INVENTORY_SIZE);
    Player* player = new Player(SPAWNPOINT, DEFAULT_PLAYER_SPEED, inventory);
	wfile->readPlayer(player);
	Level* level = new Level(world.get(), content, player, settings);
	level->inventories->store(player->getInventory());
	LOG_INFO("Level successfully created");

	world.release();
	LOG_INFO("World successfully created");
	return level;
}

bool World::hasPack(const std::string& id) const {
    for (auto& pack : packs) {
        if (pack.id == id) return true;
    }
    return false;
}

const std::vector<ContentPack>& World::getPacks() const {
	return packs;
}

void World::setSeed(uint64_t seed) {
    this->seed = seed;
}

uint64_t World::getSeed() const {
	return seed;
}

void World::setName(const std::string& name) {
	this->name = name;
}

std::string World::getName() const {
    return name;
}

void World::deserialize(dynamic::Map* root) {
    name = root->getStr("name", name);
    seed = root->getInt("seed", seed);

	auto verobj = root->map("version");
	if (verobj) {
		int major = 0, minor = -1, maintenance = -1;
		verobj->num("major", major);
		verobj->num("minor", minor);
		verobj->num("maintenance", maintenance);
		LOG_DEBUG("World version: {}.{}.{}", major, minor, maintenance);
	}

	auto timeobj = root->map("time");
	if (timeobj) {
		timeobj->num("day-time", daytime);
		timeobj->num("day-time-speed", daytimeSpeed);
        timeobj->num("total-time", totalTime);
	}

    nextInventoryId = root->getNum("next-inventory-id", 2);
}

std::unique_ptr<dynamic::Map> World::serialize() const {
	auto root = std::make_unique<dynamic::Map>();

	auto& versionobj = root->putMap("version");
	versionobj.put("major", ENGINE_VERSION_MAJOR);
	versionobj.put("minor", ENGINE_VERSION_MINOR);
	versionobj.put("maintenance", ENGINE_VERSION_MAINTENANCE);

	root->put("name", getName());
	root->put("seed", getSeed());

	auto& timeobj = root->putMap("time");
	timeobj.put("day-time", daytime);
	timeobj.put("day-time-speed", daytimeSpeed);
	timeobj.put("total-time", totalTime);

    root->put("next-inventory-id", nextInventoryId);
    return root;
}
