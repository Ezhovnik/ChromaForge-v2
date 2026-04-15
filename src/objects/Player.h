#ifndef OBJECTS_PLAYER_H_
#define OBJECTS_PLAYER_H_

#include <memory>
#include <optional>

#include <glm/glm.hpp>

#include "voxels/voxel.h"
#include "data/dynamic.h"
#include "interfaces/Serializable.h"
#include "interfaces/Object.h"
#include "constants.h"

class Camera;
struct Hitbox;
class Level;
class Inventory;
class ContentLUT;

/**
 * @brief Структура, содержащая текущее состояние ввода игрока.
 */
struct PlayerInput {
	bool zoom : 1;
	bool cameraMode : 1; ///< Переключение режима камеры
	bool moveForward : 1;
	bool moveBack : 1;
	bool moveRight : 1;
	bool moveLeft : 1;
	bool sprint : 1;
	bool crouch : 1;
	bool cheat : 1;
	bool jump : 1;
	bool noclip : 1;
	bool flight : 1;
    bool attack : 1;
    bool build : 1;
    bool pickBlock : 1;
    bool dropBlock : 1;
};

struct CursorSelection {
    voxel vox {BLOCK_VOID, {}};
    glm::ivec3 position {};
    glm::ivec3 actualPosition {};
    glm::ivec3 normal {};
    glm::vec3 hitPosition;
    entityid_t entity = ENTITY_NONE;
};

/**
 * @brief Класс игрока, наследующий Object и Serializable.
 *
 * Управляет состоянием игрока: позицией, скоростью, инвентарём, камерами,
 * режимами полёта и noclip. Также обрабатывает ввод и обновление физики.
 */
class Player : public Object, public Serializable {
private:
    Level* level;

	float speed;
	int chosenSlot;

    glm::vec3 position;

	glm::vec3 spawnpoint {};
	std::shared_ptr<Inventory> inventory;

	bool flight = false;
    bool noclip = false;

    entityid_t eid;

    entityid_t selectedEid;
public:
	std::shared_ptr<Camera> camera, spCamera, tpCamera; ///< Камеры: от первого лица, от третьего лица (спереди/сзади)
    std::shared_ptr<Camera> currentCamera; ///< Текущая активная камера

    bool debug = false;

	glm::vec3 cam {}; ///< Углы поворота камеры

	CursorSelection selection {};

	/**
	 * @brief Конструктор игрока.
     * @param level Указатель на текущий уровень.
	 * @param position Начальная позиция.
	 * @param speed Базовая скорость.
	 * @param inventory Инвентарь (shared_ptr).
     * @param eid Идентификатор сущности.
	 */
	Player(
        Level* level,
        glm::vec3 position,
        float speed,
        std::shared_ptr<Inventory> inventory,
        entityid_t eid
    );
	~Player();

	/**
     * @brief Телепортирует игрока в указанную точку.
     * @param position Новая позиция.
     */
    void teleport(glm::vec3 position);

    void updateEntity();

	/**
     * @brief Возвращает скорость игрока.
     */
	float getSpeed() const;

	/**
     * @brief Пытается найти безопасную точку возрождения в мире.
     */
	void attemptToFindSpawnpoint();

	/**
     * @brief Устанавливает выбранный слот инвентаря.
     * @param index Индекс слота.
     */
	void setChosenSlot(int index);

	/**
     * @brief Возвращает текущий выбранный слот.
     */
	int getChosenSlot() const;

	bool isFlight() const;
    void setFlight(bool flag);

    bool isNoclip() const;
    void setNoclip(bool flag);

    entityid_t getEntity() const;
    void setEntity(entityid_t eid);

    glm::vec3 getPosition() const {
        return position;
    }

    Hitbox* getHitbox();

	/**
     * @brief Возвращает инвентарь игрока.
     */
	std::shared_ptr<Inventory> getInventory() const;

	/**
     * @brief Устанавливает точку возрождения.
     * @param point Координаты точки.
     */
	void setSpawnPoint(glm::vec3 point);

	/**
     * @brief Возвращает точку возрождения.
     */
    glm::vec3 getSpawnPoint() const;

	/**
     * @brief Обновляет состояние игрока (физика, ввод, камера).
     * @param input Структура ввода.
     * @param delta Время с предыдущего кадра.
     */
	void updateInput(PlayerInput& input, float delta);

    void updateSelectedEntity();
    entityid_t getSelectedEntity() const;

    void postUpdate();

	std::unique_ptr<dynamic::Map> serialize() const override;
    void deserialize(dynamic::Map *src) override;

	/**
     * @brief Конвертирует старые данные игрока при обновлении контента.
     * @param data JSON-объект с данными игрока.
     * @param lut Таблица соответствия старых и новых идентификаторов.
     */
    static void convert(dynamic::Map* data, const ContentLUT* lut);

	inline int getId() const {
        return objectUID;
    }
};

#endif // OBJECTS_PLAYER_H_
