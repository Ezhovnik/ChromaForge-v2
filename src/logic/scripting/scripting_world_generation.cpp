#include <logic/scripting/scripting.h>

#include <algorithm>
#include <functional>

#include <logic/scripting/scripting_commons.h>
#include <typedefs.h>
#include <logic/scripting/lua/lua_engine.h>
#include <logic/scripting/lua/lua_custom_types.h>
#include <content/Content.h>
#include <voxels/Block.h>
#include <voxels/Chunk.h>
#include <world/generator/Generator.h>
#include <debug/Logger.h>
#include <data/dv.h>
#include <util/timeutil.h>
#include <files/files.h>
#include <engine.h>

class LuaGeneratorScript : public GeneratorScript {
    lua::State* L;
    const Generator& def;
    scriptenv env = nullptr;

    std::filesystem::path file;
    std::string dirPath;
public:
    LuaGeneratorScript(
        lua::State* L,
        const Generator& def,
        const std::filesystem::path& file,
        const std::string& dirPath
    ) : L(L), def(def), file(file), dirPath(dirPath) {}

    virtual ~LuaGeneratorScript() {
        env.reset();
        if (L != lua::get_main_state()) {
            lua::close(L);
        }
    }

    void initialize(uint64_t seed) override {
        env = lua::create_environment(L);
        lua::stackguard _(L);

        lua::pushenv(L, *env);
        lua::pushstring(L, dirPath);
        lua::setfield(L, "__DIR__");
        lua::pushstring(L, dirPath + "/script.lua");
        lua::setfield(L, "__FILE__");
        lua::pushinteger(L, seed);
        lua::setfield(L, "SEED");

        lua::pop(L);

        if (std::filesystem::exists(file)) {
            std::string src = files::read_string(file);
            lua::pop(L, lua::execute(L, *env, src, file.u8string()));
        } else {
            lua::pop(L, lua::execute(L, *env, "", "<empty>"));
        }
    }

    std::shared_ptr<Heightmap> generateHeightmap(
        const glm::ivec2& offset,
        const glm::ivec2& size,
        uint bpd,
        const std::vector<std::shared_ptr<Heightmap>>& inputs
    ) override {
        lua::pushenv(L, *env);
        if (lua::getfield(L, "generate_heightmap")) {
            lua::pushivec_stack(L, offset);
            lua::pushivec_stack(L, size);
            lua::pushinteger(L, bpd);
            if (!inputs.empty()) {
                size_t inputsNum = def.heightmapInputs.size();
                lua::createtable(L, inputsNum, 0);
                for (size_t i = 0; i < inputsNum; ++i) {
                    lua::newuserdata<lua::LuaHeightmap>(L, inputs[i]);
                    lua::rawseti(L, i + 1);
                }
            }
            if (lua::call_nothrow(L, 5 + (!inputs.empty()))) {
                auto map = lua::touserdata<lua::LuaHeightmap>(L, -1)->getHeightmap();
                lua::pop(L, 2);
                return map;
            }
        }
        lua::pop(L);
        return std::make_shared<Heightmap>(size.x, size.y);
    }

    std::vector<std::shared_ptr<Heightmap>> generateParameterMaps(
        const glm::ivec2& offset, const glm::ivec2& size, uint bpd
    ) override {
        std::vector<std::shared_ptr<Heightmap>> maps;

        uint biomeParameters = def.biomeParameters;
        lua::pushenv(L, *env);
        if (lua::getfield(L, "generate_biome_parameters")) {
            lua::pushivec_stack(L, offset);
            lua::pushivec_stack(L, size);
            lua::pushinteger(L, bpd);
            if (lua::call_nothrow(L, 5, biomeParameters)) {
                for (int i = biomeParameters-1; i >= 0; --i) {
                    maps.push_back(
                        lua::touserdata<lua::LuaHeightmap>(L, -1 - i)->getHeightmap()
                    );
                }
                lua::pop(L, 1 + biomeParameters);
                return maps;
            }
        }
        lua::pop(L);
        for (uint i = 0; i < biomeParameters; ++i) {
            maps.push_back(std::make_shared<Heightmap>(size.x, size.y));
        }
        return maps;
    }

