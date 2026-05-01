#include <content/ContentLoader.h>

#include <content/ContentPack.h>
#include <files/files.h>
#include <logic/scripting/scripting.h>
#include <world/generator/Generator.h>
#include <world/generator/VoxelFragment.h>
#include <debug/Logger.h>

static VoxelStructureMeta load_structure_meta(
    const std::string& name, const dv::value& config
) {
    VoxelStructureMeta meta;
    meta.name = name;

    return meta;
}

static std::vector<std::unique_ptr<GeneratingVoxelStructure>> load_structures(
    const std::filesystem::path& structuresFile
) {
    auto structuresDir = structuresFile.parent_path();
    auto map = files::read_json(structuresFile);

    std::vector<std::unique_ptr<GeneratingVoxelStructure>> structures;
    for (auto& [name, config] : map.asObject()) {
        auto structFile = structuresDir / std::filesystem::u8path(name + ".vox");
        LOG_DEBUG("Loading voxel fragment {}", structFile.u8string());
        if (!std::filesystem::exists(structFile)) {
            LOG_ERROR("Structure file does not exist: {}", structFile.u8string());
            throw std::runtime_error("Structure file does not exist: " + structFile.u8string());
        }
        auto fragment = std::make_unique<VoxelFragment>();
        fragment->deserialize(files::read_binary_json(structFile));

        structures.push_back(std::make_unique<GeneratingVoxelStructure>(
            load_structure_meta(name, config),
            std::move(fragment)
        ));
    }
    return structures;
}

static void load_structures(Generator& def, const std::filesystem::path& structuresFile) {
    auto rawStructures = load_structures(structuresFile);
    def.structures.resize(rawStructures.size());

    for (int i = 0; i < rawStructures.size(); ++i) {
        def.structures[i] = std::move(rawStructures[i]);
    }

    for (size_t i = 0; i < def.structures.size(); ++i) {
        auto& structure = def.structures[i];
        def.structuresIndices[structure->meta.name] = i;
    }
}

static inline const auto STRUCTURES_FILE = std::filesystem::u8path("structures.json");
static inline const auto GENERATORS_DIR = std::filesystem::u8path("generators");

void ContentLoader::loadGenerator(
    Generator& def, const std::string& full, const std::string& name
) {
    auto packDir = pack->folder;
    auto generatorsDir = packDir / GENERATORS_DIR;
    auto folder = generatorsDir / std::filesystem::u8path(name);
    auto generatorFile = generatorsDir / std::filesystem::u8path(name + ".lua");
    if (!std::filesystem::exists(generatorFile)) {
        return;
    }
    auto structuresFile = folder / STRUCTURES_FILE;
    if (std::filesystem::exists(structuresFile)) {
        load_structures(def, structuresFile);
    }
    def.script = scripting::load_generator(generatorFile);
}
