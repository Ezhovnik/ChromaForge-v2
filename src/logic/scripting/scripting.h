#ifndef LOGIC_SCRIPTING_SCRIPTING_H_
#define LOGIC_SCRIPTING_SCRIPTING_H_

#include <string>
#include <filesystem>

#include "../../delegates.h"

class Engine;
class Content;
class Level;
class Block;
class Player;
class Item;
class BlocksController;
struct block_funcs_set;
struct item_funcs_set;
class LuaState;
class ContentIndices;
struct uidocscript;
class Inventory;
class UIDocument;
struct ContentPack;

namespace scripting {
    using int_array_consumer = std::function<void(const int[], size_t)>;

    extern Engine* engine;
    extern const Content* content;
    extern Level* level;
    extern BlocksController* blocks;
    extern const ContentIndices* indices;

    class Environment {
    private:
        int env;
    public:
        Environment(int env);
        ~Environment();

        int getId() const;
    };

    void initialize(Engine* engine);

    runnable create_runnable(int env, const std::string& src, const std::string& file="<string>");
    wstringconsumer create_wstring_consumer(int env, const std::string& src, const std::string& file="<string>");
    int_array_consumer create_int_array_consumer(int env, const std::string& src, const std::string& file="<string>");
    doubleconsumer create_number_consumer(int env, const std::string& src, const std::string& file="<string>");

    std::unique_ptr<Environment> create_environment(int parent=0);
    std::unique_ptr<Environment> create_pack_environment(const ContentPack& pack);
    std::unique_ptr<Environment> create_doc_environment(int parent, const std::string& name);

    void on_world_load(Level* level, BlocksController* blocks);
    void on_world_quit();
    void on_world_save();
    void load_world_script(int env, std::string prefix, std::filesystem::path file);

    void on_blocks_tick(const Block* block, int tps);
    void update_block(const Block* block, int x, int y, int z);
    void random_update_block(const Block* block, int x, int y, int z);
    void on_block_placed(Player* player, const Block* block, int x, int y, int z);
    void on_block_broken(Player* player, const Block* block, int x, int y, int z);
    bool on_block_interact(Player* player, const Block* block, int x, int y, int z);

    bool on_item_use_on_block(Player* player, const Item* item, int x, int y, int z);
    bool on_item_break_block(Player* player, const Item* item, int x, int y, int z);

    void load_block_script(int env, std::string prefix, std::filesystem::path file, block_funcs_set& funcsset);
    void load_item_script(int env, std::string prefix, std::filesystem::path file, item_funcs_set& funcsset);

    void on_ui_open(UIDocument* layout, Inventory* inventory);
    void on_ui_close(UIDocument* layout, Inventory* inventory);

    void load_layout_script(int env, std::string prefix, std::filesystem::path file, uidocscript& script);

    void close();
}

#endif // LOGIC_SCRIPTING_SCRIPTING_H_
