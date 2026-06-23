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
        float minOpacity = 0.0f;
        float maxOpacity = 1.0f;
        float maxIntensity = 1.0f;
        bool opaque = false;
        std::optional<ParticlesPreset> splash;
    } fall {};

    float fogOpacity = 0.0f;
    float fogDencity = 1.0f;
    float fogCurve = 1.0f;
    float clouds = 0.0f;
    float thunderRate = 0.0f;
    float intensity = 1.0f;

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
