#pragma once

#include <queue>
#include <filesystem>
#include <memory>

#include <typedefs.h>
#include <delegates.h>
#include <interfaces/Task.h>
#include <files/world_regions_fwd.h>

class Content;
class ContentReport;
class WorldFiles;

enum class ConvertTaskType {
    Voxels,
    Inventories,
    Player,
    UpgradeRegion
};

struct ConvertTask {
    ConvertTaskType type;
    std::filesystem::path file;

    int x, z;
    RegionLayerIndex layer;
};

class WorldConverter : public Task {
private:
    std::shared_ptr<WorldFiles> wfile;
    std::shared_ptr<ContentReport> const report;
    const Content* const content;
    std::queue<ConvertTask> tasks;
    runnable onComplete;
    uint tasksDone = 0;
    bool upgradeMode;

    void upgradeRegion(
        const std::filesystem::path& file, int x, int z, RegionLayerIndex layer
    ) const;
    void convertPlayer(const std::filesystem::path& file) const;
    void convertVoxels(const std::filesystem::path& file, int x, int z) const;
    void convertInventories(const std::filesystem::path& file, int x, int z) const;

    void addRegionsTasks(
        RegionLayerIndex layerID,
        ConvertTaskType taskType
    );

    void createUpgradeTasks();
    void createConvertTasks();
public:
    WorldConverter(
        const std::shared_ptr<WorldFiles>& worldFiles, 
        const Content* content, 
        std::shared_ptr<ContentReport> report,
        bool upgradeMode
    );
    ~WorldConverter();

    void convert(const ConvertTask& task) const;
    void convertNext();
    void setOnComplete(runnable callback);
    void write();

    void update() override;
    void terminate() override;
    bool isActive() const override;
    void waitForEnd() override;
    uint getWorkTotal() const override;
    uint getWorkDone() const override;

    static std::shared_ptr<Task> startTask(
        const std::shared_ptr<WorldFiles>& worldFiles, 
        const Content* content, 
        const std::shared_ptr<ContentReport>& report,
        const runnable& onDone,
        bool upgradeMode,
        bool multithreading
    );
};
