#include <cmath>
#include <filesystem>
#include <stdexcept>

#include <logic/scripting/lua/libs/api_lua.h>
#include <world/Level.h>
#include <world/World.h>
#include <engine/Engine.h>
#include <assets/Assets.h>
#include <assets/AssetsLoader.h>
#include <files/engine_paths.h>
#include <coders/json.h>
#include <files/files.h>
#include <voxels/Chunks.h>
#include <voxels/Chunk.h>
#include <coders/compression.h>
#include <lighting/Lighting.h>
#include <voxels/GlobalChunks.h>
#include <logic/LevelController.h>
#include <logic/ChunksController.h>
#include <coders/rle.h>
#include <coders/zip.h>

static WorldInfo& require_world_info() {
    if (scripting::level == nullptr) {
        throw std::runtime_error("No world open");
    }
    return scripting::level->getWorld()->getInfo();
}

static int l_get_list(lua::State* L) {
    const auto& paths = scripting::engine->getPaths();
    auto worlds = paths.scanForWorlds();

    lua::createtable(L, worlds.size(), 0);
    for (size_t i = 0; i < worlds.size(); ++i) {
        lua::createtable(L, 0, 1);

        const auto& folder = worlds[i];

        auto root = json::parse(files::read_string(folder/std::filesystem::u8path("world.json")));
        const auto& versionMap = root["version"];
        int versionMajor = versionMap["major"].asInteger();
        int versionMinor = versionMap["minor"].asInteger();
        int versionMaintenance = versionMap["maintenance"].asInteger();

        auto name = folder.filename().u8string();
        lua::pushstring(L, name);
        lua::setfield(L, "name");

        auto assets = scripting::engine->getAssets();
        std::string icon = "world#" + name + ".icon";

        if (!AssetsLoader::loadExternalTexture(assets, icon, {
            worlds[i]/std::filesystem::path("icon.png"),
            worlds[i]/std::filesystem::path("preview.png")
        })) {
            icon = "gui/no_world_icon";
        }
        lua::pushstring(L, icon);
        lua::setfield(L, "icon");

        lua::pushvec3(L, {versionMajor, versionMinor, versionMaintenance});
        lua::setfield(L, "version");

        lua::rawseti(L, i + 1);
    }
    return 1;
}

static int l_get_total_time(lua::State* L) {
    return lua::pushnumber(L, require_world_info().totalTime);
}

static int l_get_day_time(lua::State* L) {
    return lua::pushnumber(L, require_world_info().daytime);
}

static int l_set_day_time(lua::State* L) {
    auto value = lua::tonumber(L, 1);
    require_world_info().daytime = std::fmod(value, 1.0);
    return 0;
}

static int l_set_day_time_speed(lua::State* L) {
    auto value = lua::tonumber(L, 1);
    require_world_info().daytimeSpeed = std::abs(value);
    return 0;
}

static int l_get_day_time_speed(lua::State* L) {
    return lua::pushnumber(L, require_world_info().daytimeSpeed);
}

static int l_get_seed(lua::State* L) {
    return lua::pushinteger(L, require_world_info().seed);
}

static int l_exists(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto worldsDir = scripting::engine->getPaths().getWorldFolderByName(name);
    return lua::pushboolean(L, std::filesystem::is_directory(worldsDir));
}

static int l_is_day(lua::State* L) {
    auto daytime = require_world_info().daytime;
    return lua::pushboolean(L, daytime >= 0.2 && daytime <= 0.8);
}

static int l_is_night(lua::State* L) {
    auto daytime = require_world_info().daytime;
    return lua::pushboolean(L, daytime < 0.2 || daytime > 0.8);
}

static int l_get_generator(lua::State* L) {
    return lua::pushstring(L, require_world_info().generator);
}

static int l_is_open(lua::State* L) {
    return lua::pushboolean(L, scripting::level != nullptr);
}

static std::vector<ubyte> prepare_chunk_data(const Chunk& chunk, bool compress) {
    auto data = chunk.encode();

    std::vector<ubyte> chunkData;
    if (compress) {
        static util::Buffer<ubyte> rleBuffer;
        if (rleBuffer.size() < CHUNK_DATA_LEN * 2) {
            rleBuffer = util::Buffer<ubyte>(CHUNK_DATA_LEN * 2);
        }
        size_t rleCompressedSize = extrle::encode16(data.get(), CHUNK_DATA_LEN, rleBuffer.data());

        const auto zipCompressedData = zip::compress(
            rleBuffer.data(), rleCompressedSize
        );
        auto tmp = dataio::h2le(rleCompressedSize);
        chunkData.reserve(zipCompressedData.size() + sizeof(tmp));
        chunkData.insert(
            chunkData.begin() + 0, (char*)&tmp, ((char*)&tmp) + sizeof(tmp)
        );
        chunkData.insert(
            chunkData.begin() + sizeof(tmp),
            zipCompressedData.data(),
            zipCompressedData.data() + zipCompressedData.size()
        );
    } else {
        chunkData.reserve(CHUNK_DATA_LEN);
        chunkData.insert(
            chunkData.begin(), data.get(), data.get() + CHUNK_DATA_LEN
        );
    }
    return chunkData;
}

