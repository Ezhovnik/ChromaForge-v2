#pragma once

#include <typedefs.h>
#include <util/EnumMetadata.h>

class Content;
class ContentPackRuntime;

enum class ContentType {
    None,
    Block,
    Item,
    Entity,
    Generator
};

CHROMA_ENUM_METADATA(ContentType)
    {"none", ContentType::None},
    {"block", ContentType::Block},
    {"item", ContentType::Item},
    {"entity", ContentType::Entity},
    {"generator", ContentType::Generator},
CHROMA_ENUM_END

enum class ResourceType : size_t {
    Camera,
    PostEffectSlot,

    Last=PostEffectSlot
};

inline constexpr auto RESOURCE_TYPES_COUNT = static_cast<size_t>(ResourceType::Last) + 1;

CHROMA_ENUM_METADATA(ResourceType)
    {"camera", ResourceType::Camera},
    {"post-effect-slot", ResourceType::PostEffectSlot},
CHROMA_ENUM_END
