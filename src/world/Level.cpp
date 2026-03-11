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

inline constexpr float GRAVITY = 22.6f;

inline constexpr glm::vec3 SPAWNPOINT = {0, 256, 0}; ///< Точка появления игрока
inline constexpr float DEFAULT_PLAYER_SPEED = 5.0f; ///< Базовая скорость перемещения игрока
inline constexpr int DEFAULT_PLAYER_INVENTORY_SIZE = 40; ///< Размер инвентаря игрока (количество слотов)

Level::Level(World* world, const Content* content, EngineSettings& settings) :
	world(world),
    content(content),
    chunksStorage(new ChunksStorage(this)),
    events(new LevelEvents()),
    settings(settings)
{
    objCounter = 0;
    physics = new PhysicsSolver(glm::vec3(0, -GRAVITY, 0));

    auto inventory = std::make_shared<Inventory>(0, DEFAULT_PLAYER_INVENTORY_SIZE);
    player = spawnObject<Player>(SPAWNPOINT, DEFAULT_PLAYER_SPEED, inventory);

    // Вычисляем размер матрицы чанков на основе дистанции загрузки и запаса
    uint matrixSize = (settings.chunks.loadDistance + settings.chunks.padding) * 2;
    chunks = new Chunks(matrixSize, matrixSize, 0, 0, world->wfile.get(), events, content);

	lighting = new Lighting(content, chunks);

    // Создаем событие скрытия чанка
    events->listen(CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		this->chunksStorage->remove(chunk->chunk_x, chunk->chunk_z);
	});

    // Инициализируем менеджер инвентарей и сохраняем инвентарь игрока
    inventories = std::make_unique<Inventories>(*this);
    inventories->store(player->getInventory());
}

Level::~Level(){
	delete chunks;
	delete physics;
    delete events;
	delete lighting;
    delete chunksStorage;

    for (auto obj : objects) {
        obj.reset();
    }
}

void Level::update() {
	glm::vec3 position = player->hitbox->position;
	chunks->setCenter(position.x, position.z);

    int matrixSize = (settings.chunks.loadDistance + settings.chunks.padding) * 2;
    if (chunks->width != matrixSize) chunks->resize(matrixSize, matrixSize);
}

World* Level::getWorld() {
    return world.get();
}

template<class T, typename... Args>
std::shared_ptr<T> Level::spawnObject(Args&&... args) {
    // Проверяем, что T действительно наследует Object
	static_assert(std::is_base_of<Object, T>::value, "T must be a derived of Object class");

	std::shared_ptr<T> tObj = std::make_shared<T>(args...);

    // Преобразуем к базовому типу Object для хранения в списке objects
	std::shared_ptr<Object> obj = std::dynamic_pointer_cast<Object, T>(tObj);
	objects.push_back(obj);
	obj->objectUID = objCounter;
	obj->spawned();
	objCounter += 1;
	return tObj;
}
