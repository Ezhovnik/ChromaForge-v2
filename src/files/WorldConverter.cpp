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

WorldConverter::WorldConverter(
    std::filesystem::path folder, 
    const Content* content, 
    std::shared_ptr<ContentLUT> lut
) : wfile(std::make_unique<WorldFiles>(folder, DebugSettings {})),
    lut(lut), 
    content(content) 
{
    std::filesystem::path regionsFolder = wfile->getRegions().getRegionsFolder(RegionConsts::LAYER_VOXELS);
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
}

void WorldConverter::convertRegion(std::filesystem::path file) {
    int x, z;
    std::string name = file.stem().string();
    if (!WorldRegions::parseRegionFilename(name, x, z)) {
        LOG_ERROR("Could not parse name '{}'", name);
        return;
    }

    LOG_INFO("Converting region '{}'", name);
    wfile->getRegions().processRegionVoxels(x, z, [=](ubyte* data) {
        if (lut) {
            Chunk::convert(data, lut.get());
        }
        return true;
    });
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
    if (tasks.empty()) {
        LOG_ERROR("No more tasks to convert");
        throw std::runtime_error("No more tasks to convert");
    }
    ConvertTask task = tasks.front();
    tasks.pop();
    tasksDone++;

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

uint WorldConverter::getWorkRemaining() const {
    return tasks.size();
}

uint WorldConverter::getWorkDone() const {
    return tasksDone;
}

void WorldConverter::waitForEnd() {
    while (isActive()) {
        update();
    }
}
