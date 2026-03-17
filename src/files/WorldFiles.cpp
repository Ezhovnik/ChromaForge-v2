#include "WorldFiles.h"

#include <cassert>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cstring>

#include "../coders/byte_utils.h"
#include "../coders/json.h"
#include "../constants.h"
#include "../content/Content.h"
#include "../core_content_defs.h"
#include "../data/dynamic.h"
#include "../items/Inventory.h"
#include "../items/Item.h"
#include "../lighting/Lightmap.h"
#include "../math/voxmaths.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../typedefs.h"
#include "../util/data_io.h"
#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../window/Camera.h"
#include "../world/World.h"
#include "../coders/rle.h"
#include "../debug/Logger.h"

#define REGION_FORMAT_MAGIC ".CHROMAREG"
inline constexpr int REGION_HEADER_SIZE = 13;

inline constexpr size_t BUFFER_SIZE_UNKNOWN = -1;

inline void calc_reg_coords(int x, int z, int& regionX, int& regionZ, int& localX, int& localZ) {
    regionX = floordiv(x, RegionConsts::SIZE);
    regionZ = floordiv(z, RegionConsts::SIZE);
    localX = x - (regionX * RegionConsts::SIZE);
    localZ = z - (regionZ * RegionConsts::SIZE);
}

std::unique_ptr<ubyte[]> write_inventories(Chunk* chunk, uint& datasize) {
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
    for (uint i = 0; i < datasize; ++i) {
        data[i] = datavec[i];
    }
    return data;
}

WorldRegion::WorldRegion() {
	chunksData = new ubyte*[RegionConsts::VOLUME]{};
	sizes = new uint32_t[RegionConsts::VOLUME]{};
}

WorldRegion::~WorldRegion() {
	for (uint i = 0; i < RegionConsts::VOLUME; ++i) {
		delete[] chunksData[i];
	}
	delete[] sizes;
	delete[] chunksData;
}

void WorldRegion::put(uint x, uint z, ubyte* data, uint32_t size) {
	size_t chunk_index = z * RegionConsts::SIZE + x;
	delete[] chunksData[chunk_index];
	chunksData[chunk_index] = data;
	sizes[chunk_index] = size;
}

ubyte* WorldRegion::getChunkData(uint x, uint z) {
	return chunksData[z * RegionConsts::SIZE + x];
}

uint WorldRegion::getChunkDataSize(uint x, uint z) {
	return sizes[z * RegionConsts::SIZE + x];
}

regFile::regFile(std::filesystem::path filename) : file(filename) {
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
    if (uint(version) > REGION_FORMAT_VERSION) {
		LOG_ERROR("Region format {} is not supported", version);
        throw illegal_region_format("Region format " + std::to_string(version) + " is not supported");
    }
}

WorldFiles::WorldFiles(std::filesystem::path directory) : directory(directory) {
	for (uint i = 0; i < sizeof(layers)/sizeof(RegionsLayer); ++i) {
        layers[i].layer = i;
    }
    layers[RegionConsts::LAYER_VOXELS].folder = directory/std::filesystem::path("regions");
    layers[RegionConsts::LAYER_LIGHTS].folder = directory/std::filesystem::path("lights");
    layers[RegionConsts::LAYER_INVENTORIES].folder = directory/std::filesystem::path("inventories");
}

WorldFiles::WorldFiles(std::filesystem::path directory, const DebugSettings& settings) : WorldFiles(directory) {
    generatorTestMode = settings.generatorTestMode;
    doWriteLights = settings.doWriteLights;
}

WorldFiles::~WorldFiles(){
}

void WorldFiles::createDirectories() {
    std::filesystem::create_directories(directory/std::filesystem::path("data"));
    std::filesystem::create_directories(directory/std::filesystem::path("content"));
}

WorldRegion* WorldFiles::getRegion(int x, int z, int layer) {
    RegionsLayer& regions = layers[layer];
    std::lock_guard lock(regions.mutex);
    auto found = regions.regions.find(glm::ivec2(x, z));
    if (found == regions.regions.end()) return nullptr;
    return found->second.get();
}

WorldRegion* WorldFiles::getOrCreateRegion(int x, int z, int layer) {
    RegionsLayer& regions = layers[layer];
    WorldRegion* region = getRegion(x, z, layer);
	if (region == nullptr) {
		std::lock_guard lock(regions.mutex);
		region = new WorldRegion();
		regions.regions[glm::ivec2(x, z)].reset(region);
	}
	return region;
}

