#ifndef INTERFACES_SERIALIZABLE_H_
#define INTERFACES_SERIALIZABLE_H_

#include <memory>

#include <coders/json.h>

/**
 * @brief Абстрактный интерфейс для объектов, поддерживающих сериализацию в JSON.
 *
 * Классы, наследующие Serializable, должны реализовать методы serialize() и deserialize().
 */
class Serializable {
public:
    virtual ~Serializable() {}

    /**
     * @brief Сериализует объект в JSON-объект (Map).
     * @return Умный указатель на динамический Map, содержащий данные объекта.
     */
    virtual std::unique_ptr<dynamic::Map> serialize() const = 0;

/**
     * @brief Восстанавливает состояние объекта из JSON-объекта.
     * @param src Указатель на динамический Map, содержащий сериализованные данные.
     */
    virtual void deserialize(dynamic::Map* src) = 0;
};

#endif // INTERFACES_SERIALIZABLE_H_
