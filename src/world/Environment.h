#pragma once

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

class Environment : public Serializable {
public:
    struct {
        SkyMode mode = SkyMode::Box;
        bool stars = true;
        bool clouds = true;
        bool sprites = true;
    } sky;

    Environment() = default;

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
private:
};