std::unique_ptr<ubyte[]> WorldFiles::compress(const ubyte* src, size_t srclen, size_t& len) {
	auto buffer = bufferPool.get();
    ubyte* bytes = buffer.get();
	len = extrle::encode(src, srclen, bytes);
	auto data = std::make_unique<ubyte[]>(len);
	for (size_t i = 0; i < len; ++i) {
		data[i] = bytes[i];
	}
	return data;
}

std::unique_ptr<ubyte[]> WorldFiles::decompress(const ubyte* src, size_t srclen, size_t dstlen) {
    auto decompressed = std::make_unique<ubyte[]>(dstlen);
    extrle::decode(src, srclen, decompressed.get());
	return decompressed;
}

void WorldFiles::put(Chunk* chunk) {
	assert(chunk != nullptr);

	int regionX, regionZ, localX, localZ;
    calc_reg_coords(chunk->chunk_x, chunk->chunk_z, regionX, regionZ, localX, localZ);

	put(chunk->chunk_x, chunk->chunk_z, RegionConsts::LAYER_VOXELS, chunk->encode(), CHUNK_DATA_LEN, true);

	if (doWriteLights && chunk->isLighted()) {
		put(chunk->chunk_x, chunk->chunk_z, RegionConsts::LAYER_LIGHTS, chunk->light_map.encode(), LIGHTMAP_DATA_LEN, true);
	}

	if (!chunk->inventories.empty()){
        uint datasize;
        put(chunk->chunk_x, chunk->chunk_z, RegionConsts::LAYER_INVENTORIES, write_inventories(chunk, datasize), datasize, false);
    }
}

