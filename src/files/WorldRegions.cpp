#include <files/WorldRegions.h>

#include <cstring>
#include <utility>

#include <util/data_io.h>
#include <coders/byte_utils.h>
#include <math/voxmaths.h>
#include <items/Inventory.h>
#include <debug/Logger.h>
#include <coders/json.h>

WorldRegion::WorldRegion() :
    chunksData(std::make_unique<std::unique_ptr<ubyte[]>[]>(RegionConsts::VOLUME)),
    sizes(std::make_unique<uint32_t[]>(RegionConsts::VOLUME))
{}

WorldRegion::~WorldRegion() = default;

void WorldRegion::setUnsaved(bool unsaved) {
    this->unsaved = unsaved;
}
bool WorldRegion::isUnsaved() const {
    return unsaved;
}

std::unique_ptr<ubyte[]>* WorldRegion::getChunks() const {
    return chunksData.get();
}

uint32_t* WorldRegion::getSizes() const {
    return sizes.get();
}

void WorldRegion::put(uint x, uint z, std::unique_ptr<ubyte[]> data, uint32_t size) {
    size_t chunk_index = z * RegionConsts::SIZE + x;
    chunksData[chunk_index] = std::move(data);
    sizes[chunk_index] = size;
}

ubyte* WorldRegion::getChunkData(uint x, uint z) {
    return chunksData[z * RegionConsts::SIZE + x].get();
}

uint WorldRegion::getChunkDataSize(uint x, uint z) {
    return sizes[z * RegionConsts::SIZE + x];
}

WorldRegions::WorldRegions(const std::filesystem::path& directory) : directory(directory) {
    for (size_t i = 0; i < REGION_LAYERS_COUNT; ++i) {
        layers[i].layer = static_cast<RegionLayerIndex>(i);
    }
    auto& voxels = layers[REGION_LAYER_VOXELS];
    voxels.folder = directory/std::filesystem::path("regions");
    voxels.compression = compression::Method::Extrle8;

    auto& lights = layers[REGION_LAYER_LIGHTS];
    lights.folder = directory/std::filesystem::path("lights");
    lights.compression = compression::Method::Extrle8;

    layers[REGION_LAYER_INVENTORIES].folder = directory/std::filesystem::path("inventories");

    layers[REGION_LAYER_ENTITIES].folder = directory/std::filesystem::path("entities");
}

WorldRegions::~WorldRegions() = default;

void RegionsLayer::writeAll() {
    for (auto& it : regions) {
        WorldRegion* region = it.second.get();
        if (region->getChunks() == nullptr || !region->isUnsaved()) continue;
        const auto& key = it.first;
        writeRegion(key[0], key[1], region);
    }
}

void WorldRegions::put(int x, int z, RegionLayerIndex layerID, std::unique_ptr<ubyte[]> data, size_t size) {
    auto& layer = layers[layerID];
    if (layer.compression != compression::Method::None) {
        data = compression::compress(data.get(), size, size, layer.compression);
    }
    int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);

    WorldRegion* region = layer.getOrCreateRegion(regionX, regionZ);
    region->setUnsaved(true);
    region->put(localX, localZ, std::move(data), size);
}

static std::unique_ptr<ubyte[]> write_inventories(Chunk* chunk, uint& datasize) {
    auto& inventories = chunk->inventories;
    ByteBuilder builder;
    builder.putInt32(inventories.size());
    for (auto& entry : inventories) {
        builder.putInt32(entry.first);
        auto map = entry.second->serialize();
        auto bytes = json::to_binary(map.get(), true);
        builder.putInt32(bytes.size());
        builder.put(bytes.data(), bytes.size());
    }   
    auto datavec = builder.data();
    datasize = builder.size();
    auto data = std::make_unique<ubyte[]>(datasize);
    std::memcpy(data.get(), datavec, datasize);
    return data;
}

void WorldRegions::put(Chunk* chunk, std::vector<ubyte> entitiesData) {
    assert(chunk != nullptr);

    if (!chunk->flags.lighted) return;
    bool lightsUnsaved = !chunk->flags.loadedLights && doWriteLights;
    if (!chunk->flags.unsaved && !lightsUnsaved && !chunk->flags.entities) return;

    int regionX, regionZ, localX, localZ;
    calc_reg_coords(chunk->chunk_x, chunk->chunk_z, regionX, regionZ, localX, localZ);

    put(
        chunk->chunk_x, chunk->chunk_z,
        REGION_LAYER_VOXELS,
        chunk->encode(),
        CHUNK_DATA_LEN
    );

    if (doWriteLights && chunk->flags.lighted) {
        put(
            chunk->chunk_x, chunk->chunk_z,
            REGION_LAYER_LIGHTS, 
            chunk->light_map.encode(),
            LIGHTMAP_DATA_LEN
        );
    }
    if (!chunk->inventories.empty()) {
        uint datasize;
        auto data = write_inventories(chunk, datasize);
        put(
            chunk->chunk_x, chunk->chunk_z,
            REGION_LAYER_INVENTORIES,
            std::move(data),
            datasize
        );
    }
    if (!entitiesData.empty()) {
        auto data = std::make_unique<ubyte[]>(entitiesData.size());
        for (size_t i = 0; i < entitiesData.size(); ++i) {
            data[i] = entitiesData[i];
        }
        put(
            chunk->chunk_x, chunk->chunk_z,
            REGION_LAYER_ENTITIES,
            std::move(data),
            entitiesData.size()
        );
    }
}

