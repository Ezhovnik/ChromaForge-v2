#pragma once

#include <memory>
#include <optional>

#include <glm/glm.hpp>

#include <voxels/voxel.h>
#include <data/dv.h>
#include <interfaces/Serializable.h>
#include <constants.h>
#include <util/Interpolation.h>

class Camera;
struct Hitbox;
class Level;
class Inventory;
class ContentReport;
class Chunks;

/**
 * @brief Структура, содержащая текущее состояние ввода игрока.
 */
struct PlayerInput {
    struct {
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
        bool attack : 1;
        bool destroy : 1;
        bool build : 1;
        bool dropBlock : 1;
    };
    glm::vec2 delta;
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
class Player : public Serializable {
private:
    Level& level;

    int64_t id;

    std::string name;

	float speed;
	int chosenSlot;

    glm::vec3 position;

	glm::vec3 spawnpoint {};
	std::shared_ptr<Inventory> inventory;

    bool suspended = false;
	bool flight = false;
    bool noclip = false;
    bool infiniteItems = true;
    bool instantDestruction = true;
    bool loadingChunks = true;

    entityid_t eid = ENTITY_AUTO;

    entityid_t selectedEid = 0;

    glm::vec3 rotation {}; ///< Углы поворота камеры
public:
    util::VecInterpolation<3, float, true> rotationInterpolation {true};

    std::unique_ptr<Chunks> chunks;
	std::shared_ptr<Camera> fpCamera, spCamera, tpCamera; ///< Камеры: от первого лица, от третьего лица (спереди/сзади)
    std::shared_ptr<Camera> currentCamera; ///< Текущая активная камера

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
        Level& level,
        int64_t id,
        const std::string& name,
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

    bool isInfiniteItems() const;
    void setInfiniteItems(bool flag);

    bool isInstantDestruction() const;
    void setInstantDestruction(bool flag);

    bool isLoadingChunks() const;
    void setLoadingChunks(bool flag);

    bool isSuspended() const;
    void setSuspended(bool flag);

    entityid_t getEntity() const;
    void setEntity(entityid_t eid);

    void setName(const std::string& name);
    const std::string& getName() const;

    const glm::vec3& getPosition() const {
        return position;
    }

    Hitbox* getHitbox();

	/**
     * @brief Возвращает инвентарь игрока.
     */
	const std::shared_ptr<Inventory>& getInventory() const;

	/**
     * @brief Устанавливает точку возрождения.
     * @param point Координаты точки.
     */
	void setSpawnPoint(glm::vec3 point);

	/**
     * @brief Возвращает точку возрождения.
     */
    glm::vec3 getSpawnPoint() const;

    glm::vec3 getRotation(bool interpolated=false) const;
    void setRotation(const glm::vec3& rotation);

	/**
     * @brief Обновляет состояние игрока (физика, ввод, камера).
     * @param input Структура ввода.
     * @param delta Время с предыдущего кадра.
     */
	void updateInput(PlayerInput& input, float delta);

    void updateSelectedEntity();
    entityid_t getSelectedEntity() const;

    void postUpdate();

	dv::value serialize() const override;
    void deserialize(const dv::value& src) override;

    static void convert(dv::value& data, const ContentReport* report);

	inline uint64_t getId() const {
        return id;
    }
};
