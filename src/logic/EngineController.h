#ifndef LOGIC_ENGINE_CONTROLLER_H_
#define LOGIC_ENGINE_CONTROLLER_H_

#include <string>
#include <vector>

class Engine;
class World;
class LevelController;

class EngineController {
private:
    Engine* engine;
public:
    EngineController(Engine* engine);

    void openWorld(const std::string& name, bool confirmConvert);
    void deleteWorld(const std::string& name);
    void createWorld(
        const std::string& name, 
        const std::string& seedstr,
        const std::string& generatorID
    );
    void reopenWorld(World* world);

    void reconfigPacks(
        LevelController* controller,
        const std::vector<std::string>& packsToAdd,
        const std::vector<std::string>& packsToRemove
    );
};

#endif // LOGIC_ENGINE_CONTROLLER_H_