void WorldFiles::put(int x, int z, int layer, std::unique_ptr<ubyte[]> data, size_t size, bool rle) {
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

std::filesystem::path WorldFiles::getRegionFilename(int x, int y) const {
	std::string filename = std::to_string(x) + "_" + std::to_string(y) + ".bin";
	return std::filesystem::path(filename);
}

bool WorldFiles::parseRegionFilename(const std::string& name, int& x, int& z) {
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

std::filesystem::path WorldFiles::getPlayerFile() const {
	return directory/std::filesystem::path("player.json");
}

std::filesystem::path WorldFiles::getWorldFile() const {
	return directory/std::filesystem::path(WORLD_FILE);
}

std::filesystem::path WorldFiles::getIndicesFile() const {
	return directory/std::filesystem::path("indices.json");
}

std::filesystem::path WorldFiles::getPacksFile() const {
	return directory/std::filesystem::path("packs.list");
}

std::unique_ptr<ubyte[]> WorldFiles::getChunk(int x, int z) {
    uint32_t size;
    auto* data = getData(x, z, RegionConsts::LAYER_VOXELS, size);
    if (data == nullptr) return nullptr;
    return decompress(data, size, CHUNK_DATA_LEN);
}

std::unique_ptr<light_t[]> WorldFiles::getLights(int x, int z) {
    uint32_t size;
    auto* bytes = getData(x, z, RegionConsts::LAYER_LIGHTS, size);
    if (bytes == nullptr) return nullptr;
	auto data = decompress(bytes, size, LIGHTMAP_DATA_LEN);
	return LightMap::decode(data.get());
}

chunk_inventories_map WorldFiles::fetchInventories(int x, int z) {
	chunk_inventories_map inventories;
	uint32_t bytesSize;
    const ubyte* data = getData(x, z, RegionConsts::LAYER_INVENTORIES, bytesSize);
	if (data == nullptr) return inventories;
	ByteReader reader(data, bytesSize);
	int count = reader.getInt32();
	for (int i = 0; i < count; i++) {
		uint index = reader.getInt32();
		uint size = reader.getInt32();
		auto map = json::from_binary(reader.pointer(), size);
		reader.skip(size);
		auto inv = std::make_shared<Inventory>(0, 0);
		inv->deserialize(map.get());
		inventories[index] = inv;
	}
	return inventories;
}

ubyte* WorldFiles::getData(
	int x, 
	int z, 
	int layer, 
	uint32_t& size)
{
	int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);

	WorldRegion* region = getOrCreateRegion(regionX, regionZ, layer);

	ubyte* data = region->getChunkData(localX, localZ);
	if (data == nullptr) {
		auto regfile = getRegFile(glm::ivec3(regionX, regionZ, layer));
        if (regfile != nullptr) data = readChunkData(x, z, size, regfile.get()).release();
		if (data != nullptr) region->put(localX, localZ, data, size);
	}

	if (data != nullptr) {
		size = region->getChunkDataSize(localX, localZ);
		return data;
	}

	return nullptr;
}

std::shared_ptr<regFile> WorldFiles::useRegFile(glm::ivec3 coord) {
    return std::shared_ptr<regFile>(openRegFiles[coord].get(), [this](regFile* ptr) {
        ptr->inUse = false;
        regFilesCv.notify_one();
    });
}

void WorldFiles::closeRegFile(glm::ivec3 coord) {
    openRegFiles.erase(coord);
    regFilesCv.notify_one();
}

std::shared_ptr<regFile> WorldFiles::getRegFile(glm::ivec3 coord) {
    {
        std::lock_guard lock(regFilesMutex);
        const auto found = openRegFiles.find(coord);
        if (found != openRegFiles.end()) {
            if (found->second->inUse) {
				LOG_ERROR("Regfile is currently in use");
                throw std::runtime_error("Regfile is currently in use");
            }
            found->second->inUse = true;
            return useRegFile(found->first);
        }
    }
    return createRegFile(coord);
}

std::shared_ptr<regFile> WorldFiles::createRegFile(glm::ivec3 coord) {
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

std::unique_ptr<ubyte[]> WorldFiles::readChunkData(
	int x, 
	int z, 
	uint32_t& length, 
	regFile* rfile)
{
	if (generatorTestMode) return nullptr;

	int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);

	int chunkIndex = localZ * RegionConsts::SIZE + localX;

	files::rafile& file = rfile->file;

	size_t file_size = file.length();
	size_t table_offset = file_size - RegionConsts::VOLUME * 4;

	uint32_t offset;
	file.seekg(table_offset + chunkIndex * 4);
	file.read((char*)(&offset), 4);
	offset = dataio::read_int32_big((const ubyte*)(&offset), 0);
	if (offset == 0) return nullptr;
	file.seekg(offset);
	file.read((char*)(&offset), 4);
	length = dataio::read_int32_big((const ubyte*)(&offset), 0);

	auto data = std::make_unique<ubyte[]>(length);
    file.read((char*)data.get(), length);
	return data;
}

void WorldFiles::fetchChunks(WorldRegion* region, int x, int y, regFile* rfile) {
    ubyte** chunks = region->getChunks();
	uint32_t* sizes = region->getSizes();

    for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
		int chunk_x = (i % RegionConsts::SIZE) + x * RegionConsts::SIZE;
		int chunk_z = (i / RegionConsts::SIZE) + y * RegionConsts::SIZE;
		if (chunks[i] == nullptr) chunks[i] = readChunkData(chunk_x, chunk_z, sizes[i], rfile).release();
	}
}

void WorldFiles::writeRegion(int x, int z, int layer, WorldRegion* entry){
    std::filesystem::path filename = layers[layer].folder/getRegionFilename(x, z);

	glm::ivec3 regcoord(x, z, layer);
    if (auto regfile = getRegFile(regcoord)) {
        fetchChunks(entry, x, z, regfile.get());

        std::lock_guard lock(regFilesMutex);
        closeRegFile(regcoord);
    }

	char header[REGION_HEADER_SIZE] = REGION_FORMAT_MAGIC;
    header[REGION_HEADER_SIZE - 2] = REGION_FORMAT_VERSION;
    header[REGION_HEADER_SIZE - 1] = 0;

	std::ofstream file(filename, std::ios::out | std::ios::binary);
	file.write(header, REGION_HEADER_SIZE);

	size_t offset = REGION_HEADER_SIZE;
	char intbuf[4]{};
	uint offsets[RegionConsts::VOLUME]{};

	ubyte** region = entry->getChunks();
	uint32_t* sizes = entry->getSizes();

	for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
		ubyte* chunk = region[i];
		if (chunk == nullptr) {
			offsets[i] = 0;
		} else {
			offsets[i] = offset;

			size_t compressedSize = sizes[i];
			dataio::write_int32_big(compressedSize, (ubyte*)intbuf, 0);
			offset += 4 + compressedSize;

			file.write(intbuf, 4);
			file.write((const char*)chunk, compressedSize);
		}
	}
	for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
		dataio::write_int32_big(offsets[i], (ubyte*)intbuf, 0);
		file.write(intbuf, 4);
	}
}

