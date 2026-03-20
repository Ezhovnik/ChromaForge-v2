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

    void openWorld(std::string name, bool confirmConvert);
    void deleteWorld(std::string name);
    void createWorld(
        const std::string& name, 
        const std::string& seedstr,
        const std::string& generatorID
    );
    void reopenWorld(World* world);

    void removePacks(
        LevelController* controller, 
        std::vector<std::string> packs
    );
    void addPacks(
        LevelController* controller,
        std::vector<std::string> packs
    );
};

#endif // LOGIC_ENGINE_CONTROLLER_H_
