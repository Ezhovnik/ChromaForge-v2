#include <files/WorldRegions.h>

#include <cstring>
#include <utility>

#include <util/data_io.h>
#include <coders/byte_utils.h>
#include <math/voxmaths.h>
#include <items/Inventory.h>
#include <debug/Logger.h>
#include <coders/binary_json.h>

WorldRegion::WorldRegion() :
    chunksData(std::make_unique<std::unique_ptr<ubyte[]>[]>(RegionConsts::VOLUME)),
    sizes(std::make_unique<glm::u32vec2[]>(RegionConsts::VOLUME))
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

glm::u32vec2* WorldRegion::getSizes() const {
    return sizes.get();
}

void WorldRegion::put(
    uint x, uint z, std::unique_ptr<ubyte[]> data, uint32_t size, uint32_t srcSize
) {
    size_t chunk_index = z * RegionConsts::SIZE + x;
    chunksData[chunk_index] = std::move(data);
    sizes[chunk_index] = glm::u32vec2(size, srcSize);
}

ubyte* WorldRegion::getChunkData(uint x, uint z) {
    return chunksData[z * RegionConsts::SIZE + x].get();
}

glm::u32vec2 WorldRegion::getChunkDataSize(uint x, uint z) {
    return sizes[z * RegionConsts::SIZE + x];
}

WorldRegions::WorldRegions(const std::filesystem::path& directory) : directory(directory) {
    for (size_t i = 0; i < REGION_LAYERS_COUNT; ++i) {
        layers[i].layer = static_cast<RegionLayerIndex>(i);
    }
    auto& voxels = layers[REGION_LAYER_VOXELS];
    voxels.folder = directory/std::filesystem::path("regions");
    voxels.compression = compression::Method::Extrle16;

    auto& lights = layers[REGION_LAYER_LIGHTS];
    lights.folder = directory/std::filesystem::path("lights");
    lights.compression = compression::Method::Extrle8;

    layers[REGION_LAYER_INVENTORIES].folder = directory/std::filesystem::path("inventories");

    layers[REGION_LAYER_ENTITIES].folder = directory/std::filesystem::path("entities");

    auto& blocksData = layers[REGION_LAYER_BLOCKS_DATA];
    blocksData.folder = directory/std::filesystem::path("blocksdata");
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

void WorldRegions::put(
    int x, int z,
    RegionLayerIndex layerID,
    std::unique_ptr<ubyte[]> data,
    size_t srcSize
) {
    size_t size = srcSize;
    auto& layer = layers[layerID];
    int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);

    WorldRegion* region = layer.getOrCreateRegion(regionX, regionZ);
    region->setUnsaved(true);

    if (data == nullptr) {
        region->put(localX, localZ, nullptr, 0, 0);
        return;
    }

    if (layer.compression != compression::Method::None) {
        data = compression::compress(
            data.get(), size, size, layer.compression
        );
    }

    region->put(localX, localZ, std::move(data), size, srcSize);
}

static std::unique_ptr<ubyte[]> write_inventories(
    const ChunkInventoriesMap& inventories, uint32_t& datasize
) {
    ByteBuilder builder;
    builder.putInt32(inventories.size());
    for (auto& entry : inventories) {
        builder.putInt32(entry.first);
        auto map = entry.second->serialize();
        auto bytes = json::to_binary(map, true);
        builder.putInt32(bytes.size());
        builder.put(bytes.data(), bytes.size());
    }   
    auto datavec = builder.data();
    datasize = builder.size();
    auto data = std::make_unique<ubyte[]>(datasize);
    std::memcpy(data.get(), datavec, datasize);
    return data;
}

static ChunkInventoriesMap load_inventories(
    const ubyte* src, uint32_t size
) {
    ChunkInventoriesMap inventories;
    ByteReader reader(src, size);
    auto count = reader.getInt32();
    for (int i = 0; i < count; ++i) {
        uint index = reader.getInt32();
        uint size = reader.getInt32();
        auto map = json::from_binary(reader.pointer(), size);
        reader.skip(size);
        auto inv = std::make_shared<Inventory>(0, 0);
        inv->deserialize(map);
        inventories[index] = inv;
    }
    return inventories;
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
        auto data = write_inventories(chunk->inventories, datasize);
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
    if (chunk->flags.blocksData) {
        auto bytes = chunk->blocksMetadata.serialize();
        put(chunk->chunk_x, chunk->chunk_z,
            REGION_LAYER_BLOCKS_DATA,
            bytes.release(),
            bytes.size()
        );
    }
}

std::unique_ptr<ubyte[]> WorldRegions::getVoxels(int x, int z){
    uint32_t size;
    uint32_t srcSize;
    auto& layer = layers[REGION_LAYER_VOXELS];
    auto* data = layer.getData(x, z, size, srcSize);
    if (data == nullptr) return nullptr;
    assert(srcSize == CHUNK_DATA_LEN);
    return compression::decompress(data, size, srcSize, layer.compression);
}

std::unique_ptr<light_t[]> WorldRegions::getLights(int x, int z) {
    uint32_t size;
    uint32_t srcSize;
    auto& layer = layers[REGION_LAYER_LIGHTS];
    auto* bytes = layer.getData(x, z, size, srcSize);
    if (bytes == nullptr) return nullptr;
    auto data = compression::decompress(
        bytes, size, srcSize, layer.compression
    );
    assert(srcSize == LIGHTMAP_DATA_LEN);
    return LightMap::decode(data.get());
}

