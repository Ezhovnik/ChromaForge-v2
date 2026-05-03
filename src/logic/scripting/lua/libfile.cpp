#include <string>
#include <filesystem>

#include <logic/scripting/lua/api_lua.h>
#include <engine.h>
#include <files/files.h>
#include <files/engine_paths.h>
#include <util/stringutil.h>
#include <coders/zip.h>

static std::filesystem::path resolve_path(const std::string& path) {
    return scripting::engine->getPaths()->resolve(path);
}

static std::filesystem::path resolve_path_soft(const std::string& path) {
    if (path.find(':') == std::string::npos) return path;

    return scripting::engine->getPaths()->resolve(path, false);
}

static int l_file_resolve(lua::State* L) {
    std::filesystem::path path = resolve_path(lua::require_string(L, 1));
    return lua::pushstring(L, path.u8string());
}

static int l_file_read(lua::State* L) {
    std::filesystem::path path = resolve_path(lua::require_string(L, 1));
    if (std::filesystem::is_regular_file(path)) {
        return lua::pushstring(L, files::read_string(path));
    }
    throw std::runtime_error("File does not exists " + util::quote(path.u8string()));
}

static int l_file_write(lua::State* L) {
    std::filesystem::path path = resolve_path(lua::require_string(L, 1));
    auto text = lua::require_string(L, 2);
    files::write_string(path, text);
    return 1;    
}

static int l_file_exists(lua::State* L) {
    std::filesystem::path path = resolve_path_soft(lua::require_string(L, 1));
    return lua::pushboolean(L, std::filesystem::exists(path));
}

static int l_file_isfile(lua::State* L) {
    std::filesystem::path path = resolve_path_soft(lua::require_string(L, 1));
    return lua::pushboolean(L, std::filesystem::is_regular_file(path));
}

static int l_file_isdir(lua::State* L) {
    std::filesystem::path path = resolve_path_soft(lua::require_string(L, 1));
    return lua::pushboolean(L, std::filesystem::is_directory(path));
}

static int l_file_length(lua::State* L) {
    std::filesystem::path path = resolve_path(lua::require_string(L, 1));
    if (std::filesystem::exists(path)){
        return lua::pushinteger(L, std::filesystem::file_size(path));
    } else {
        return lua::pushinteger(L, -1);
    }
    return 1;
}

static int l_file_mkdir(lua::State* L) {
    std::filesystem::path path = resolve_path(lua::require_string(L, 1));
    return lua::pushboolean(L, std::filesystem::create_directory(path));    
}

static int l_file_mkdirs(lua::State* L) {
    std::filesystem::path path = resolve_path(lua::require_string(L, 1));
    return lua::pushboolean(L, std::filesystem::create_directories(path));    
}

static int l_file_read_bytes(lua::State* L) {
    std::filesystem::path path = resolve_path(lua::require_string(L, 1));
    if (std::filesystem::is_regular_file(path)) {
        size_t length = static_cast<size_t>(std::filesystem::file_size(path));

        auto bytes = files::read_bytes(path, length);

        lua::createtable(L, length, 0);
        int newTable = lua::gettop(L);

        for(size_t i = 0; i < length; ++i) {
            lua::pushinteger(L, bytes[i]);
            lua::rawseti(L, i + 1, newTable);
        }
        return 1;
    }
    throw std::runtime_error("File does not exists " + util::quote(path.u8string()));
}

static int read_bytes_from_table(lua::State* L, int tableIndex, std::vector<ubyte>& bytes) {
    if (!lua::istable(L, tableIndex)) throw std::runtime_error("Table expected");
    lua::pushnil(L);
    while(lua::next(L, tableIndex - 1) != 0) {
        const int byte = lua::tointeger(L, -1);
        if (byte < 0 || byte > 255) throw std::runtime_error("Invalid byte '" + std::to_string(byte) + "'");
        bytes.push_back(byte);
        lua::pop(L);
    }
    return 1;
}

static int l_file_write_bytes(lua::State* L) {
    int pathIndex = 1;
    if (!lua::isstring(L, pathIndex)) throw std::runtime_error("String expected");
    std::filesystem::path path = resolve_path(lua::require_string(L, pathIndex));
    if (auto bytearray = lua::touserdata<lua::LuaBytearray>(L, -1)) {
        auto& bytes = bytearray->data();
        return lua::pushboolean(L, files::write_bytes(path, bytes.data(), bytes.size()));
    }
    std::vector<ubyte> bytes;
    int result = read_bytes_from_table(L, -1, bytes);
    if (result != 1) {
        return result;
    } else {
        return lua::pushboolean(L, files::write_bytes(path, bytes.data(), bytes.size()));
    }
}

