#include <world/World.h>

#include <memory>
#include <utility>

#include <world/Level.h>
#include <world/LevelEvents.h>
#include <files/WorldFiles.h>
#include <voxels/Chunk.h>
#include <voxels/Chunks.h>
#include <voxels/ChunksStorage.h>
#include <objects/Player.h>
#include <physics/PhysicsSolver.h>
#include <debug/Logger.h>
#include <content/Content.h>
#include <content/ContentReport.h>
#include <items/Inventories.h>
#include <world/generator/WorldGenerator.h>
#include <world/generator/Generator.h>
#include <settings.h>
#include <objects/Entities.h>

static constexpr float DAYTIME_SPECIFIC_SPEED = 1.0f / (24.0f * 60.0f);

world_load_error::world_load_error(const std::string& message) : std::runtime_error(message) {
}

World::World(
	WorldInfo info,
    const std::shared_ptr<WorldFiles>& worldFiles, 
	const Content* content, 
	const std::vector<ContentPack>& packs
) : info(std::move(info)),
    content(content),
    packs(packs),
    wfile(std::move(worldFiles)) {}

World::~World() {
}

void World::updateTimers(float delta) {
	info.daytime += delta * info.daytimeSpeed * DAYTIME_SPECIFIC_SPEED;
	info.daytime = std::fmod(info.daytime, 1.0f); // зацикливаем в [0,1)
	info.totalTime += delta;
}

void World::writeResources(const Content* content) {
    auto root = dynamic::Map();
    for (size_t typeIndex = 0; typeIndex < RESOURCE_TYPES_COUNT; ++typeIndex) {
        auto typeName = to_string(static_cast<ResourceType>(typeIndex));
        auto& list = root.putList(typeName);
        auto& indices = content->resourceIndices[typeIndex];
        for (size_t i = 0; i < indices.size(); ++i) {
            auto& map = list.putMap();
            map.put("name", indices.getName(i));
            if (auto data = indices.getSavedData(i)) {
                map.put("saved", data);
            }
        }
    }
    files::write_json(wfile->getResourcesFile(), &root);
}

void World::write(Level* level) {
	const Content* content = level->content;

	level->chunks->saveAll();

	info.nextEntityId = level->entities->peekNextID();

	// Запись метаданных мира и игрока
	wfile->write(this, content);
	auto playerFile = dynamic::Map();

    auto& players = playerFile.putList("players");
    for (const auto& object : level->objects) {
        if (auto player = std::dynamic_pointer_cast<Player>(object)) {
            players.put(player->serialize());
        }
    }
    files::write_json(wfile->getPlayerFile(), &playerFile);

	writeResources(content);
}

std::unique_ptr<Level> World::create(
	const std::string& name,
	const std::string& generator,
	const std::filesystem::path& directory, 
	uint64_t seed, 
	EngineSettings& settings, 
	const Content* content, 
	const std::vector<ContentPack>& packs
) {
	WorldInfo info {};
    info.name = name;
    info.generator = generator;
    info.seed = seed;
	auto world = std::make_unique<World>(
        info,
        std::make_unique<WorldFiles>(directory, settings.debug),
        content,
        packs
    );
	return std::make_unique<Level>(std::move(world), content, settings);
}

std::shared_ptr<ContentReport> World::checkIndices(const std::shared_ptr<WorldFiles>& worldFiles, const Content* content) {
	std::filesystem::path indicesFile = worldFiles->getIndicesFile();
	if (std::filesystem::is_regular_file(indicesFile)) return ContentReport::create(worldFiles, indicesFile, content);

	return nullptr;
}

