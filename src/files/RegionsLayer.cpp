#include <files/WorldRegions.h>

#include <cstring>

#include <util/data_io.h>
#include <debug/Logger.h>

#define REGION_FORMAT_MAGIC ".CHROMAREG"
inline constexpr int REGION_HEADER_SIZE = 13;

static std::filesystem::path get_region_filename(int x, int z) {
    return std::filesystem::path(std::to_string(x) + "_" + std::to_string(z) + ".bin");
}

static void fetch_chunks(WorldRegion* region, int x, int z, regFile* file) {
    auto* chunks = region->getChunks();
    uint32_t* sizes = region->getSizes();

    for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
        int chunk_x = (i % RegionConsts::SIZE) + x * RegionConsts::SIZE;
        int chunk_z = (i / RegionConsts::SIZE) + z * RegionConsts::SIZE;
        if (chunks[i] == nullptr) {
            chunks[i] = RegionsLayer::readChunkData(chunk_x, chunk_z, sizes[i], file);
        }
    }
}

regFile::regFile(std::filesystem::path filename) : file(std::move(filename)) {
    if (file.length() < REGION_HEADER_SIZE) {
        LOG_ERROR("Incomplete region file header");
        throw std::runtime_error("Incomplete region file header");
    }
    char header[REGION_HEADER_SIZE];
    file.read(header, REGION_HEADER_SIZE);

    if (std::string(header, std::strlen(REGION_FORMAT_MAGIC)) != REGION_FORMAT_MAGIC) {
        LOG_ERROR("Invalid region file magic number");
        throw std::runtime_error("Invalid region file magic number");
    }
    version = header[REGION_HEADER_SIZE - 2];
    if (static_cast<uint>(version) > REGION_FORMAT_VERSION) {
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
    if (offset == 0) {
        return nullptr;
    }

    file.seekg(offset);
    file.read(reinterpret_cast<char*>(&offset), 4);
    length = dataio::read_int32_big(reinterpret_cast<const ubyte*>(&offset), 0);
    auto data = std::make_unique<ubyte[]>(length);
    file.read(reinterpret_cast<char*>(data.get()), length);
    return data;
}


void RegionsLayer::closeRegFile(glm::ivec2 coord) {
    openRegFiles.erase(coord);
    regFilesCv.notify_one();
}

regFile_ptr RegionsLayer::useRegFile(glm::ivec2 coord) {
    auto* file = openRegFiles[coord].get();
    file->inUse = true;
    return regFile_ptr(file, &regFilesCv);
}

regFile_ptr RegionsLayer::getRegFile(glm::ivec2 coord, bool create) {
    {
        std::lock_guard lock(regFilesMutex);
        const auto found = openRegFiles.find(coord);
        if (found != openRegFiles.end()) {
            if (found->second->inUse) {
                LOG_ERROR("regFile is currently in use");
                throw std::runtime_error("regFile is currently in use");
            }
            return useRegFile(found->first);
        }
    }
    if (create) {
        return createRegFile(coord);
    }
    return nullptr;
}

regFile_ptr RegionsLayer::createRegFile(glm::ivec2 coord) {
    auto file =  folder / get_region_filename(coord[0], coord[1]);
    if (!std::filesystem::exists(file)) {
        return nullptr;
    }
    if (openRegFiles.size() == MAX_OPEN_REGION_FILES) {
        std::unique_lock lock(regFilesMutex);
        while (true) {
            bool closed = false;
            // FIXME: bad choosing algorithm
            for (auto& entry : openRegFiles) {
                if (!entry.second->inUse) {
                    closeRegFile(entry.first);
                    closed = true;
                    break;
                }
            }
            if (closed) {
                break;
            }
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

WorldRegion* RegionsLayer::getRegion(int x, int z) {
    std::lock_guard lock(mapMutex);
    auto found = regions.find({x, z});
    if (found == regions.end()) {
        return nullptr;
    }
    return found->second.get();
}

WorldRegion* RegionsLayer::getOrCreateRegion(int x, int z) {
    if (auto region = getRegion(x, z)) {
        return region;
    }
    std::lock_guard lock(mapMutex);
    auto region_ptr = std::make_unique<WorldRegion>();
    auto region = region_ptr.get();
    regions[{x, z}] = std::move(region_ptr);
    return region;
}

ubyte* RegionsLayer::getData(int x, int z, uint32_t& size) {
    int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);

    WorldRegion* region = getOrCreateRegion(regionX, regionZ);
    ubyte* data = region->getChunkData(localX, localZ);
    if (data == nullptr) {
        auto regFile = getRegFile({regionX, regionZ});
        if (regFile != nullptr) {
            auto dataptr = readChunkData(x, z, size, regFile.get());
            if (dataptr) {
                data = dataptr.get();
                region->put(localX, localZ, std::move(dataptr), size);
            }
        }
    }
    if (data != nullptr) {
        size = region->getChunkDataSize(localX, localZ);
        return data;
    }
    return nullptr;
}

void RegionsLayer::writeRegion(int x, int z, WorldRegion* entry) {
    std::filesystem::path filename = folder / get_region_filename(x, z);

    glm::ivec2 regcoord(x, z);
    if (auto regFile = getRegFile(regcoord, false)) {
        fetch_chunks(entry, x, z, regFile.get());

        std::lock_guard lock(regFilesMutex);
        regFile.reset();
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

    auto* region = entry->getChunks();
    uint32_t* sizes = entry->getSizes();

    for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
        ubyte* chunk = region[i].get();
        if (chunk == nullptr) {
            offsets[i] = 0;
        } else {
            offsets[i] = offset;

            size_t compressedSize = sizes[i];
            dataio::write_int32_big(
                compressedSize, reinterpret_cast<ubyte*>(intbuf), 0
            );
            offset += 4 + compressedSize;

            file.write(intbuf, 4);
            file.write(reinterpret_cast<const char*>(chunk), compressedSize);
        }
    }
    for (size_t i = 0; i < RegionConsts::VOLUME; ++i) {
        dataio::write_int32_big(
            offsets[i], reinterpret_cast<ubyte*>(intbuf), 0
        );
        file.write(intbuf, 4);
    }
}

std::unique_ptr<ubyte[]> RegionsLayer::readChunkData(
    int x, int z, uint32_t& length, regFile* rfile
) {
    int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);
    int chunkIndex = localZ * RegionConsts::SIZE + localX;
    return rfile->read(chunkIndex, length);
}