static int l_file_find(lua::State* L) {
    std::string path = lua::require_string(L, 1);

    try {
        return lua::pushstring(L, scripting::engine->getResPaths()->findRaw(path));
    } catch (const std::runtime_error& err) {
        return 0;
    }
}

static int l_file_list_all_res(lua::State* L, const std::string& path) {
    auto files = scripting::engine->getResPaths()->listdirRaw(path);
    lua::createtable(L, files.size(), 0);
    for (size_t i = 0; i < files.size(); ++i) {
        lua::pushstring(L, files[i]);
        lua::rawseti(L, i + 1);
    }
    return 1;
}

static int l_file_list(lua::State* L) {
    std::string dirname = lua::require_string(L, 1);
    if (dirname.find(':') == std::string::npos) {
        return l_file_list_all_res(L, dirname);
    }
    std::filesystem::path path = resolve_path(dirname);
    if (!std::filesystem::is_directory(path)) {
        throw std::runtime_error(util::quote(path.u8string()) + " is not a directory");
    }
    lua::createtable(L, 0, 0);
    size_t index = 1;
    for (auto& entry : std::filesystem::directory_iterator(path)) {
        auto name = entry.path().filename().u8string();
        auto file = dirname + "/" + name;
        lua::pushstring(L, file);
        lua::rawseti(L, index);
        ++index;
    }
    return 1;
}

static int l_file_remove(lua::State* L) {
    std::string rawpath = lua::require_string(L, 1);
    std::filesystem::path path = resolve_path(rawpath);
    auto entryPoint = rawpath.substr(0, rawpath.find(':'));
    if (entryPoint != "world") {
        throw std::runtime_error("Access denied");
    }
    return lua::pushboolean(L, std::filesystem::remove(path));
}

static int l_file_remove_tree(lua::State* L) {
    std::string rawpath = lua::require_string(L, 1);
    std::filesystem::path path = resolve_path(rawpath);
    auto entryPoint = rawpath.substr(0, rawpath.find(':'));
    if (entryPoint != "world") {
        throw std::runtime_error("Access denied");
    }
    return lua::pushinteger(L, std::filesystem::remove_all(path));
}

static int l_file_zip_compress(lua::State* L) {
    std::vector<ubyte> bytes;
    int result = read_bytes_from_table(L, -1, bytes);
    if (result != 1) {
        return result;
    } else {
        auto compressed_bytes = zip::compress(bytes.data(), bytes.size());
        int newTable = lua::gettop(L);

        for(size_t i = 0; i < compressed_bytes.size(); ++i) {
            lua::pushinteger(L, compressed_bytes.data()[i]);
            lua::rawseti(L, i + 1, newTable);
        }
        return 1;
    }
}

static int l_file_zip_decompress(lua::State* L) { 
    std::vector<ubyte> bytes;
    int result = read_bytes_from_table(L, -1, bytes);

    if (result != 1) {
        return result;
    } else {
        auto decompressed_bytes = zip::decompress(bytes.data(), bytes.size());
        int newTable = lua::gettop(L);

        for(size_t i = 0; i < decompressed_bytes.size(); ++i) {
            lua::pushinteger(L, decompressed_bytes.data()[i]);
            lua::rawseti(L, i + 1, newTable);
        }
        return 1;
    } 
}

static int l_file_read_combined_list(lua::State* L) {
    std::string path = lua::require_string(L, 1);
    if (path.find(':') != std::string::npos) {
        throw std::runtime_error("Entry point must not be specified");
    }
    return lua::pushvalue(L, scripting::engine->getResPaths()->readCombinedList(path));
}

const luaL_Reg filelib [] = {
    {"exists", lua::wrap<l_file_exists>},
    {"find", lua::wrap<l_file_find>},
    {"isdir", lua::wrap<l_file_isdir>},
    {"isfile", lua::wrap<l_file_isfile>},
    {"length", lua::wrap<l_file_length>},
    {"list", lua::wrap<l_file_list>},
    {"mkdir", lua::wrap<l_file_mkdir>},
    {"mkdirs", lua::wrap<l_file_mkdirs>},
    {"read_bytes", lua::wrap<l_file_read_bytes>},
    {"read", lua::wrap<l_file_read>},
    {"remove", lua::wrap<l_file_remove>},
    {"remove_tree", lua::wrap<l_file_remove_tree>},
    {"resolve", lua::wrap<l_file_resolve>},
    {"write_bytes", lua::wrap<l_file_write_bytes>},
    {"write", lua::wrap<l_file_write>},
    {"zip_compress", lua::wrap<l_file_zip_compress>},
    {"zip_decompress", lua::wrap<l_file_zip_decompress>},
    {"read_combined_list", lua::wrap<l_file_read_combined_list>},
    {NULL, NULL}
};
