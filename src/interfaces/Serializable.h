#pragma once

#include <memory>

#include <data/dv.h>

/**
 * @brief Абстрактный интерфейс для объектов, поддерживающих сериализацию в JSON.
 *
 * Классы, наследующие Serializable, должны реализовать методы serialize() и deserialize().
 */
class Serializable {
public:
    virtual ~Serializable() {}
    virtual dv::value serialize() const = 0;
    virtual void deserialize(const dv::value& src) = 0;
};
