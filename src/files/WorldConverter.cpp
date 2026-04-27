#include <files/WorldConverter.h>

#include <memory>
#include <stdexcept>
#include <utility>

#include <files/WorldFiles.h>
#include <voxels/Chunk.h>
#include <content/ContentReport.h>
#include <debug/Logger.h>
#include <data/dynamic.h>
#include <files/files.h>
#include <objects/Player.h>
#include <util/ThreadPool.h>
#include <items/Inventory.h>
#include <files/compatibility.h>

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

void WorldConverter::addRegionsTasks(
    RegionLayerIndex layerID,
    ConvertTaskType taskType
) {
    const auto& regions = wfile->getRegions();
    auto regionsFolder = regions.getRegionsFolder(layerID);
    if (!std::filesystem::is_directory(regionsFolder)) {
        return;
    }
    for (const auto& file : std::filesystem::directory_iterator(regionsFolder)) {
        int x, z;
        std::string name = file.path().stem().string();
        if (!WorldRegions::parseRegionFilename(name, x, z)) {
            LOG_ERROR("Could not parse region name {}", name);
            continue;
        }
        tasks.push(ConvertTask {taskType, file.path(), x, z, layerID});
    }
}

void WorldConverter::createUpgradeTasks() {
    const auto& regions = wfile->getRegions();
    for (auto& issue : report->getIssues()) {
        if (issue.issueType != ContentIssueType::RegionFormatUpdate) {
            continue;
        }
        addRegionsTasks(issue.regionLayer, ConvertTaskType::UpgradeRegion);
    }
}

void WorldConverter::createConvertTasks() {
    auto handleReorder = [=](ContentType contentType) {
        switch (contentType) {
            case ContentType::Block:
                addRegionsTasks(
                    REGION_LAYER_VOXELS,
                    ConvertTaskType::Voxels
                );
                break;
            case ContentType::Item:
                addRegionsTasks(
                    REGION_LAYER_INVENTORIES,
                    ConvertTaskType::Inventories
                );
                break;
            default:
                break;
        }
    };

    const auto& regions = wfile->getRegions();
    for (auto& issue : report->getIssues()) {
        switch (issue.issueType) {
            case ContentIssueType::RegionFormatUpdate:
                break;
            case ContentIssueType::Missing:
                LOG_ERROR("Issue can't be resolved");
                throw std::runtime_error("Issue can't be resolved");
            case ContentIssueType::Reorder:
                handleReorder(issue.contentType);
                break;
        }
    }

    tasks.push(ConvertTask {ConvertTaskType::Player, wfile->getPlayerFile()});
}

WorldConverter::WorldConverter(
    const std::shared_ptr<WorldFiles>& worldFiles, 
    const Content* content, 
    std::shared_ptr<ContentReport> reportPtr,
    bool upgradeMode
) : wfile(worldFiles),
    report(std::move(reportPtr)),
    content(content),
    upgradeMode(upgradeMode)
{
    if (upgradeMode) {
        createUpgradeTasks();
    } else {
        createConvertTasks();
    }
}

WorldConverter::~WorldConverter() {
}

std::shared_ptr<Task> WorldConverter::startTask(
    const std::shared_ptr<WorldFiles>& worldFiles, 
    const Content* content, 
    const std::shared_ptr<ContentReport>& report,
    const runnable& onDone,
    bool upgradeMode,
    bool multithreading
) {
    auto converter = std::make_shared<WorldConverter>(
        worldFiles, content, report, upgradeMode
    );
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

void WorldConverter::upgradeRegion(
    const std::filesystem::path& file, int x, int z, RegionLayerIndex layer
) const {
    auto path = wfile->getRegions().getRegionFilePath(layer, x, z);
    auto bytes = files::read_bytes_buffer(path);
    auto buffer = compatibility::convert_region_2to3(bytes, layer);
    files::write_bytes(path, buffer.data(), buffer.size());
}

void WorldConverter::convertVoxels(const std::filesystem::path& file, int x, int z) const {
    LOG_INFO("Converting voxels region {} {}", x, z);
    wfile->getRegions().processRegion(x, z, REGION_LAYER_VOXELS,
    [=](std::unique_ptr<ubyte[]> data, uint32_t*) {
        Chunk::convert(data.get(), report.get());
        return data;
    });
}

void WorldConverter::convertInventories(const std::filesystem::path& file, int x, int z) const {
    LOG_INFO("Converting inventories region {} {}", x, z);
    wfile->getRegions().processInventories(x, z, [=](Inventory* inventory) {
        inventory->convert(report.get());
    });
}

void WorldConverter::convertPlayer(const std::filesystem::path& file) const {
    LOG_INFO("Converting player {}", file.u8string());
    auto map = files::read_json(file);
    Player::convert(map.get(), report.get());
    files::write_json(file, map.get());
    LOG_INFO("Player {} successfully converted", file.u8string());
}

void WorldConverter::convert(const ConvertTask& task) const {
    if (!std::filesystem::is_regular_file(task.file)) return;

    switch (task.type) {
        case ConvertTaskType::UpgradeRegion:
            upgradeRegion(task.file, task.x, task.z, task.layer);
            break;
        case ConvertTaskType::Voxels:
            convertVoxels(task.file, task.x, task.z);
            break;
        case ConvertTaskType::Inventories:
            convertInventories(task.file, task.x, task.z);
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
    if (upgradeMode) {
        LOG_INFO("Refreshing version");
        wfile->patchIndicesVersion("region-version", REGION_FORMAT_VERSION);
    } else {
        LOG_INFO("Writing world");
        wfile->write(nullptr, content);
        LOG_INFO("World successfully writed");
    }
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
