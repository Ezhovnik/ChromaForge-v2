#include <logic/scripting/lua/usertypes/lua_type_pcmstream.h>

#include <logic/scripting/lua/lua_util.h>
#include <assets/Assets.h>
#include <audio/MemoryPCMStream.h>
#include <engine/Engine.h>

lua::LuaPCMStream::LuaPCMStream(
    std::shared_ptr<audio::MemoryPCMStream>&& stream
) : stream(std::move(stream)) {}

lua::LuaPCMStream::~LuaPCMStream() = default;

const std::shared_ptr<audio::MemoryPCMStream>& lua::LuaPCMStream::getStream() const {
    return stream;
}

static int l_feed(lua::State* L) {
    auto stream = lua::touserdata<lua::LuaPCMStream>(L, 1);
    if (stream == nullptr) return 0;

    auto bytes = lua::bytearray_as_string(L, 2);
    stream->getStream()->feed(
        {reinterpret_cast<const ubyte*>(bytes.data()), bytes.size()}
    );
    return 0;
}

static int l_share(lua::State* L) {
    auto stream = lua::touserdata<lua::LuaPCMStream>(L, 1);
    if (stream == nullptr) return 0;

    auto alias = lua::require_lstring(L, 2);
    if (scripting::engine->isHeadless()) return 0;

    auto assets = scripting::engine->getAssets();
    assets->store<audio::PCMStream>(stream->getStream(), std::string(alias));
    return 0;
}

static std::unordered_map<std::string, lua_CFunction> methods {
    {"feed", lua::wrap<l_feed>},
    {"share", lua::wrap<l_share>},
};

static int l_meta_meta_call(lua::State* L) {
    auto sampleRate = lua::touinteger(L, 2);
    auto channels = lua::touinteger(L, 3);
    auto bitsPerSample = lua::touinteger(L, 4);
    auto stream = std::make_shared<audio::MemoryPCMStream>(sampleRate, channels, bitsPerSample);
    return lua::newuserdata<lua::LuaPCMStream>(L, std::move(stream));
}

static int l_meta_tostring(lua::State* L) {
    return lua::pushstring(L, "PCMStream");
}

static int l_meta_index(lua::State* L) {
    auto stream = lua::touserdata<lua::LuaPCMStream>(L, 1);
    if (stream == nullptr) return 0;

    if (lua::isstring(L, 2)) {
        auto found = methods.find(lua::tostring(L, 2));
        if (found != methods.end()) {
            return lua::pushcfunction(L, found->second);
        }
    }
    return 0;
}

int lua::LuaPCMStream::createMetatable(lua::State* L) {
    createtable(L, 0, 3);
    pushcfunction(L, lua::wrap<l_meta_tostring>);
    setfield(L, "__tostring");
    pushcfunction(L, lua::wrap<l_meta_index>);
    setfield(L, "__index");

    createtable(L, 0, 1);
    pushcfunction(L, lua::wrap<l_meta_meta_call>);
    setfield(L, "__call");
    setmetatable(L);
    return 1;
}
