#include <presets/ParticlesPreset.h>

#include <data/dv_util.h>

dv::value ParticlesPreset::serialize() const {
    auto root = dv::object();
    root["collision"] = collision;
    root["lighting"] = lighting;
    root["max_distance"] = maxDistance;
    root["acceleration"] = dv::to_value(acceleration);
    return root;
}

void ParticlesPreset::deserialize(const dv::value& src) {
    src.at("collision").get(collision);
    src.at("lighting").get(lighting);
    if (src.has("acceleration")) {
        dv::get_vec(src["acceleration"], acceleration);
    }
}
