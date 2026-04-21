#include <files/WorldConverter.h>

#include <memory>
#include <stdexcept>
#include <utility>

#include <files/WorldFiles.h>
#include <voxels/Chunk.h>
#include <content/ContentLUT.h>
#include <debug/Logger.h>
#include <data/dynamic.h>
#include <files/files.h>
#include <objects/Player.h>
#include <util/ThreadPool.h>

class ConverterWorker : public util::Worker<ConvertTask, int> {
private:
    std::shared_ptr<WorldConverter> converter;
public:
    ConverterWorker(std::shared_ptr<WorldConverter> converter) : converter(std::move(converter)) {}

    int operator()(const std::shared_ptr<ConvertTask>& task) override {
        converter->convert(*task);
        return 0;
    }
};

WorldConverter::WorldConverter(
    const std::filesystem::path& folder, 
    const Content* content, 
    std::shared_ptr<ContentLUT> lut
) : wfile(std::make_unique<WorldFiles>(folder)),
    lut(std::move(lut)),
    content(content) 
{
    std::filesystem::path regionsFolder = wfile->getRegions().getRegionsFolder(RegionConsts::LAYER_VOXELS);
    if (!std::filesystem::is_directory(regionsFolder)) {
        LOG_WARN("Nothing to convert");
        return;
    }

    tasks.push(ConvertTask{ConvertTaskType::Player, wfile->getPlayerFile()});

    for (const auto& file : std::filesystem::directory_iterator(regionsFolder)) {
        tasks.push(ConvertTask{ConvertTaskType::Region, file.path()});
    }
}

WorldConverter::~WorldConverter() {
}

std::shared_ptr<Task> WorldConverter::startTask(
    const std::filesystem::path& folder, 
    const Content* content, 
    const std::shared_ptr<ContentLUT>& lut,
    const runnable& onDone,
    bool multithreading
) {
    auto converter = std::make_shared<WorldConverter>(folder, content, lut);
    if (!multithreading) {
        converter->setOnComplete([=]() {
            converter->write();
            onDone();
        });
        return converter;
    }
    auto pool = std::make_shared<util::ThreadPool<ConvertTask, int>>(
        "converter-pool",
        [=](){return std::make_shared<ConverterWorker>(converter);},
        [=](int&) {}
    );
    auto& converterTasks = converter->tasks;
    while (!converterTasks.empty()) {
        const ConvertTask& task = converterTasks.front();
        auto ptr = std::make_shared<ConvertTask>(task);
        pool->enqueueJob(ptr);
        converterTasks.pop();
    }
    pool->setOnComplete([=]() {
        converter->write();
        onDone();
    });
    return pool;
}

void WorldConverter::convertRegion(const std::filesystem::path& file) const {
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

void WorldConverter::convertPlayer(const std::filesystem::path& file) const {
    LOG_INFO("Converting player {}", file.u8string());
    auto map = files::read_json(file);
    Player::convert(map.get(), lut.get());
    files::write_json(file, map.get());
    LOG_INFO("Player {} successfully converted", file.u8string());
}

void WorldConverter::convert(const ConvertTask& task) const {
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

void WorldConverter::convertNext() {
    if (tasks.empty()) {
        LOG_ERROR("No more regions to convert");
        throw std::runtime_error("no more regions to convert");
    }
    ConvertTask task = tasks.front();
    tasks.pop();
    tasksDone++;

    convert(task);
}

void WorldConverter::setOnComplete(runnable callback) {
    this->onComplete = std::move(callback);
}

void WorldConverter::update() {
    convertNext();
    if (onComplete && tasks.empty()) onComplete();
}

void WorldConverter::terminate() {
    tasks = {};
}

bool WorldConverter::isActive() const {
    return !tasks.empty();
}

void WorldConverter::write() {
    LOG_INFO("Writing world");
    wfile->write(nullptr, content);
    LOG_INFO("World successfully writed");
}

uint WorldConverter::getWorkTotal() const {
    return tasks.size() + tasksDone;
}

uint WorldConverter::getWorkDone() const {
    return tasksDone;
}

void WorldConverter::waitForEnd() {
    while (isActive()) {
        update();
    }
}
