#pragma once

#include <optional>

#include <interfaces/Serializable.h>
#include <presets/ParticlesPreset.h>

struct WeatherPreset : Serializable {
    struct {
        std::string texture;
        std::string noise;
        float vspeed = 1.0f;
        float hspeed = 0.1f;
        float scale = 0.1f;
        float minOpacity = 0.8f;
        std::optional<ParticlesPreset> splash;
    } fall {};

    float fogOpacity = 0.8f;
    float fogDencity = 2.0f;
    float clouds = 0.5f;
    float intensity = 0.9f;

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
