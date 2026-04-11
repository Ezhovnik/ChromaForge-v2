#include "WorldRegions.h"

#include <cstring>
#include <utility>

#include "../coders/rle.h"
#include "../util/data_io.h"
#include "../coders/byte_utils.h"
#include "../math/voxmaths.h"
#include "../items/Inventory.h"
#include "../debug/Logger.h"

#define REGION_FORMAT_MAGIC ".CHROMAREG"
inline constexpr int REGION_HEADER_SIZE = 13;

regFile::regFile(std::filesystem::path filename) : file(std::move(filename)) {
    if (file.length() < REGION_HEADER_SIZE) {
        LOG_ERROR("Incomplete region file header");
        throw std::runtime_error("Incomplete region file header");
    }

    char header[REGION_HEADER_SIZE];
    file.read(header, REGION_HEADER_SIZE);

    if (std::string(header, strlen(REGION_FORMAT_MAGIC)) != REGION_FORMAT_MAGIC) {
        LOG_ERROR("Invalid region file magic number");
        throw std::runtime_error("Invalid region file magic number");
    }
    version = header[REGION_HEADER_SIZE - 2];
    if (uint(version) > RegionConsts::FORMAT_VERSION) {
        LOG_ERROR("Region format {} is not supported", version);
        throw illegal_region_format(
            "Region format " + std::to_string(version) + " is not supported"
        );
    }
}

std::unique_ptr<ubyte[]> regFile::read(int index, uint32_t& length) {
    size_t file_size = file.length();
    size_t table_offset = file_size - RegionConsts::VOLUME * 4;

    uint32_t offset;
    file.seekg(table_offset + index * 4);
    file.read(reinterpret_cast<char*>(&offset), 4);
    offset = dataio::read_int32_big(reinterpret_cast<const ubyte*>(&offset), 0);
    if (offset == 0) return nullptr;

    file.seekg(offset);
    file.read(reinterpret_cast<char*>(&offset), 4);
    length = dataio::read_int32_big(reinterpret_cast<const ubyte*>(&offset), 0);
    auto data = std::make_unique<ubyte[]>(length);
    file.read(reinterpret_cast<char*>(data.get()), length);
    return data;
}

WorldRegion::WorldRegion() :
    chunksData(std::make_unique<std::unique_ptr<ubyte[]>[]>(RegionConsts::VOLUME)),
    sizes(std::make_unique<uint32_t[]>(RegionConsts::VOLUME))
{}

WorldRegion::~WorldRegion() {
}

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

void WorldRegion::put(uint x, uint z, ubyte* data, uint32_t size) {
    size_t chunk_index = z * RegionConsts::SIZE + x;
    chunksData[chunk_index].reset(data);
    sizes[chunk_index] = size;
}

ubyte* WorldRegion::getChunkData(uint x, uint z) {
    return chunksData[z * RegionConsts::SIZE + x].get();
}

uint WorldRegion::getChunkDataSize(uint x, uint z) {
    return sizes[z * RegionConsts::SIZE + x];
}

WorldRegions::WorldRegions(const std::filesystem::path& directory) : directory(directory) {
    for (size_t i = 0; i < sizeof(layers) / sizeof(RegionsLayer); ++i) {
        layers[i].layer = i;
    }
    layers[RegionConsts::LAYER_VOXELS].folder = directory/std::filesystem::path("regions");
    layers[RegionConsts::LAYER_LIGHTS].folder = directory/std::filesystem::path("lights");
    layers[RegionConsts::LAYER_INVENTORIES].folder = directory/std::filesystem::path("inventories");
    layers[RegionConsts::LAYER_ENTITIES].folder = directory/std::filesystem::path("entities");
}

WorldRegions::~WorldRegions() {
}

WorldRegion* WorldRegions::getRegion(int x, int z, int layer) {
    RegionsLayer& regions = layers[layer];
    std::lock_guard lock(regions.mutex);
    auto found = regions.regions.find(glm::ivec2(x, z));
    if (found == regions.regions.end()) return nullptr;
    return found->second.get();
}

WorldRegion* WorldRegions::getOrCreateRegion(int x, int z, int layer) {
    if (auto region = getRegion(x, z, layer)) {
        return region;
    }
    RegionsLayer& regions = layers[layer];
    std::lock_guard lock(regions.mutex);
    auto region_ptr = std::make_unique<WorldRegion>();
    auto region = region_ptr.get();
    regions.regions[{x, z}] = std::move(region_ptr);
    return region;
}

std::unique_ptr<ubyte[]> WorldRegions::compress(const ubyte* src, size_t srclen, size_t& len) {
    auto buffer = bufferPool.get();
    auto bytes = buffer.get();

    len = extrle::encode(src, srclen, bytes);
    auto data = std::make_unique<ubyte[]>(len);
    for (size_t i = 0; i < len; ++i) {
        data[i] = bytes[i];
    }
    return data;
}

std::unique_ptr<ubyte[]> WorldRegions::decompress(const ubyte* src, size_t srclen, size_t dstlen) {
    auto decompressed = std::make_unique<ubyte[]>(dstlen);
    extrle::decode(src, srclen, decompressed.get());
    return decompressed;
}