std::unique_ptr<Level> World::load(
	const std::shared_ptr<WorldFiles>& worldFilesPtr,
	EngineSettings& settings,
	const Content* content,
	const std::vector<ContentPack>& packs
) {
	LOG_INFO("Loading world");
	// Временно создаём мир с заглушкой имени и сидом 0 — они будут перезаписаны при десериализации

    auto worldFiles = worldFilesPtr.get();
    auto info = worldFiles->readWorldInfo();
    if (!info.has_value()) {
		LOG_ERROR("Could not to find world.json");
        throw world_load_error("could not to find world.json");
	}
	LOG_INFO("World version: {}.{}.{}", info->major, info->minor, info->maintenance);

	auto world = std::make_unique<World>(
		info.value(),
        std::move(worldFilesPtr),
		content, 
		packs
	);
	auto& wfile = world->wfile;

	wfile->readResourcesData(content);

	LOG_INFO("Creating a level");
	auto level = std::make_unique<Level>(std::move(world), content, settings);
	{
        std::filesystem::path file = wfile->getPlayerFile();
        if (!std::filesystem::is_regular_file(file)) {
			LOG_WARN("'player.json' does not exists");
        } else {
            auto playerFile = files::read_json(file);
            if (playerFile->has("players")) {
                level->objects.clear();
                auto players = playerFile->list("players");
                for (size_t i = 0; i < players->size(); ++i) {
                    auto player = level->spawnObject<Player>(
						level.get(),
						DEFAULT_SPAWNPOINT,
						DEFAULT_PLAYER_SPEED,
						level->inventories->create(DEFAULT_PLAYER_INVENTORY_SIZE),
						0
					);
                    player->deserialize(players->map(i).get());
                    level->inventories->store(player->getInventory());
                }
            } else {
				auto player = level->getObject<Player>(0);
                player->deserialize(playerFile.get());
                level->inventories->store(player->getInventory());
            }
        }
    }
	LOG_INFO("Level successfully created");
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
    this->info.seed = seed;
}

uint64_t World::getSeed() const {
	return info.seed;
}

void World::setName(const std::string& name) {
	this->info.name = name;
}

std::string World::getName() const {
    return info.name;
}

void World::setGenerator(const std::string& generator) {
    this->info.generator = generator;
}

std::string World::getGenerator() const {
    return info.generator;
}

void WorldInfo::deserialize(dynamic::Map* root) {
    name = root->get("name", name);
	generator = root->get("generator", generator);
    seed = root->get("seed", seed);

	if (generator.empty()) generator = WorldGenerator::DEFAULT;

	// Информация о версии движка
	if (auto verobj = root->map("version")) {
		verobj->num("major", major);
		verobj->num("minor", minor);
		verobj->num("maintenance", maintenance);
	}

	// Таймеры
	if (auto timeobj = root->map("time")) {
		timeobj->num("day-time", daytime);
		timeobj->num("day-time-speed", daytimeSpeed);
        timeobj->num("total-time", totalTime);
	}

	if (auto weatherobj = root->map("weather")) {
        weatherobj->num("skyClearness", skyClearness);
    }

	// Счётчик инвентарей (по умолчанию 2, т.к. 1 обычно зарезервирован)
    nextInventoryId = root->get("next-inventory-id", 2);

	nextEntityId = root->get("next-entity-id", 1);
}

std::unique_ptr<dynamic::Map> WorldInfo::serialize() const {
	auto root = std::make_unique<dynamic::Map>();

	// Информация о версии движка
	auto& versionobj = root->putMap("version");
	versionobj.put("major", ENGINE_VERSION_MAJOR);
	versionobj.put("minor", ENGINE_VERSION_MINOR);
	versionobj.put("maintenance", ENGINE_VERSION_MAINTENANCE);

	root->put("name", name);
	root->put("generator", generator);
	root->put("seed", static_cast<integer_t>(seed));

	// Время
	auto& timeobj = root->putMap("time");
	timeobj.put("day-time", daytime);
	timeobj.put("day-time-speed", daytimeSpeed);
	timeobj.put("total-time", totalTime);

	auto& weatherobj = root->putMap("weather");
    weatherobj.put("skyClearness", skyClearness);

    root->put("next-inventory-id", nextInventoryId);
	root->put("next-entity-id", nextEntityId);
    return root;
}
