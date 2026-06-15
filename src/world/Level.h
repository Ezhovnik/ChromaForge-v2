#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include <typedefs.h>

class World;
class PhysicsSolver;
class GlobalChunks;
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
     const EngineSettings& settings;
	std::unique_ptr<World> world;
public:
     const Content* const content;
	std::unique_ptr<PhysicsSolver> physics; ///< Физический солвер (обработка движения, гравитации)
     std::unique_ptr<GlobalChunks> chunks; ///< Хранилище чанков
     std::unique_ptr<LevelEvents> events; ///< Обработчик событий уровня
     std::unique_ptr<Entities> entities;
     std::unique_ptr<Players> players;
     std::vector<std::shared_ptr<Camera>> cameras;
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

	/**
     * @brief Возвращает указатель на мир.
     * @return Указатель на World (принадлежит Level).
     */
	World* getWorld();
     const World* getWorld() const;

     void onSave();

     std::shared_ptr<Camera> getCamera(const std::string& name);
};