ChunkInventoriesMap WorldRegions::fetchInventories(int x, int z) {
    uint32_t bytesSize;
    uint32_t srcSize;
    auto bytes = layers[REGION_LAYER_INVENTORIES].getData(x, z, bytesSize, srcSize);
    if (bytes == nullptr) return {};
    return load_inventories(bytes, bytesSize);
}

BlocksMetadata WorldRegions::getBlocksData(int x, int z) {
    uint32_t bytesSize;
    uint32_t srcSize;
    auto bytes = layers[REGION_LAYER_BLOCKS_DATA].getData(x, z, bytesSize, srcSize);
    if (bytes == nullptr) return {};
    BlocksMetadata heap;
    heap.deserialize(bytes, bytesSize);
    return heap;
}

void WorldRegions::processInventories(
    int x, int z, const InventoryProc& func
) {
    processRegion(x, z, REGION_LAYER_INVENTORIES,
    [=](std::unique_ptr<ubyte[]> data, uint32_t* size) {
        auto inventories = load_inventories(data.get(), *size);
        for (const auto& [_, inventory] : inventories) {
            func(inventory.get());
        }
        return write_inventories(inventories, *size);
    });
}

void WorldRegions::processBlocksData(int x, int z, const BlockDataProc& func) {
    auto& voxLayer = layers[REGION_LAYER_VOXELS];
    auto& datLayer = layers[REGION_LAYER_BLOCKS_DATA];
    if (voxLayer.getRegion(x, z) || datLayer.getRegion(x, z)) {
        LOG_ERROR("Not implemented for in-memory regions");
        throw std::runtime_error("Not implemented for in-memory regions");
    }
    auto datRegfile = datLayer.getRegFile({x, z});
    if (datRegfile == nullptr) {
        LOG_ERROR("Could not open region file");
        throw std::runtime_error("Could not open region file");
    }
    auto voxRegfile = voxLayer.getRegFile({x, z});
    if (voxRegfile == nullptr) {
        LOG_WARN("Missing voxels region - discard blocks data for {}x {}z", x, z);;
        deleteRegion(REGION_LAYER_BLOCKS_DATA, x, z);
        return;
    }
    for (uint cz = 0; cz < RegionConsts::SIZE; cz++) {
        for (uint cx = 0; cx < RegionConsts::SIZE; cx++) {
            int gx = cx + x * RegionConsts::SIZE;
            int gz = cz + z * RegionConsts::SIZE;

            uint32_t datLength;
            uint32_t datSrcSize;
            auto datData = RegionsLayer::readChunkData(
                gx, gz, datLength, datSrcSize, datRegfile.get()
            );
            if (datData == nullptr) {
                continue;
            }
            uint32_t voxLength;
            uint32_t voxSrcSize;
            auto voxData = RegionsLayer::readChunkData(
                gx, gz, voxLength, voxSrcSize, voxRegfile.get()
            );
            if (voxData == nullptr) {
                LOG_WARN("Missing voxels for chunk {}x {}z", gx, gz);
                put(gx, gz, REGION_LAYER_BLOCKS_DATA, nullptr, 0);
                continue;
            }
            voxData = compression::decompress(
                voxData.get(), voxLength, voxSrcSize, voxLayer.compression
            );

            BlocksMetadata blocksData;
            blocksData.deserialize(datData.get(), datLength);
            try {
                func(&blocksData, std::move(voxData));
            } catch (const std::exception& err) {
                LOG_WARN("An error ocurred while processing blocks data in chunk {}x {}z: {}", gx, gz, err.what());
                blocksData = {};
            }
            auto bytes = blocksData.serialize();
            put(gx, gz, REGION_LAYER_BLOCKS_DATA, bytes.release(), bytes.size());
        }
    }
}

dv::value WorldRegions::fetchEntities(int x, int z) {
    if (generatorTestMode) return nullptr;

    uint32_t bytesSize;
    uint32_t srcSize;
    const ubyte* data = layers[REGION_LAYER_ENTITIES].getData(x, z, bytesSize, srcSize);
    if (data == nullptr) return nullptr;
    auto map = json::from_binary(data, bytesSize);
    if (map.size() == 0) return nullptr;
    return map;
}

void WorldRegions::processRegion(
    int x, int z, RegionLayerIndex layerID, const RegionProc& func
) {
    auto& layer = layers[layerID];
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
            uint32_t srcSize;
            auto data = RegionsLayer::readChunkData(gx, gz, length, srcSize, regFile.get());
            if (data == nullptr) continue;
            if (layer.compression != compression::Method::None) {
                data = compression::decompress(
                    data.get(), length, srcSize, layer.compression
                );
            } else {
                srcSize = length;
            }
            if (auto writeData = func(std::move(data), &srcSize)) {
                put(gx, gz, layerID, std::move(writeData), srcSize);
            }
        }
    }
}

const std::filesystem::path& WorldRegions::getRegionsFolder(RegionLayerIndex layerID) const {
    return layers[layerID].folder;
}

std::filesystem::path WorldRegions::getRegionFilePath(RegionLayerIndex layerID, int x, int z) const {
    return layers[layerID].getRegionFilePath(x, z);
}

void WorldRegions::writeAll() {
    for (auto& layer : layers) {
        std::filesystem::create_directories(layer.folder);
        layer.writeAll();
    }
}

void WorldRegions::deleteRegion(RegionLayerIndex layerid, int x, int z) {
    auto& layer = layers[layerid];
    if (layer.getRegFile({x, z}, false)) {
        LOG_ERROR("Region file is currently in use");
        throw std::runtime_error("Region file is currently in use");
    }
    auto file = layer.getRegionFilePath(x, z);
    if (std::filesystem::exists(file)) {
        LOG_INFO("Remove region file {}", file.u8string());
        std::filesystem::remove(file);
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
