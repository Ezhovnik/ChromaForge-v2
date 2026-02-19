#include "WorldConverter.h"

#include <memory>
#include <stdexcept>

#include "WorldFiles.h"
#include "../voxels/Chunk.h"
#include "../content/ContentLUT.h"
#include "../logger/Logger.h"

WorldConverter::WorldConverter(std::filesystem::path folder, const Content* content, const ContentLUT* lut) : lut(lut), content(content) {
    DebugSettings settings;
    wfile = new WorldFiles(folder, settings);

    std::filesystem::path regionsFolder = wfile->getRegionsFolder();
    if (!std::filesystem::is_directory(regionsFolder)) {
        LOG_WARN("Nothing to convert");
        return;
    }
    for (auto file : std::filesystem::directory_iterator(regionsFolder)) {
        regions.push(file.path());
    }
}

WorldConverter::~WorldConverter() {
    delete wfile;
}

bool WorldConverter::hasNext() const {
    return !regions.empty();
}

void WorldConverter::convertNext() {
    if (!hasNext()) {
        LOG_ERROR("No more regions to convert");
        throw std::runtime_error("No more regions to convert");
    }
    std::filesystem::path regfile = regions.front();
    regions.pop();
    if (!std::filesystem::is_regular_file(regfile)) return;
    int x, y;
    std::string name = regfile.stem().string();
    if (!WorldFiles::parseRegionFilename(name, x, y)) {
        LOG_ERROR("Could not parse name {}", name);
        return;
    }
    LOG_INFO("Converting region {}", name);
    for (uint cz = 0; cz < RegionConsts::SIZE; ++cz) {
        for (uint cx = 0; cx < RegionConsts::SIZE; ++cx) {
            int gx = cx + x * RegionConsts::SIZE;
            int gz = cz + y * RegionConsts::SIZE;
            std::unique_ptr<ubyte[]> data (wfile->getChunk(gx, gz));
            if (data == nullptr) continue;
            Chunk::convert(data.get(), lut);
            wfile->put(gx, gz, data.get());
        }
    }
    LOG_INFO("Region {} successfully converted", name);
}

void WorldConverter::write() {
    LOG_INFO("Writing world");
    wfile->write(nullptr, content);
    LOG_INFO("World successfully writed");
}