std::unique_ptr<ubyte[]> WorldRegions::getVoxels(int x, int z){
    uint32_t size;
    auto& layer = layers[REGION_LAYER_VOXELS];
    auto* data = layer.getData(x, z, size);
    if (data == nullptr) return nullptr;
    return compression::decompress(data, size, CHUNK_DATA_LEN, layer.compression);
}

std::unique_ptr<light_t[]> WorldRegions::getLights(int x, int z) {
    uint32_t size;
    auto& layer = layers[REGION_LAYER_LIGHTS];
    auto* bytes = layer.getData(x, z, size);
    if (bytes == nullptr) return nullptr;
    auto data = compression::decompress(
        bytes, size, LIGHTMAP_DATA_LEN, layer.compression
    );
    return LightMap::decode(data.get());
}

chunk_inventories_map WorldRegions::fetchInventories(int x, int z) {
    chunk_inventories_map meta;
    uint32_t bytesSize;
    auto bytes = layers[REGION_LAYER_INVENTORIES].getData(x, z, bytesSize);
    if (bytes == nullptr) return meta;
    ByteReader reader(bytes, bytesSize);
    auto count = reader.getInt32();
    for (int i = 0; i < count; ++i) {
        uint index = reader.getInt32();
        uint size = reader.getInt32();
        auto map = json::from_binary(reader.pointer(), size);
        reader.skip(size);
        auto inv = std::make_shared<Inventory>(0, 0);
        inv->deserialize(map.get());
        meta[index] = inv;
    }
    return meta;
}

dynamic::Map_sptr WorldRegions::fetchEntities(int x, int z) {
    if (generatorTestMode) return nullptr;

    uint32_t bytesSize;
    const ubyte* data = layers[REGION_LAYER_ENTITIES].getData(x, z, bytesSize);
    if (data == nullptr) return nullptr;
    auto map = json::from_binary(data, bytesSize);
    if (map->size() == 0) return nullptr;
    return map;
}

void WorldRegions::processRegionVoxels(int x, int z, const regionproc& func) {
    auto& layer = layers[REGION_LAYER_VOXELS];
    if (layer.getRegion(x, z)) {
        LOG_ERROR("Not implemented for in-memory regions");
        throw std::runtime_error("Not implemented for in-memory regions");
    }
    auto regFile = layer.getRegFile({x, z});
    if (regFile == nullptr) {
        LOG_ERROR("Could not open region file");
        throw std::runtime_error("Could not open region file");
    }
    for (uint cz = 0; cz < RegionConsts::SIZE; ++cz) {
        int gz = cz + z * RegionConsts::SIZE;
        for (uint cx = 0; cx < RegionConsts::SIZE; ++cx) {
            int gx = cx + x * RegionConsts::SIZE;
            uint32_t length;
            auto data = RegionsLayer::readChunkData(gx, gz, length, regFile.get());
            if (data == nullptr) continue;
            data = compression::decompress(
                data.get(), length, CHUNK_DATA_LEN, layer.compression
            );
            if (func(data.get())) {
                put(
                    gx, gz,
                    REGION_LAYER_VOXELS,
                    std::move(data),
                    CHUNK_DATA_LEN
                );
            }
        }
    }
}

std::filesystem::path WorldRegions::getRegionsFolder(int layer) const {
    return layers[layer].folder;
}

void WorldRegions::write() {
    for (auto& layer : layers) {
        std::filesystem::create_directories(layer.folder);
        layer.writeAll();
    }
}

bool WorldRegions::parseRegionFilename(const std::string& name, int& x, int& z) {
    size_t sep = name.find('_');
    if (sep == std::string::npos || sep == 0 || sep == name.length() - 1) return false;
    try {
        x = std::stoi(name.substr(0, sep));
        z = std::stoi(name.substr(sep + 1));
    } catch (std::invalid_argument& err) {
        return false;
    } catch (std::out_of_range& err) {
        return false;
    }
    return true;
}