inline void calc_reg_coords(int x, int z, int& regionX, int& regionZ, int& localX, int& localZ) {
    regionX = floordiv(x, RegionConsts::SIZE);
    regionZ = floordiv(z, RegionConsts::SIZE);
    localX = x - (regionX * RegionConsts::SIZE);
    localZ = z - (regionZ * RegionConsts::SIZE);
}

std::unique_ptr<ubyte[]> WorldRegions::readChunkData(
    int x, 
    int z, 
    uint32_t& length, 
    regFile* rfile
) {
    int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);
    int chunkIndex = localZ * RegionConsts::SIZE + localX;

    return rfile->read(chunkIndex, length);
}

void WorldRegions::fetchChunks(WorldRegion* region, int x, int z, regFile* file) {
    auto* chunks = region->getChunks();
    uint32_t* sizes = region->getSizes();

    for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
        int chunk_x = (i % RegionConsts::SIZE) + x * RegionConsts::SIZE;
        int chunk_z = (i / RegionConsts::SIZE) + z * RegionConsts::SIZE;
        if (chunks[i] == nullptr) {
            chunks[i] = readChunkData(chunk_x, chunk_z, sizes[i], file);
        }
    }
}

ubyte* WorldRegions::getData(
    int x, int z, int layer, 
    uint32_t& size
) {
    if (generatorTestMode) return nullptr;

    int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);

    WorldRegion* region = getOrCreateRegion(regionX, regionZ, layer);
    ubyte* data = region->getChunkData(localX, localZ);
    if (data == nullptr) {
        auto regFile = getRegFile(glm::ivec3(regionX, regionZ, layer));
        if (regFile != nullptr) {
            data = readChunkData(x, z, size, regFile.get()).release();
        }
        if (data != nullptr) {
            region->put(localX, localZ, data, size);
        }
    }
    if (data != nullptr) {
        size = region->getChunkDataSize(localX, localZ);
        return data;
    }
    return nullptr;
}

regFile_ptr WorldRegions::useRegFile(glm::ivec3 coord) {
    auto* file = openRegFiles[coord].get();
    file->inUse = true;
    return regFile_ptr(file, &regFilesCv);
}

void WorldRegions::closeRegFile(glm::ivec3 coord) {
    openRegFiles.erase(coord);
    regFilesCv.notify_one();
}

regFile_ptr WorldRegions::getRegFile(glm::ivec3 coord, bool create) {
    {
        std::lock_guard lock(regFilesMutex);
        const auto found = openRegFiles.find(coord);
        if (found != openRegFiles.end()) {
            if (found->second->inUse) {
                LOG_ERROR("regFile is currentry in use");
                throw std::runtime_error("regFile is currently in use");
            }
            return useRegFile(found->first);
        }
    }
    if (create) return createRegFile(coord);
    return nullptr;
}

regFile_ptr WorldRegions::createRegFile(glm::ivec3 coord) {
    std::filesystem::path file = layers[coord[2]].folder/getRegionFilename(coord[0], coord[1]);
    if (!std::filesystem::exists(file)) return nullptr;

    if (openRegFiles.size() == RegionConsts::MAX_OPEN_FILES) {
        std::unique_lock lock(regFilesMutex);
        while (true) {
            bool closed = false;
            for (auto& entry : openRegFiles) {
                if (!entry.second->inUse) {
                    closeRegFile(entry.first);
                    closed = true;
                    break;
                }
            }
            if (closed) break;
            regFilesCv.wait(lock);
        }
        openRegFiles[coord] = std::make_unique<regFile>(file);
        return useRegFile(coord);
    } else {
        std::lock_guard lock(regFilesMutex);
        openRegFiles[coord] = std::make_unique<regFile>(file);
        return useRegFile(coord);
    }
}

std::filesystem::path WorldRegions::getRegionFilename(int x, int z) const {
    return std::filesystem::path(std::to_string(x) + "_" + std::to_string(z) + ".bin");
}

void WorldRegions::writeRegion(int x, int z, int layer, WorldRegion* entry){
    std::filesystem::path filename = layers[layer].folder/getRegionFilename(x, z);

    glm::ivec3 regcoord(x, z, layer);
    if (auto regFile = getRegFile(regcoord, false)) {
        fetchChunks(entry, x, z, regFile.get());
        std::lock_guard lock(regFilesMutex);
        regFile.reset();
        closeRegFile(regcoord);
    }

    char header[REGION_HEADER_SIZE] = REGION_FORMAT_MAGIC;
    header[REGION_HEADER_SIZE - 2] = RegionConsts::FORMAT_VERSION;
    header[REGION_HEADER_SIZE - 1] = 0;
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file.write(header, REGION_HEADER_SIZE);

    size_t offset = REGION_HEADER_SIZE;
    char intbuf[4]{};
    uint offsets[RegionConsts::VOLUME]{};

    auto* region = entry->getChunks();
    uint32_t* sizes = entry->getSizes();

    for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
        ubyte* chunk = region[i].get();
        if (chunk == nullptr){
            offsets[i] = 0;
        } else {
            offsets[i] = offset;

            size_t compressedSize = sizes[i];
            dataio::write_int32_big(compressedSize, reinterpret_cast<ubyte*>(intbuf), 0);
            offset += 4 + compressedSize;

            file.write(intbuf, 4);
            file.write(reinterpret_cast<const char*>(chunk), compressedSize);
        }
    }
    for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
        dataio::write_int32_big(offsets[i], reinterpret_cast<ubyte*>(intbuf), 0);
        file.write(intbuf, 4);
    }
}

