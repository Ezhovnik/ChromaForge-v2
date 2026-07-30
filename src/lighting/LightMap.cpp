#include <lighting/Lightmap.h>

#include <cassert>
#include <cstring>

#include <util/data_io.h>

static_assert(sizeof(light_t) == 2, "Replace the dataio calls with the new light_t value");

void Lightmap::set(const Lightmap* lightmap) {
    set(lightmap->map);
}

void Lightmap::set(const light_t* map) {
    std::memcpy(this->map, map, sizeof(light_t) * CHUNK_VOLUME);
}

std::unique_ptr<ubyte[]> Lightmap::encode() const {
    auto buffer = std::make_unique<ubyte[]>(LIGHTMAP_DATA_LEN);
    for (uint i = 0; i < CHUNK_VOLUME; i += 2) {
        buffer[i / 2] = ((map[i] >> 12) & 0xF) | ((map[i + 1] >> 8) & 0xF0);
    }
    return buffer;
}

void Lightmap::decode(const ubyte* src) {
    for (uint i = 0; i < CHUNK_VOLUME; i += 2) {
        ubyte b = src[i/2];
        map[i] = ((b & 0xF) << 12);
        map[i + 1] = ((b & 0xF0) << 8);
    }
}
