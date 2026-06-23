#pragma once

#include <interfaces/Serializable.h>


struct WeatherPreset : Serializable {
    struct {
        std::string texture;
        float vspeed = 1.0f;
        float hspeed = 0.1f;
        float scale = 0.1f;
    } fall {};

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