void WorldRegions::writeRegions(int layer) {
    for (auto& it : layers[layer].regions){
        WorldRegion* region = it.second.get();
        if (region->getChunks() == nullptr || !region->isUnsaved()) continue;
        glm::ivec2 key = it.first;
        writeRegion(key[0], key[1], layer, region);
    }
}

void WorldRegions::put(int x, int z, int layer, std::unique_ptr<ubyte[]> data, size_t size, bool rle) {
    if (rle) {
        size_t compressedSize;
        auto compressed = compress(data.get(), size, compressedSize);
        put(x, z, layer, std::move(compressed), compressedSize, false);
        return;
    }
    int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);

    WorldRegion* region = getOrCreateRegion(regionX, regionZ, layer);
    region->setUnsaved(true);
    region->put(localX, localZ, data.release(), size);
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

    put(chunk->chunk_x, chunk->chunk_z, RegionConsts::LAYER_VOXELS, chunk->encode(), CHUNK_DATA_LEN, true);

    if (doWriteLights && chunk->flags.lighted) {
        put(
            chunk->chunk_x, chunk->chunk_z,
            RegionConsts::LAYER_LIGHTS, 
            chunk->light_map.encode(),
            LIGHTMAP_DATA_LEN,
            true
        );
    }
    if (!chunk->inventories.empty()) {
        uint datasize;
        auto data = write_inventories(chunk, datasize);
        put(
            chunk->chunk_x, chunk->chunk_z,
            RegionConsts::LAYER_INVENTORIES,
            std::move(data),
            datasize,
            false
        );
    }
    if (!entitiesData.empty()) {
        auto data = std::make_unique<ubyte[]>(entitiesData.size());
        for (size_t i = 0; i < entitiesData.size(); ++i) {
            data[i] = entitiesData[i];
        }
        put(
            chunk->chunk_x, chunk->chunk_z,
            RegionConsts::LAYER_ENTITIES,
            std::move(data),
            entitiesData.size(),
            false
        );
    }
}

std::unique_ptr<ubyte[]> WorldRegions::getChunk(int x, int z){
    uint32_t size;
    auto* data = getData(x, z, RegionConsts::LAYER_VOXELS, size);
    if (data == nullptr) return nullptr;
    return decompress(data, size, CHUNK_DATA_LEN);
}

std::unique_ptr<light_t[]> WorldRegions::getLights(int x, int z) {
    uint32_t size;
    auto* bytes = getData(x, z, RegionConsts::LAYER_LIGHTS, size);
    if (bytes == nullptr) return nullptr;
    auto data = decompress(bytes, size, LIGHTMAP_DATA_LEN);
    return LightMap::decode(data.get());
}

chunk_inventories_map WorldRegions::fetchInventories(int x, int z) {
    chunk_inventories_map meta;
    uint32_t bytesSize;
    const ubyte* data = getData(x, z, RegionConsts::LAYER_INVENTORIES, bytesSize);
    if (data == nullptr) return meta;
    ByteReader reader(data, bytesSize);
    int count = reader.getInt32();
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
    uint32_t bytesSize;
    const ubyte* data = getData(x, z, RegionConsts::LAYER_ENTITIES, bytesSize);
    if (data == nullptr) return nullptr;
    auto map = json::from_binary(data, bytesSize);
    if (map->size() == 0) return nullptr;
    return map;
}

void WorldRegions::processRegionVoxels(int x, int z, const regionproc& func) {
    if (getRegion(x, z, RegionConsts::LAYER_VOXELS)) {
        LOG_ERROR("Not implemented for in-memory regions");
        throw std::runtime_error("Not implemented for in-memory regions");
    }
    auto regFile = getRegFile(glm::ivec3(x, z, RegionConsts::LAYER_VOXELS));
    if (regFile == nullptr) {
        LOG_ERROR("Could not open region file");
        throw std::runtime_error("Could not open region file");
    }
    for (uint cz = 0; cz < RegionConsts::SIZE; ++cz) {
        int gz = cz + z * RegionConsts::SIZE;
        for (uint cx = 0; cx < RegionConsts::SIZE; ++cx) {
            int gx = cx + x * RegionConsts::SIZE;
            uint32_t length;
            auto data = readChunkData(gx, gz, length, regFile.get());
            if (data == nullptr) continue;
            data = decompress(data.get(), length, CHUNK_DATA_LEN);
            if (func(data.get())) {
                put(gx, gz, RegionConsts::LAYER_VOXELS, std::move(data), CHUNK_DATA_LEN, true);
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
        writeRegions(layer.layer);
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
