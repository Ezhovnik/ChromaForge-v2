#include "WorldConverter.h"

#include <memory>
#include <stdexcept>

#include "WorldFiles.h"
#include "../voxels/Chunk.h"
#include "../content/ContentLUT.h"
#include "../debug/Logger.h"
#include "../data/dynamic.h"
#include "../files/files.h"
#include "../objects/Player.h"

WorldConverter::WorldConverter(std::filesystem::path folder, const Content* content, std::shared_ptr<ContentLUT> lut) : lut(lut), content(content) {
    DebugSettings settings;
    wfile = new WorldFiles(folder, settings);

    std::filesystem::path regionsFolder = wfile->getRegionsFolder();
    if (!std::filesystem::is_directory(regionsFolder)) {
        LOG_WARN("Nothing to convert");
        return;
    }

    tasks.push(ConvertTask{ConvertTaskType::Player, wfile->getPlayerFile()});

    for (auto file : std::filesystem::directory_iterator(regionsFolder)) {
        tasks.push(ConvertTask{ConvertTaskType::Region, file.path()});
    }
}

WorldConverter::~WorldConverter() {
    delete wfile;
}

bool WorldConverter::hasNext() const {
    return !tasks.empty();
}

void WorldConverter::convertRegion(std::filesystem::path file) {
    int x, z;
    std::string name = file.stem().string();
    if (!WorldFiles::parseRegionFilename(name, x, z)) {
        LOG_ERROR("Could not parse name '{}'", name);
        return;
    }

    LOG_INFO("Converting region '{}'", name);
    for (uint cz = 0; cz < RegionConsts::SIZE; ++cz) {
        for (uint cx = 0; cx < RegionConsts::SIZE; ++cx) {
            int gx = cx + x * RegionConsts::SIZE;
            int gz = cz + z * RegionConsts::SIZE;
            std::unique_ptr<ubyte[]> data (wfile->getChunk(gx, gz));
            if (data == nullptr) continue;

            if (wfile->getVoxelRegionVersion(x, z) != REGION_FORMAT_VERSION) Chunk::fromOld(data.get());
            if (lut) Chunk::convert(data.get(), lut.get());

            wfile->put(gx, gz, data.get());
        }
    }
    LOG_INFO("Region '{}' successfully converted", name);
}

void WorldConverter::convertPlayer(std::filesystem::path file) {
    LOG_INFO("Converting player {}", file.u8string());
    auto map = files::read_json(file);
    Player::convert(map.get(), lut.get());
    files::write_json(file, map.get());
    LOG_INFO("Player {} successfully converted", file.u8string());
}

void WorldConverter::convertNext() {
    if (!hasNext()) {
        LOG_ERROR("No more tasks to convert");
        throw std::runtime_error("No more tasks to convert");
    }
    ConvertTask task = tasks.front();
    tasks.pop();

    if (!std::filesystem::is_regular_file(task.file)) return;

    switch (task.type) {
        case ConvertTaskType::Region:
            convertRegion(task.file);
            break;
        case ConvertTaskType::Player:
            convertPlayer(task.file);
            break;
    }
}

void WorldConverter::write() {
    LOG_INFO("Writing world");
    wfile->write(nullptr, content);
    LOG_INFO("World successfully writed");
}

uint WorldConverter::getTotalTasks() const {
    return tasks.size();
}
