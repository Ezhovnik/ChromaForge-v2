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
    WorldFiles* wfile;
    std::shared_ptr<ContentLUT> const lut;
    const Content* const content;
    std::queue<ConvertTask> tasks;
    runnable onComplete;
    uint tasksDone = 0;

    void convertPlayer(std::filesystem::path file);
    void convertRegion(std::filesystem::path file);
public:
    WorldConverter(std::filesystem::path folder, const Content* content, std::shared_ptr<ContentLUT> const lut);
    ~WorldConverter();

    void convertNext();

    void setOnComplete(runnable callback) {
        this->onComplete = callback;
    }

    void update() override {
        convertNext();
        if (onComplete && tasks.empty()) onComplete();
    }

    void terminate() override {
        tasks = {};
    }

    void write();

    uint getWorkRemaining() const override;
    uint getWorkDone() const override;
};

#endif // FILES_WORLD_CONVERTER_H_
