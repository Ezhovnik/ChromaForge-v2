#include <content/ContentLoader.h>

#include <algorithm>

#include <content/ContentPack.h>
#include <files/files.h>
#include <logic/scripting/scripting.h>
#include <world/generator/Generator.h>
#include <world/generator/VoxelFragment.h>
#include <debug/Logger.h>

static BlocksLayer load_layer(
    const dv::value& map, uint& lastLayersHeight, bool& hasResizeableLayer
) {
    const auto& name = map["block"].asString();
    int height = map["height"].asInteger();
    bool belowSeaLevel = true;
    map.at("below_sea_level").get(belowSeaLevel);

    if (hasResizeableLayer) {
        lastLayersHeight += height;
    }
    if (height == -1) {
        if (hasResizeableLayer) {
            LOG_ERROR("Only one resizeable layer allowed");
            throw std::runtime_error("Only one resizeable layer allowed");
        }
        hasResizeableLayer = true;
    }
    return BlocksLayer {name, height, belowSeaLevel, {}};
}

static inline BlocksLayers load_layers(
    const dv::value& layersArr, const std::string& fieldname
) {
    uint lastLayersHeight = 0;
    bool hasResizeableLayer = false;
    std::vector<BlocksLayer> layers;

    for (int i = 0; i < layersArr.size(); i++) {
        const auto& layerMap = layersArr[i];
        try {
            layers.push_back(
                load_layer(layerMap, lastLayersHeight, hasResizeableLayer)
            );
        } catch (const std::runtime_error& err) {
            LOG_ERROR("{} №{}: {}", fieldname, i, err.what());
            throw std::runtime_error(
                fieldname + " №" + std::to_string(i) + ": " + err.what()
            );
        }
    }
    return BlocksLayers {std::move(layers), lastLayersHeight};
}

static inline BiomeElementList load_biome_element_list(
    const dv::value map,
    const std::string& chanceName,
    const std::string& arrName,
    const std::string& nameName
) {
    float chance = 0.0f;
    map.at(chanceName).get(chance);
    std::vector<WeightedEntry> entries;
    if (map.has(arrName)) {
        const auto& arr = map[arrName];
        for (const auto& entry : arr) {
            const auto& name = entry[nameName].asString();
            float weight = entry["weight"].asNumber();
            if (weight <= 0.0f) {
                LOG_ERROR("Weight must be positive");
                throw std::runtime_error("Weight must be positive");
            }
            entries.push_back(WeightedEntry {name, weight, {}});
        }
    }
    std::sort(entries.begin(), entries.end(), std::greater<WeightedEntry>());
    return BiomeElementList(std::move(entries), chance);
}

static inline BiomeElementList load_plants(const dv::value& biomeMap) {
    return load_biome_element_list(biomeMap, "plant_chance", "plants", "block");
}

static inline BiomeElementList load_structures(const dv::value map) {
    return load_biome_element_list(map, "structure_chance", "structures", "name");
}

static inline Biome load_biome(
    const dv::value& biomeMap,
    const std::string& name,
    uint parametersCount
) {
    std::vector<BiomeParameter> parameters;

    const auto& paramsArr = biomeMap["parameters"];
    if (paramsArr.size() < parametersCount) {
        LOG_ERROR("{} parameters expected", parametersCount);
        throw std::runtime_error(
            std::to_string(parametersCount) + " parameters expected"
        );
    }
    for (size_t i = 0; i < parametersCount; ++i) {
        const auto& paramMap = paramsArr[i];
        float value = paramMap["value"].asNumber();
        float weight = paramMap["weight"].asNumber();
        parameters.push_back(BiomeParameter {value, weight});
    }

    auto plants = load_plants(biomeMap);
    auto groundLayers = load_layers(biomeMap["layers"], "layers");
    auto seaLayers = load_layers(biomeMap["sea_layers"], "sea_layers");

    BiomeElementList structures;
    if (biomeMap.has("structures")) {
        structures = load_structures(biomeMap);
    }
    return Biome {
        name,
        std::move(parameters),
        std::move(plants),
        std::move(structures),
        std::move(groundLayers),
        std::move(seaLayers)
    };
}

static VoxelStructureMeta load_structure_meta(
    const std::string& name, const dv::value& config
) {
    VoxelStructureMeta meta;
    meta.name = name;

    return meta;
}

static std::vector<std::unique_ptr<VoxelStructure>> load_structures(
    const std::filesystem::path& structuresFile
) {
    auto structuresDir = structuresFile.parent_path()/std::filesystem::path("fragments");
    auto map = files::read_json(structuresFile);

    std::vector<std::unique_ptr<VoxelStructure>> structures;
    for (auto& [name, config] : map.asObject()) {
        auto structFile = structuresDir / std::filesystem::u8path(name + ".vox");
        LOG_DEBUG("Loading voxel fragment {}", structFile.u8string());
        if (!std::filesystem::exists(structFile)) {
            LOG_ERROR("Structure file does not exist: {}", structFile.u8string());
            throw std::runtime_error("Structure file does not exist: " + structFile.u8string());
        }
        auto fragment = std::make_unique<VoxelFragment>();
        fragment->deserialize(files::read_binary_json(structFile));

        structures.push_back(std::make_unique<VoxelStructure>(
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
static inline const auto BIOMES_FILE = std::filesystem::u8path("biomes.json");
static inline const auto GENERATORS_DIR = std::filesystem::u8path("generators");

static void load_biomes(Generator& def, const std::filesystem::path& file) {
    auto root = files::read_json(file);
    for (const auto& [biomeName, biomeMap] : root.asObject()) {
        try {
            def.biomes.push_back(
                load_biome(biomeMap, biomeName, def.biomeParameters)
            );
        } catch (const std::runtime_error& err) {
            LOG_ERROR("Biome {}: {}", biomeName, err.what());
            throw std::runtime_error("Biome " + biomeName + ": " + err.what());
        }
    }
}

void ContentLoader::loadGenerator(
    Generator& def, const std::string& full, const std::string& name
) {
    auto packDir = pack->folder;
    auto generatorsDir = packDir/GENERATORS_DIR;
    auto generatorFile = generatorsDir/std::filesystem::u8path(name + ".toml");
    if (!std::filesystem::exists(generatorFile)) return;
    auto map = files::read_toml(generatorsDir/std::filesystem::u8path(name + ".toml"));
    map.at("biome_parameters").get(def.biomeParameters);
    map.at("biome-bpd").get(def.biomesBPD);
    map.at("heights-bpd").get(def.heightsBPD);
    map.at("sea_level").get(def.seaLevel);

    auto folder = generatorsDir/std::filesystem::u8path(name + ".files");
    auto scriptFile = folder/std::filesystem::u8path("script.lua");

    auto structuresFile = folder / STRUCTURES_FILE;
    if (std::filesystem::exists(structuresFile)) {
        load_structures(def, structuresFile);
    }

    auto biomesFiles = folder/BIOMES_FILE;
    if (!std::filesystem::exists(biomesFiles)) {
        LOG_ERROR("{}: file not found (at least one biome required)", BIOMES_FILE.u8string());
        throw std::runtime_error(
            BIOMES_FILE.u8string() + ": file not found (at least one biome required)"
        );
    }
    load_biomes(def, biomesFiles);
    def.script = scripting::load_generator(
        def, scriptFile, pack->id + ":generators/" + name + ".files"
    );
}
