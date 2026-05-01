#include <logic/scripting/lua/lua_util.h>

#include <iostream>
#include <iomanip>

#include <util/stringutil.h>

using namespace lua;

static int nextEnvironment = 1;

std::unordered_map<std::type_index, std::string> lua::usertypeNames;

int lua::userdata_destructor(lua::State* L) {
    if (auto obj = touserdata<Userdata>(L, 1)) {
        obj->~Userdata();
    }
    return 0;
}

std::string lua::env_name(int env) {
    return "_ENV" + util::mangleid(env);
}

int lua::pushvalue(lua::State* L, const dv::value& value) {
    switch (value.getType()) {
        case dv::value_type::None:
            pushnil(L);
            break;
        case dv::value_type::Boolean:
            pushboolean(L, value.asBoolean());
            break;
        case dv::value_type::Number:
            pushnumber(L, value.asNumber());
            break;
        case dv::value_type::Integer:
            pushinteger(L, value.asInteger());
            break;
        case dv::value_type::String:
            pushstring(L, value.asString());
            break;
        case dv::value_type::List: {
            createtable(L, value.size(), 0);
            size_t index = 1;
            for (const auto& elem : value) {
                pushvalue(L, elem);
                rawseti(L, index);
                index++;
            }
            break;
        }
        case dv::value_type::Object: {
            createtable(L, 0, value.size());
            for (const auto& [key, elem] : value.asObject()) {
                pushvalue(L, elem);
                setfield(L, key);
            }
        }
    }
    return 1;
}

std::wstring lua::require_wstring(lua::State* L, int idx) {
    return util::str2wstr_utf8(lua::require_string(L, idx));
}

int lua::pushwstring(lua::State* L, const std::wstring& str) {
    return lua::pushstring(L, util::wstr2str_utf8(str));
}

dv::value lua::tovalue(State* L, int idx) {
    auto type = lua::type(L, idx);
    switch (type) {
        case LUA_TNIL:
        case LUA_TNONE:
            return nullptr;
        case LUA_TBOOLEAN:
            return toboolean(L, idx) == 1;
        case LUA_TNUMBER: {
            auto number = tonumber(L, idx);
            auto integer = tointeger(L, idx);
            if (number == static_cast<lua::Number>(integer)) {
                return integer;
            } else {
                return number;
            }
        }
        case LUA_TFUNCTION:
            return "<function " + std::to_string(reinterpret_cast<ptrdiff_t>(lua_topointer(L, idx))) + ">";
        case LUA_TSTRING:
            return std::string(tostring(L, idx));
        case LUA_TTABLE: {
            int len = lua::objlen(L, idx);
            if (len) {
                auto list = dv::list();
                for (int i = 1; i <= len; ++i) {
                    rawgeti(L, i, idx);
                    list.add(tovalue(L, -1));
                    pop(L);
                }
                return list;
            } else {
                auto map = dv::object();
                pushvalue(L, idx);
                pushnil(L);
                while (next(L, -2)) {
                    pushvalue(L, -2);
                    auto key = tostring(L, -1);
                    map[key] = tovalue(L, -2);
                    pop(L, 2);
                }
                pop(L);
                return map;
            }
        }
        default:
            std::string errLog = "Lua type " + std::string(lua_typename(L, type)) + " is not supported";
            log_error(errLog);
            throw std::runtime_error(errLog);
    }
}

static int l_error_handler(lua_State* L) {
    if (!isstring(L, 1)) return 1;
    if (get_from(L, "debug", "traceback")) {
        lua_pushvalue(L, 1);
        lua_pushinteger(L, 2);
        lua_call(L, 2, 1);
    }
    return 1;
}

int lua::call(lua::State* L, int argc, int nresults) {
    int handler_pos = gettop(L) - argc;
    pushcfunction(L, l_error_handler);
    insert(L, handler_pos);
    if (lua_pcall(L, argc, nresults, handler_pos)) {
        std::string log = tostring(L, -1);
        pop(L);
        remove(L, handler_pos);
        throw luaerror(log);
    }
    remove(L, handler_pos);
    return nresults == -1 ? 1 : nresults;
}

int lua::call_nothrow(lua::State* L, int argc, int nresults) {
    int handler_pos = gettop(L) - argc;
    pushcfunction(L, l_error_handler);
    insert(L, handler_pos);
    if (lua_pcall(L, argc, LUA_MULTRET, handler_pos)) {
        log_error(tostring(L, -1));
        pop(L);
        remove(L, handler_pos);
        return 0;
    }
    remove(L, handler_pos);
    return nresults == -1 ? 1 : nresults;
}

void lua::dump_stack(lua::State* L) {
    int top = gettop(L);
    for (int i = 1; i <= top; ++i) {
        std::cout << std::setw(3) << i << std::setw(20) << luaL_typename(L, i) << std::setw(30);
        switch (lua::type(L, i)) {
            case LUA_TNUMBER:
                std::cout << tonumber(L, i);
                break;
            case LUA_TSTRING:
                std::cout << tostring(L, i);
                break;
            case LUA_TBOOLEAN:
                std::cout << (toboolean(L, i) ? "true" : "false");
                break;
            case LUA_TNIL:
                std::cout << "nil";
                break;
            default:
                std::cout << topointer(L, i);
                break;
        }
        std::cout << std::endl;
    }
}

static std::shared_ptr<std::string> create_lambda_handler(lua::State* L) {
    auto ptr = reinterpret_cast<ptrdiff_t>(topointer(L, -1));
    auto name = util::mangleid(ptr);
    getglobal(L, LAMBDAS_TABLE);
    pushvalue(L, -2);
    setfield(L, name);
    pop(L, 2);

    return std::shared_ptr<std::string>(new std::string(name), [=](std::string* name) {
        getglobal(L, LAMBDAS_TABLE);
        pushnil(L);
        setfield(L, *name);
        pop(L);
        delete name;
    });
}

runnable lua::create_runnable(lua::State* L) {
    auto funcptr = create_lambda_handler(L);
    return [=]() {
        getglobal(L, LAMBDAS_TABLE);
        getfield(L, *funcptr);
        call_nothrow(L, 0);
    };
}

scripting::common_func lua::create_lambda(lua::State* L) {
    auto funcptr = create_lambda_handler(L);
    return [=](const std::vector<dv::value>& args) -> dv::value {
        getglobal(L, LAMBDAS_TABLE);
        getfield(L, *funcptr);
        for (const auto& arg : args) {
            pushvalue(L, arg);
        }
        if (call(L, args.size(), 1)) {
            auto result = tovalue(L, -1);
            pop(L);
            return result;
        }
        return nullptr;
    };
}

int lua::create_environment(lua::State* L, int parent) {
    int id = nextEnvironment++;

    createtable(L, 0, 1);

    createtable(L, 0, 1);
    if (parent == 0) {
        pushglobals(L);
    } else {
        if (pushenv(L, parent) == 0) {
            pushglobals(L);
        }
    }
    setfield(L, "__index");
    setmetatable(L);

    setglobal(L, env_name(id));
    return id;
}

void lua::removeEnvironment(lua::State* L, int id) {
    if (id == 0) return;

    pushnil(L);
    setglobal(L, env_name(id));
}
