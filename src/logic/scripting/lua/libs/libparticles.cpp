#include <logic/scripting/lua/libs/api_lua.h>

#include <logic/scripting/scripting_hud.h>
#include <graphics/render/Emitter.h>
#include <graphics/render/WorldRenderer.h>
#include <assets/assets_util.h>
#include <engine.h>
#include <graphics/render/ParticlesRenderer.h>

static int l_emit(lua::State* L) {
    EmitterOrigin origin;
    if (lua::istable(L, 1)) {
        origin = lua::tovec3(L, 1);
    } else {
        origin = static_cast<entityid_t>(lua::tointeger(L, 1));
    }
    int count = lua::tointeger(L, 2);
    auto preset = lua::tovalue(L, 3);
    auto extension = lua::tovalue(L, 4);

    ParticlesPreset particlesPreset {};
    particlesPreset.deserialize(preset);
    if (extension != nullptr) {
        particlesPreset.deserialize(extension);
    }
    auto& assets = *scripting::engine->getAssets();
    auto region = util::get_texture_region(assets, particlesPreset.texture, "");
    auto emitter = std::make_unique<Emitter>(
        *scripting::level,
        std::move(origin),
        std::move(particlesPreset),
        region.texture,
        region.region,
        count
    );
    return lua::pushinteger(L, scripting::renderer->particles->add(std::move(emitter)));
}

static int l_stop(lua::State* L) {
    uint64_t id = lua::touinteger(L, 1);
    if (auto emitter = scripting::renderer->particles->getEmitter(id)) {
        emitter->stop();
    }
    return 0;
}

const luaL_Reg particleslib[] = {
    {"emit", lua::wrap<l_emit>},
    {"stop", lua::wrap<l_stop>},
    {NULL, NULL}
};
