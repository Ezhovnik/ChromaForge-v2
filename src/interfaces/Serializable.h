#ifndef INTERFACES_SERIALIZABLE_H_
#define INTERFACES_SERIALIZABLE_H_

#include <memory>

#include "../coders/json.h"

class Serializable {
public:
    virtual ~Serializable() {}
    virtual std::unique_ptr<dynamic::Map> serialize() const = 0;
    virtual void deserialize(dynamic::Map* src) = 0;
};

#endif // INTERFACES_SERIALIZABLE_H_
