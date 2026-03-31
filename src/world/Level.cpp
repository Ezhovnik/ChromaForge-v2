#include "Level.h"

#include "../lighting/Lighting.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksStorage.h"
#include "../voxels/Chunk.h"
#include "../physics/PhysicsSolver.h"
#include "../physics/Hitbox.h"
#include "../objects/Player.h"
#include "World.h"
#include "LevelEvents.h"
#include "../content/Content.h"
#include "../items/Inventories.h"
#include "../interfaces/Object.h"
#include "../settings.h"

inline constexpr float GRAVITY = -22.6f;

Level::Level(
    std::unique_ptr<World> world,
    const Content* content,
    EngineSettings& settings
) : world(std::move(world)),
    content(content),
    chunksStorage(std::make_unique<ChunksStorage>(this)),
	physics(std::make_unique<PhysicsSolver>(glm::vec3(0, GRAVITY, 0))),
    events(std::make_unique<LevelEvents>()),
    settings(settings)
{
    auto inventory = std::make_shared<Inventory>(
        this->world->getNextInventoryId(), 
        DEFAULT_PLAYER_INVENTORY_SIZE
    );
    auto player = spawnObject<Player>(
        DEFAULT_SPAWNPOINT, 
        DEFAULT_PLAYER_SPEED, 
        inventory
    );

    // Вычисляем размер матрицы чанков на основе дистанции загрузки и запаса
    uint matrixSize = (settings.chunks.loadDistance.get() + settings.chunks.padding.get()) * 2;
    chunks = std::make_unique<Chunks>(matrixSize, matrixSize, 0, 0, this->world->wfile.get(), events.get(), content);
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
