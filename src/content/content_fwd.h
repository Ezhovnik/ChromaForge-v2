#pragma once

#include <typedefs.h>

class Content;
class ContentPackRuntime;

enum class ContentType {
    None,
    Block,
    Item,
    Entity,
    Generator
};

enum class ResourceType : size_t {
    Camera,
    Last=Camera
};

inline constexpr auto RESOURCE_TYPES_COUNT = static_cast<size_t>(ResourceType::Last) + 1;
