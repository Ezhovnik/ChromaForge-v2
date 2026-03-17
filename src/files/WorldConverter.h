#ifndef FILES_WORLD_CONVERTER_H_
#define FILES_WORLD_CONVERTER_H_

#include <queue>
#include <filesystem>
#include <memory>

#include "../typedefs.h"
#include "../delegates.h"
#include "../interfaces/Task.h"

class Content;
class ContentLUT;
class WorldFiles;

enum class ConvertTaskType {
    Region, 
    Player
};

struct ConvertTask {
    ConvertTaskType type;
    std::filesystem::path file;
};

class WorldConverter : public Task {
private:
    std::unique_ptr<WorldFiles> wfile;
    std::shared_ptr<ContentLUT> const lut;
    const Content* const content;
    std::queue<ConvertTask> tasks;
    runnable onComplete;
    uint tasksDone = 0;

    void convertPlayer(std::filesystem::path file) const;
    void convertRegion(std::filesystem::path file) const;
public:
    WorldConverter(
        std::filesystem::path folder, 
        const Content* content, 
        std::shared_ptr<ContentLUT> lut
    );
    ~WorldConverter();

    void convert(ConvertTask task) const;
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
        std::filesystem::path folder, 
        const Content* content, 
        std::shared_ptr<ContentLUT> lut,
        runnable onDone,
        bool multithreading
    );
};

#endif // FILES_WORLD_CONVERTER_H_
