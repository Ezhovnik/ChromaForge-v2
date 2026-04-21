#include <world/Level.h>

#include <lighting/Lighting.h>
#include <voxels/Chunks.h>
#include <voxels/ChunksStorage.h>
#include <voxels/Chunk.h>
#include <physics/PhysicsSolver.h>
#include <physics/Hitbox.h>
#include <objects/Player.h>
#include <world/World.h>
#include <world/LevelEvents.h>
#include <content/Content.h>
#include <items/Inventories.h>
#include <interfaces/Object.h>
#include <settings.h>
#include <objects/Entities.h>
#include <window/Camera.h>
#include <data/dynamic_util.h>

inline constexpr float GRAVITY = -22.6f;

Level::Level(
    std::unique_ptr<World> worldPtr,
    const Content* content,
    EngineSettings& settings
) : world(std::move(worldPtr)),
    content(content),
    chunksStorage(std::make_unique<ChunksStorage>(this)),
	physics(std::make_unique<PhysicsSolver>(glm::vec3(0, GRAVITY, 0))),
    events(std::make_unique<LevelEvents>()),
    entities(std::make_unique<Entities>(this)),
    settings(settings)
{
    auto& cameraIndices = content->getIndices(ResourceType::Camera);
    for (size_t i = 0; i < cameraIndices.size(); ++i) {
        auto camera = std::make_shared<Camera>();
        if (auto map = cameraIndices.getSavedData(i)) {
            dynamic::get_vec(map, "pos", camera->position);
            dynamic::get_mat(map, "rot", camera->rotation);
            map->flag("perspective", camera->perspective);
            map->flag("flipped", camera->flipped);
            map->num("zoom", camera->zoom);
            float fov = camera->getFov();
            map->num("fov", fov);
            camera->setFov(fov);
        }
        camera->updateVectors();
        cameras.push_back(std::move(camera));
    }

    if (world->nextEntityId) {
        entities->setNextID(world->nextEntityId);
    }

    auto inventory = std::make_shared<Inventory>(
        world->getNextInventoryId(), 
        DEFAULT_PLAYER_INVENTORY_SIZE
    );
    auto player = spawnObject<Player>(
        this,
        DEFAULT_SPAWNPOINT, 
        DEFAULT_PLAYER_SPEED, 
        inventory,
        0
    );

    // Вычисляем размер матрицы чанков на основе дистанции загрузки и запаса
    uint matrixSize = (settings.chunks.loadDistance.get() + settings.chunks.padding.get()) * 2;
    chunks = std::make_unique<Chunks>(matrixSize, matrixSize, 0, 0, world->wfile.get(), this);
	lighting = std::make_unique<Lighting>(content, chunks.get());

    // Создаем событие скрытия чанка
    events->listen(CHUNK_HIDDEN, [this](lvl_event_type, Chunk* chunk) {
		this->chunksStorage->remove(chunk->chunk_x, chunk->chunk_z);
	});

    // Инициализируем менеджер инвентарей и сохраняем инвентарь игрока
    inventories = std::make_unique<Inventories>(*this);
    inventories->store(player->getInventory());
}

Level::~Level(){
    for (auto obj : objects) {
        obj.reset();
    }
}

void Level::loadMatrix(int32_t x, int32_t z, uint32_t radius) {
	chunks->setCenter(x, z);
    uint32_t diameter = std::min(
        radius * 2LL, 
        (settings.chunks.loadDistance.get() + settings.chunks.padding.get()) * 2
    );
	if (chunks->width != diameter) chunks->resize(diameter, diameter);
}

World* Level::getWorld() {
    return world.get();
}

void Level::onSave() {
    auto& cameraIndices = content->getIndices(ResourceType::Camera);
    for (size_t i = 0; i < cameraIndices.size(); ++i) {
        auto& camera = *cameras.at(i);
        auto map = dynamic::create_map();
        map->put("pos", dynamic::to_value(camera.position));
        map->put("rot", dynamic::to_value(camera.rotation));
        map->put("perspective", camera.perspective);
        map->put("flipped", camera.flipped);
        map->put("zoom", camera.zoom);
        map->put("fov", camera.getFov());
        cameraIndices.saveData(i, std::move(map));
    }
}

std::shared_ptr<Camera> Level::getCamera(const std::string& name) {
    size_t index = content->getIndices(ResourceType::Camera).indexOf(name);
    if (index == ResourceIndices::MISSING) return nullptr;
    return cameras.at(index);
}
