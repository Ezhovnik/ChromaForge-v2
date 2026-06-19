#pragma once

#include <string>
#include <vector>
#include <memory>

#include <typedefs.h>

class Engine;
class World;
class LevelController;
class ContentReport;

class EngineController {
private:
    Engine& engine;

    int64_t localPlayer = -1;
    void onMissingContent(const std::shared_ptr<ContentReport>& report);
public:
    EngineController(Engine& engine);

    void openWorld(const std::string& name, bool confirmConvert);
    void deleteWorld(const std::string& name);
    void createWorld(
        const std::string& name, 
        const std::string& seedstr,
        const std::string& generatorID
    );
    void reopenWorld(World* world);

    void setLocalPlayer(int64_t player);

    void reconfigPacks(
        LevelController* controller,
        const std::vector<std::string>& packsToAdd,
        const std::vector<std::string>& packsToRemove
    );
};
