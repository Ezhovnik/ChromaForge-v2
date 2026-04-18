#ifndef CONTENT_CONTENT_FWD_H_
#define CONTENT_CONTENT_FWD_H_

#include "typedefs.h"

class Content;
class ContentPackRuntime;

enum class ContentType {
    None,
    Block,
    Item,
    Entity
};

enum class ResourceType : size_t {
    Camera,
    Last=Camera
};

inline constexpr auto RESOURCE_TYPES_COUNT = static_cast<size_t>(ResourceType::Last) + 1;

#endif // CONTENT_CONTENT_FWD_H_
