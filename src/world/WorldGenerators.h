#pragma once

#include <map>
#include <vector>
#include <string>

#include <voxels/WorldGenerator.h>

class Content;

using gen_constructor = WorldGenerator* (*)(const Content*);

class WorldGenerators {
private:
    static inline std::map<std::string, gen_constructor> generators;
public:
    template <typename T>
    static void addGenerator(std::string id);

    static std::vector<std::string> getGeneratorsIDs();

    static std::string getDefaultGeneratorID();

    static std::unique_ptr<WorldGenerator> createGenerator(
        const std::string& id, const Content* content
    );
};

template <typename T>
void WorldGenerators::addGenerator(std::string id) {
    generators[id] = [] (const Content* content) {
        return (WorldGenerator*) new T(content);
    };
}
