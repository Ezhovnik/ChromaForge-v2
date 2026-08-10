#define CHROMA_ENABLE_REFLECTION
#include <world/Environment.h>

#include <data/dv.h>
#include <data/dv_util.h>

dv::value Environment::serialize() const {
    return dv::object({
        {
            "sky",
            dv::object({
                {"mode", SkyModeMeta.getNameString(sky.mode)},
                {"stars", sky.stars},
                {"clouds", sky.clouds},
                {"sprites", sky.sprites},
            })
        },
    });
}

void Environment::deserialize(const dv::value& src) {
    if (auto skyItem = src.at("sky")) {
        skyItem->at("mode").get(sky.mode, SkyModeMeta);
        skyItem->at("stars").get(sky.stars);
        skyItem->at("clouds").get(sky.clouds);
        skyItem->at("sprites").get(sky.sprites);
    }
}
