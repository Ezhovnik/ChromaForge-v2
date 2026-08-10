#pragma once

#include <vector>

#include <glm/gtc/constants.hpp>

#include <interfaces/Serializable.h>
#include <util/EnumMetadata.h>

enum class SkyMode {
    None,
    Solid,
    Box
};

CHROMA_ENUM_METADATA(SkyMode)
    {"none", SkyMode::None},
    {"solid", SkyMode::Solid},
    {"box", SkyMode::Box},
CHROMA_ENUM_END

struct SkySprite {
    std::string texture;
    float phase;
    float distance;
    bool emissive;
    float altitude;
};

class Environment : public Serializable {
public:
    struct {
        SkyMode mode = SkyMode::Box;
        bool stars = true;
        bool clouds = true;

        // TODO: move standart to config file
        std::vector<SkySprite> sprites {
            SkySprite {
                "misc/moon",
                glm::pi<float>() * 0.5f,
                4.0f,
                false,
                glm::pi<float>() * 0.25f,
            },
            SkySprite {
                "misc/moon_flare",
                glm::pi<float>() * 0.5f,
                0.5f,
                false,
                glm::pi<float>() * 0.25f,
            },
            SkySprite {
                "misc/sun",
                glm::pi<float>() * 1.5f,
                4.0f,
                true,
                glm::pi<float>() * 0.25f,
            },
        };
    } sky;

    std::string generator;

    Environment() = default;

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
private:
};