static int l_get_chunk_data(lua::State* L) {
    int x = static_cast<int>(lua::tointeger(L, 1));
    int y = static_cast<int>(lua::tointeger(L, 2));
    const auto& chunk = scripting::level->chunks->getChunk(x, y);
    if (chunk == nullptr) {
        lua::pushnil(L);
        return 0;
    }

    bool compress = true;
    if (lua::gettop(L) >= 3) {
        compress = lua::toboolean(L, 3);
    }
    auto chunkData = prepare_chunk_data(*chunk, compress);
    return lua::newuserdata<lua::LuaBytearray>(L, std::move(chunkData));
}

static int l_set_chunk_data(lua::State* L) {
    int x = static_cast<int>(lua::tointeger(L, 1));
    int y = static_cast<int>(lua::tointeger(L, 2));
    auto buffer = lua::touserdata<lua::LuaBytearray>(L, 3);
    bool isCompressed = true;
    if (lua::gettop(L) >= 4) {
        isCompressed = lua::toboolean(L, 4);
    }
    auto chunk = scripting::level->chunks->getChunk(x, y);
    if (chunk == nullptr) return 0;
    if (isCompressed) {
        std::vector<ubyte>& rawData = buffer->data();
        size_t zipDecompressedSize = dataio::le2h(*(size_t*)(rawData.data()));
        auto rleData = compression::decompress(
            rawData.data() + sizeof(zipDecompressedSize),
            buffer->data().size() - sizeof(zipDecompressedSize),
            zipDecompressedSize,
            compression::Method::Zip
        );
        auto data = compression::decompress(
            rleData.get(),
            zipDecompressedSize,
            CHUNK_DATA_LEN,
            compression::Method::Extrle16
        );
        chunk->decode(data.get());
    } else {
        chunk->decode(buffer->data().data());
    }

    chunk->setModifiedAndUnsaved();
    chunk->updateHeights();

    auto chunksController = scripting::controller->getChunksController();
    if (chunksController->lighting == nullptr) {
        return lua::pushboolean(L, true);
    }

    Lighting& lighting = *chunksController->lighting;
    chunk->flags.loadedLights = false;
    chunk->flags.lighted = false;

    Lighting::preBuildSkyLight(*chunk, *scripting::indices);
    lighting.onChunkLoaded(x, y, true);

    for (int lz = -1; lz <= 1; ++lz) {
        for (int lx = -1; lx <= 1; ++lx) {
            if (std::abs(lx) + std::abs(lz) != 1) {
                continue;
            }
            chunk = scripting::level->chunks->getChunk(x + lx, y + lz);
            if (chunk != nullptr) {
                chunk->flags.modified = true;
                lighting.onChunkLoaded(x - 1, y, true);
            }
        }
    }

    return lua::pushboolean(L, true);
}

static int l_count_chunks(lua::State* L) {
    if (scripting::level == nullptr) {
        return 0;
    }
    return lua::pushinteger(L, scripting::level->chunks->size());
}

const luaL_Reg worldlib [] = {
    {"is_open", lua::wrap<l_is_open>},
    {"get_list", lua::wrap<l_get_list>},
    {"get_total_time", lua::wrap<l_get_total_time>},
    {"get_day_time", lua::wrap<l_get_day_time>},
    {"set_day_time", lua::wrap<l_set_day_time>},
    {"set_day_time_speed", lua::wrap<l_set_day_time_speed>},
    {"get_day_time_speed", lua::wrap<l_get_day_time_speed>},
    {"get_seed", lua::wrap<l_get_seed>},
    {"is_day", lua::wrap<l_is_day>},
    {"is_night", lua::wrap<l_is_night>},
    {"exists", lua::wrap<l_exists>},
    {"get_generator", lua::wrap<l_get_generator>},
    {"get_chunk_data", lua::wrap<l_get_chunk_data>},
    {"set_chunk_data", lua::wrap<l_set_chunk_data>},
    {"count_chunks", lua::wrap<l_count_chunks>},
    {NULL, NULL}
};
