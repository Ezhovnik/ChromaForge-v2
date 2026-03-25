#include "WorldGenerators.h"

#include <vector>
#include <map>
#include <string>

#include "../voxels/WorldGenerator.h"
#include "../voxels/FlatWorldGenerator.h"
#include "../content/Content.h"
#include "../debug/Logger.h"
#include "../core_content_defs.h"

std::vector<std::string> WorldGenerators::getGeneratorsIDs() {
    std::vector<std::string> ids;
    for (auto& entry : generators) {
        ids.push_back(entry.first);
    }

    return ids;
}

std::string WorldGenerators::getDefaultGeneratorID() {
    return BUILTIN_CONTENT_NAMESPACE + ":default";
}

std::unique_ptr<WorldGenerator> WorldGenerators::createGenerator(std::string id, const Content* content) {
    auto found = generators.find(id);
    if (found == generators.end()) {
        LOG_ERROR("Unknown generator ID: {}", id);
        throw std::runtime_error("Unknown generator ID: " + id);
    }

    return std::unique_ptr<WorldGenerator>(found->second(content));
}
