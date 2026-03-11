#ifndef INTERFACES_OBJECT_H_
#define INTERFACES_OBJECT_H_

#include <stdlib.h>
#include <stdint.h>

class Level;

/**
 * @brief Базовый класс для всех объектов (сущностей) в игровом мире.
 */
class Object {
private:

public:
    uint64_t objectUID; ///< Уникальный идентификатор объекта (задаётся уровнем при создании)    
    bool shouldUpdate = true; ///< Флаг, указывающий, нужно ли обновлять объект каждый кадр

public:
    ~Object() { destroyed(); }

public:
    /**
     * Создаёт объект в игровом мире.
     */
    virtual void spawned() {  }

    /**
     * @brief Вызывается каждый кадр для обновления состояния объекта.
     * @param delta Время, прошедшее с предыдущего кадра (в секундах).
     */
    virtual void update(float delta) { }

    /**
     * @brief Вызывается перед уничтожением объекта.
     */
    virtual void destroyed() {  }
};

#endif // INTERFACES_OBJECT_H_
