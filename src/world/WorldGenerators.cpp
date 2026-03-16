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
    for(std::map<std::string, gen_constructor>::iterator it = generators.begin(); it != generators.end(); ++it) {
        ids.push_back(it->first);
    }

    return ids;
}

std::string WorldGenerators::getDefaultGeneratorID() {
    return BUILTIN_CONTENT_NAMESPACE + ":default";
}

WorldGenerator* WorldGenerators::createGenerator(std::string id, const Content* content) {
    for(std::map<std::string, gen_constructor>::iterator it = generators.begin(); it != generators.end(); ++it) {
        if (id == it->first) return (WorldGenerator*) it->second(content);
    }

    LOG_ERROR("Unknown generator ID: {}", id);
    return nullptr;
}
