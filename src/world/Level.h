#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "../typedefs.h"
#include "../interfaces/Object.h"

inline constexpr glm::vec3 DEFAULT_SPAWNPOINT = {0, 256, 0}; ///< Точка появления игрока
inline constexpr float DEFAULT_PLAYER_SPEED = 5.0f; ///< Базовая скорость перемещения игрока
inline constexpr int DEFAULT_PLAYER_INVENTORY_SIZE = 40; ///< Размер инвентаря игрока (количество слотов)

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
	std::vector<std::shared_ptr<Object>> objects;
	std::unique_ptr<Chunks> chunks; ///< Менеджер чанков
	std::unique_ptr<PhysicsSolver> physics; ///< Физический солвер (обработка движения, гравитации)
	std::unique_ptr<Lighting> lighting; ///< Освещение чанков
     std::unique_ptr<ChunksStorage> chunksStorage; ///< Хранилище чанков
     std::unique_ptr<LevelEvents> events; ///< Обработчик событий уровня
     std::unique_ptr<Entities> entities;
     std::vector<std::shared_ptr<Camera>> cameras;
     const EngineSettings& settings;
     const Content* const content;
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

	/**
     * @brief Создаёт новый объект (сущность) типа T и добавляет его на уровень.
     * @tparam T Класс, производный от Object.
     * @param args Аргументы для конструктора T.
     * @return Умный указатель на созданный объект (shared_ptr<T>).
     */
	template<class T, typename... Args>
     std::shared_ptr<T> spawnObject(Args&&... args) {
          static_assert(std::is_base_of<Object, T>::value, "T must be a derived of Object class");
          std::shared_ptr<T> tObj = std::make_shared<T>(args...);

          std::shared_ptr<Object> obj = std::dynamic_pointer_cast<Object, T>(tObj);
          obj->objectUID = objects.size();
          objects.push_back(obj);
          obj->spawned();
          return tObj;
     }

     template<class T>
     std::shared_ptr<T> getObject(uint64_t id) {
          static_assert(std::is_base_of<Object, T>::value, "T must be a derived of Object class");
          if (id >= objects.size()) return nullptr;
          std::shared_ptr<T> object = std::dynamic_pointer_cast<T>(objects[id]);
          return object; 
     }

     void onSave();

     std::shared_ptr<Camera> getCamera(const std::string& name);
};

#endif // WORLD_LEVEL_H_
