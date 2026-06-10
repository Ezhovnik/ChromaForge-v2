#include <world/Level.h>

#include <lighting/Lighting.h>
#include <voxels/Chunks.h>
#include <voxels/ChunksStorage.h>
#include <voxels/Chunk.h>
#include <physics/PhysicsSolver.h>
#include <physics/Hitbox.h>
#include <objects/Player.h>
#include <objects/Players.h>
#include <world/World.h>
#include <world/LevelEvents.h>
#include <content/Content.h>
#include <items/Inventories.h>
#include <settings.h>
#include <objects/Entities.h>
#include <window/Camera.h>
#include <data/dv_util.h>

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
    players(std::make_unique<Players>(this)),
    settings(settings)
{
    const auto& worldInfo = world->getInfo();
    auto& cameraIndices = content->getIndices(ResourceType::Camera);
    for (size_t i = 0; i < cameraIndices.size(); ++i) {
        auto camera = std::make_shared<Camera>();
        auto map = cameraIndices.getSavedData(i);
        if (map != nullptr) {
            dv::get_vec(map, "pos", camera->position);
            dv::get_mat(map, "rot", camera->rotation);
            map.at("perspective").get(camera->perspective);
            map.at("flipped").get(camera->flipped);
            map.at("zoom").get(camera->zoom);
            float fov = camera->getFov();
            map.at("fov").get(fov);
            camera->setFov(fov);
        }
        camera->updateVectors();
        cameras.push_back(std::move(camera));
    }

    if (worldInfo.nextEntityId) {
        entities->setNextID(worldInfo.nextEntityId);
    }

    events->listen(LevelEventType::CHUNK_SHOWN, [this](LevelEventType, Chunk* chunk) {
        chunksStorage->incref(chunk);
    });
    events->listen(LevelEventType::CHUNK_HIDDEN, [this](LevelEventType, Chunk* chunk) {
        chunksStorage->decref(chunk);
    });

    // Вычисляем размер матрицы чанков на основе дистанции загрузки и запаса
    uint matrixSize = (settings.chunks.loadDistance.get() + settings.chunks.padding.get()) * 2;
    chunks = std::make_unique<Chunks>(matrixSize, matrixSize, 0, 0, world->wfile.get(), this);
	lighting = std::make_unique<Lighting>(content, chunks.get());

    // Инициализируем менеджер инвентарей и сохраняем инвентарь игрока
    inventories = std::make_unique<Inventories>(*this);
}

Level::~Level() = default;

void Level::loadMatrix(int32_t x, int32_t z, uint32_t radius) {
	chunks->setCenter(x, z);
    uint32_t diameter = std::min(
        radius * 2LL, 
        (settings.chunks.loadDistance.get() + settings.chunks.padding.get()) * 2
    );
	if (chunks->getWidth() != diameter) chunks->resize(diameter, diameter);
}

World* Level::getWorld() {
    return world.get();
}

const World* Level::getWorld() const {
    return world.get();
}

void Level::onSave() {
    auto& cameraIndices = content->getIndices(ResourceType::Camera);
    for (size_t i = 0; i < cameraIndices.size(); ++i) {
        auto& camera = *cameras.at(i);
        auto map = dv::object();
        map["pos"] = dv::to_value(camera.position);
        map["rot"] = dv::to_value(camera.rotation);
        map["perspective"] = camera.perspective;
        map["flipped"] = camera.flipped;
        map["zoom"] = camera.zoom;
        map["fov"] = camera.getFov();
        cameraIndices.saveData(i, std::move(map));
    }
}

std::shared_ptr<Camera> Level::getCamera(const std::string& name) {
    size_t index = content->getIndices(ResourceType::Camera).indexOf(name);
    if (index == ResourceIndices::MISSING) return nullptr;
    return cameras.at(index);
}