void WorldFiles::writeRegions(int layer) {
    for (auto& it : layers[layer].regions) {
		WorldRegion* region = it.second.get();
		if (region->getChunks() == nullptr || !region->isUnsaved()) continue;
		glm::ivec2 key = it.first;
		writeRegion(key.x, key.y, layer, region);
	}
}

void WorldFiles::write(const World* world, const Content* content) {
	for (auto& layer : layers) {
        std::filesystem::create_directories(layer.folder);
    }

	if (world) {
		writeWorldInfo(world);
		if (!std::filesystem::exists(getPacksFile())) writePacks(world->getPacks());
	}

	if (generatorTestMode) return;

	writeIndices(content->getIndices());
	for (auto& layer : layers) {
        writeRegions(layer.layer);
    }
}

void WorldFiles::writePacks(const std::vector<ContentPack>& packs) {
	auto packsFile = getPacksFile();

	std::stringstream ss;
	ss << "# autogenerated; do not modify\n";
	for (const auto& pack : packs) {
		ss << pack.id << "\n";
	}
	files::write_string(packsFile, ss.str());
}

void WorldFiles::writeIndices(const ContentIndices* indices) {
	dynamic::Map root;
	uint count;

	auto& blocks = root.putList("blocks");
	count = indices->countBlockDefs();
	for (uint i = 0; i < count; ++i) {
		const Block* def = indices->getBlockDef(i);
		blocks.put(def->name);
	}

	auto& items = root.putList("items");
	count = indices->countItemDefs();
	for (uint i = 0; i < count; i++) {
		const Item* def = indices->getItemDef(i);
		items.put(def->name);
	}

	files::write_json(getIndicesFile(), &root);
}

void WorldFiles::writeWorldInfo(const World* world) {
	files::write_json(getWorldFile(), world->serialize().get());
}

bool WorldFiles::readWorldInfo(World* world) {
	std::filesystem::path file = getWorldFile();
	if (!std::filesystem::is_regular_file(file)) {
		LOG_WARN("World.json does not exists");
		return false;
	}

	auto root = files::read_json(file);
	world->deserialize(root.get());

	return true;
}

static void erase_pack_indices(dynamic::Map* root, const std::string& id) {
	auto prefix = id + ":";
    auto blocks = root->list("blocks");
    for (uint i = 0; i < blocks->size(); ++i) {
        auto name = blocks->str(i);
        if (name.find(prefix) != 0) continue;
        auto value = blocks->getValueWriteable(i);
        value->value = BUILTIN_AIR;
    }

    auto items = root->list("items");
    for (uint i = 0; i < items->size(); ++i) {
        auto name = items->str(i);
        if (name.find(prefix) != 0) continue;
        auto value = items->getValueWriteable(i);
        value->value = BUILTIN_EMPTY;
    }
}

void WorldFiles::removeIndices(const std::vector<std::string>& packs) {
    auto root = files::read_json(getIndicesFile());
    for (const auto& id : packs) {
        erase_pack_indices(root.get(), id);
    }
    files::write_json(getIndicesFile(), root.get());
}

void WorldFiles::processRegionVoxels(int x, int z, regionproc func) {
    if (getRegion(x, z, RegionConsts::LAYER_VOXELS)) {
		LOG_ERROR("Not implemented for in-memory regions");
        throw std::runtime_error("Not implemented for in-memory regions");
    }
    auto regfile = getRegFile(glm::ivec3(x, z, RegionConsts::LAYER_VOXELS));
    if (regfile == nullptr) {
		LOG_ERROR("Could not open region file");
        throw std::runtime_error("Could not open region file");
    }
    for (uint cz = 0; cz < RegionConsts::SIZE; ++cz) {
        for (uint cx = 0; cx < RegionConsts::SIZE; ++cx) {
            int gx = cx + x * RegionConsts::SIZE;
            int gz = cz + z * RegionConsts::SIZE;
            uint32_t length;
            auto data = readChunkData(gx, gz, length, regfile.get());
            if (data == nullptr) continue;
			data = decompress(data.get(), length, CHUNK_DATA_LEN);
            if (func(data.get())) {
                put(gx, gz, RegionConsts::LAYER_VOXELS, std::move(data), CHUNK_DATA_LEN, true);
            }
        }
    }
}

std::filesystem::path WorldFiles::getFolder() const {
    return directory;
}

std::filesystem::path WorldFiles::getRegionsFolder(int layer) const {
    return layers[layer].folder;
}
