#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

#include <memory>
#include <list>

#include "../typedefs.h"
#include "../settings.h"

class World;
class Player;
class Chunks;
class Lighting;
class PhysicsSolver;
class ChunksStorage;
class LevelEvents;
class Content;
class Inventories;
class Object;

/**
 * @brief Основной класс уровня, объединяющий мир, игрока, чанки, физику и освещение.
 *
 * Level владеет объектами World, Chunks, Lighting, PhysicsSolver и другими компонентами,
 * необходимыми для игрового процесса. Также управляет созданием объектов (сущностей) и
 * содержит инвентари.
 */
class Level {
private:
	int objCounter; ///< Счётчик для присвоения уникальных идентификаторов объектам (UID).
public:
	std::unique_ptr<World> world;
	std::list<std::shared_ptr<Object>> objects;
	std::shared_ptr<Player> player; ///< Игрок (основная сущность)
	Chunks* chunks; ///< Менеджер чанков
	PhysicsSolver* physics; ///< Физический солвер (обработка движения, гравитации)
	Lighting* lighting; ///< Освещение чанков
    ChunksStorage* chunksStorage; ///< Хранилище чанков
    LevelEvents* events; ///< Обработчик событий уровня
    const EngineSettings& settings;
    const Content* const content;
	std::unique_ptr<Inventories> inventories; ///< Менеджер инвентарей (хранит все инвентари уровня)

	/**
     * @brief Конструктор уровня.
     * @param world Указатель на объект World (передаёт владение Level).
     * @param content Указатель на контент.
     * @param settings Ссылка на настройки.
     */
	Level(World* world, const Content* content, EngineSettings& settings);
	~Level();

	/**
     * @brief Обновляет состояние уровня (вызывается каждый кадр).
     * Обновляет позицию центра чанков относительно игрока,
     * проверяет необходимость изменения размера сетки чанков и т.д.
     */
    void update();

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
     *
     * @note Определение шаблона находится в файле реализации (.cpp), поэтому
     *       его можно вызывать только из этого же translation unit (Level.cpp).
     */
	template<class T, typename... Args>
	std::shared_ptr<T> spawnObject(Args&&... args);
};

#endif // WORLD_LEVEL_H_