    void perform_line(lua::State* L, std::vector<Placement>& placements) {
        lua::rawgeti(L, 2);
        blockid_t block = lua::touinteger(L, -1);
        lua::pop(L);

        lua::rawgeti(L, 3);
        glm::ivec3 a = lua::tovec3(L, -1);
        lua::pop(L);

        lua::rawgeti(L, 4);
        glm::ivec3 b = lua::tovec3(L, -1);
        lua::pop(L);

        lua::rawgeti(L, 5);
        int radius = lua::touinteger(L, -1);
        lua::pop(L);

        int priority = 0;
        if (lua::objlen(L, -1) >= 6) {
            lua::rawgeti(L, 6);
            priority = lua::tointeger(L, -1);
            lua::pop(L);
        }

        placements.emplace_back(priority, LinePlacement {block, a, b, radius});
    }

    void perform_placement(lua::State* L, std::vector<Placement>& placements) {
        lua::rawgeti(L, 1);
        int structIndex = 0;
        if (lua::isstring(L, -1)) {
            const char* name = lua::require_string(L, -1);
            if (!std::strcmp(name, ":line")) {
                lua::pop(L);

                perform_line(L, placements);
                return;
            }
            const auto& found = def.structuresIndices.find(name);
            if (found != def.structuresIndices.end()) {
                structIndex = found->second;
            }
        } else {
            structIndex = lua::tointeger(L, -1);
        }
        lua::pop(L);

        lua::rawgeti(L, 2);
        glm::ivec3 pos = lua::tovec3(L, -1);
        lua::pop(L);

        lua::rawgeti(L, 3);
        uint8_t rotation = lua::tointeger(L, -1) & 0b11;
        lua::pop(L);

        int priority = 1;
        if (lua::objlen(L, -1) >= 4) {
            lua::rawgeti(L, 4);
            priority = lua::tointeger(L, -1);
            lua::pop(L);
        }

        placements.emplace_back(
            priority, StructurePlacement {structIndex, pos, rotation}
        );
    }

    std::vector<Placement> placeStructuresWide(
        const glm::ivec2& offset,
        const glm::ivec2& size,
        uint chunkHeight
    ) override {
        std::vector<Placement> placements {};

        lua::stackguard _(L);
        lua::pushenv(L, *env);
        if (lua::getfield(L, "place_structures_wide")) {
            lua::pushivec_stack(L, offset);
            lua::pushivec_stack(L, size);
            lua::pushinteger(L, chunkHeight);
            if (lua::call_nothrow(L, 5, 1)) {
                int len = lua::objlen(L, -1);
                for (int i = 1; i <= len; ++i) {
                    lua::rawgeti(L, i);

                    perform_placement(L, placements);

                    lua::pop(L);
                }
                lua::pop(L);
            }
        }
        return placements;
    }

    std::vector<Placement> placeStructures(
        const glm::ivec2& offset, const glm::ivec2& size,
        const std::shared_ptr<Heightmap>& heightmap,
        uint chunkHeight
    ) override {
        std::vector<Placement> placements {};

        lua::stackguard _(L);
        lua::pushenv(L, *env);
        try {
            if (lua::getfield(L, "place_structures")) {
                lua::pushivec_stack(L, offset);
                lua::pushivec_stack(L, size);
                lua::newuserdata<lua::LuaHeightmap>(L, heightmap);
                lua::pushinteger(L, chunkHeight);
                if (lua::call_nothrow(L, 6, 1)) {
                    int len = lua::objlen(L, -1);
                    for (int i = 1; i <= len; ++i) {
                        lua::rawgeti(L, i);

                        perform_placement(L, placements);

                        lua::pop(L);
                    }
                    lua::pop(L);
                }
            }
        } catch (const std::runtime_error& err) {
            LOG_ERROR("{}", err.what());
        }
        return placements;
    }
};

std::unique_ptr<GeneratorScript> scripting::load_generator(
    const Generator& def, const std::filesystem::path& file, const std::string& dirPath
) {
    auto L = lua::create_state(scripting::engine->getPaths(), lua::StateType::Generator);
    return std::make_unique<LuaGeneratorScript>(L, def, file, dirPath);
}
