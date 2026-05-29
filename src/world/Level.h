#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include <typedefs.h>

class World;
class Chunks;
class Lighting;
class PhysicsSolver;
class ChunksStorage;
class LevelEvents;
class Content;
class Inventories;
struct EngineSettings;
class Entities;
class Players;
class Camera;

/**
 * @brief Основной класс уровня, объединяющий мир, игрока, чанки, физику и освещение.
 *
 * Level владеет объектами World, Chunks, Lighting, PhysicsSolver и другими компонентами,
 * необходимыми для игрового процесса. Также управляет созданием объектов (сущностей) и
 * содержит инвентари.
 */
class Level {
private:
	std::unique_ptr<World> world;
public:
     const Content* const content;
	std::unique_ptr<Chunks> chunks; ///< Менеджер чанков
	std::unique_ptr<PhysicsSolver> physics; ///< Физический солвер (обработка движения, гравитации)
	std::unique_ptr<Lighting> lighting; ///< Освещение чанков
     std::unique_ptr<ChunksStorage> chunksStorage; ///< Хранилище чанков
     std::unique_ptr<LevelEvents> events; ///< Обработчик событий уровня
     std::unique_ptr<Entities> entities;
     std::unique_ptr<Players> players;
     std::vector<std::shared_ptr<Camera>> cameras;
     const EngineSettings& settings;
	std::unique_ptr<Inventories> inventories; ///< Менеджер инвентарей (хранит все инвентари уровня)

	/**
     * @brief Конструктор уровня.
     * @param world Указатель на объект World.
     * @param content Указатель на контент.
     * @param settings Ссылка на настройки.
     */
	Level(
          std::unique_ptr<World> world, 
          const Content* content, 
          EngineSettings& settings
     );
	~Level();

     void loadMatrix(int32_t x, int32_t z, uint32_t radius);

	/**
     * @brief Возвращает указатель на мир.
     * @return Указатель на World (принадлежит Level).
     */
	World* getWorld();
     const World* getWorld() const;

     void onSave();

     std::shared_ptr<Camera> getCamera(const std::string& name);
};
