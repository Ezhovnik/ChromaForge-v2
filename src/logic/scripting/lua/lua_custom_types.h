#pragma once

#include <string>
#include <vector>

#include <logic/scripting/lua/lua_commons.h>
#include <math/Heightmap.h>

struct fnl_state;

namespace lua {
    class Userdata {
    public:
        virtual ~Userdata() {};
        virtual const std::string& getTypeName() const = 0;
    };

    class Bytearray : public Userdata {
    private:
        std::vector<ubyte> buffer;
    public:
        Bytearray(size_t capacity);
        Bytearray(std::vector<ubyte> buffer);
        virtual ~Bytearray();

        const std::string& getTypeName() const override {
            return TYPENAME;
        }

        inline std::vector<ubyte>& data() {
            return buffer;
        }

        static int createMetatable(lua::State*);

        inline static std::string TYPENAME = "Bytearray";
    };

    class LuaHeightmap : public Userdata {
    private:
        std::shared_ptr<Heightmap> map;
        std::unique_ptr<fnl_state> noise;
    public:
        LuaHeightmap(uint width, uint height);

        virtual ~LuaHeightmap();

        uint getWidth() const {
            return map->getWidth();
        }

        uint getHeight() const {
            return map->getHeight();
        }

        float* getValues() {
            return map->getValues();
        }

        const float* getValues() const {
            return map->getValues();
        }

        const std::string& getTypeName() const override {
            return TYPENAME;
        }

        std::shared_ptr<Heightmap> getHeightmap() const {
            return map;
        }

        fnl_state* getNoise() {
            return noise.get();
        }

        void setSeed(uint64_t seed);

        static int createMetatable(lua::State*);

        inline static std::string TYPENAME = "Heightmap";
    };
}
