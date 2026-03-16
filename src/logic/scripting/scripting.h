#ifndef LOGIC_SCRIPTING_SCRIPTING_H_
#define LOGIC_SCRIPTING_SCRIPTING_H_

#include <string>
#include <filesystem>

#include <glm/glm.hpp>

#include "lua/LuaState.h"
#include "../../delegates.h"
#include "scripting_functional.h"

class Engine;
class Content;
class Level;
class Block;
class Player;
class Item;
class BlocksController;
struct block_funcs_set;
struct item_funcs_set;
class ContentIndices;
struct uidocscript;
class Inventory;
class UIDocument;
struct ContentPack;
class LevelController;

namespace scripting {
    extern Engine* engine;
    extern const Content* content;
    extern Level* level;
    extern BlocksController* blocks;
    extern const ContentIndices* indices;
    extern LevelController* controller;

    class Environment;

    void initialize(Engine* engine);

    extern bool register_event(int env, const std::string& name, const std::string& id);

    static inline int noargs(lua::LuaState *) {return 0;}
    extern bool emit_event(const std::string& name, std::function<int(lua::LuaState* state)> args = noargs);

    std::unique_ptr<Environment> create_environment(int parent=0);
    std::unique_ptr<Environment> create_pack_environment(const ContentPack& pack);
    std::unique_ptr<Environment> create_doc_environment(int parent, const std::string& name);

    void on_world_load(LevelController* controller);
    void on_world_quit();
    void on_world_spark();
    void on_world_save();
    void load_world_script(int env, std::string prefix, std::filesystem::path file);

    void process_post_runnables();

    void on_blocks_spark(const Block* block, int tps);
    void update_block(const Block* block, int x, int y, int z);
    void random_update_block(const Block* block, int x, int y, int z);
    void on_block_placed(Player* player, const Block* block, int x, int y, int z);
    void on_block_broken(Player* player, const Block* block, int x, int y, int z);
    bool on_block_interact(Player* player, const Block* block, int x, int y, int z);

    bool on_item_use(Player* player, const Item* item);
    bool on_item_use_on_block(Player* player, const Item* item, int x, int y, int z);
    bool on_item_break_block(Player* player, const Item* item, int x, int y, int z);

    void load_block_script(int env, std::string prefix, std::filesystem::path file, block_funcs_set& funcsset);
    void load_item_script(int env, std::string prefix, std::filesystem::path file, item_funcs_set& funcsset);

    void on_ui_open(UIDocument* layout, Inventory* inventory, glm::ivec3 blockcoord);
    void on_ui_close(UIDocument* layout, Inventory* inventory);

    void load_layout_script(int env, std::string prefix, std::filesystem::path file, uidocscript& script);

    void close();
}

#endif // LOGIC_SCRIPTING_SCRIPTING_H_
