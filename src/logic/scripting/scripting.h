#ifndef LOGIC_SCRIPTING_SCRIPTING_H_
#define LOGIC_SCRIPTING_SCRIPTING_H_

#include <string>
#include <filesystem>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "delegates.h"
#include "scripting_functional.h"
#include "typedefs.h"
#include "data/dynamic.h"

class Engine;
class Content;
class Level;
class Block;
class Player;
struct Item;
class BlocksController;
struct block_funcs_set;
struct item_funcs_set;
struct world_funcs_set;
struct UserComponent;
class ContentIndices;
struct uidocscript;
class Inventory;
class UIDocument;
struct ContentPack;
class LevelController;
struct Entity;
class Entt_Entity;

namespace scripting {
    extern Engine* engine;
    extern const Content* content;
    extern const ContentIndices* indices;
    extern Level* level;
    extern BlocksController* blocks;
    extern LevelController* controller;

    void initialize(Engine* engine);

    bool register_event(int env, const std::string& name, const std::string& id);
    int get_values_on_stack();

    scriptenv get_root_environment();
    scriptenv create_pack_environment(const ContentPack& pack);
    scriptenv create_doc_environment(const scriptenv& parent, const std::string& name);

    void process_post_runnables();

    void on_world_load(LevelController* controller);
    void on_world_spark();
    void on_world_save();
    void on_world_quit();
    void on_blocks_spark(const Block* block, int tps);
    void update_block(const Block* block, int x, int y, int z);
    void random_update_block(const Block* block, int x, int y, int z);
    void on_block_placed(Player* player, const Block* block, int x, int y, int z);
    void on_block_broken(Player* player, const Block* block, int x, int y, int z);
    bool on_block_interact(Player* player, const Block* block, glm::ivec3 pos);

    bool on_item_use(Player* player, const Item* item);
    bool on_item_use_on_block(Player* player, const Item* item, int x, int y, int z);
    bool on_item_break_block(Player* player, const Item* item, int x, int y, int z);

    dynamic::Value get_component_value(const scriptenv& env, const std::string& name);

    void on_entity_spawn(
        const Entity& def,
        entityid_t eid,
        const std::vector<std::unique_ptr<UserComponent>>& components,
        dynamic::Map_sptr args,
        dynamic::Map_sptr saved
    );
    void on_entity_despawn(const Entt_Entity& entity);
    void on_entity_grounded(const Entt_Entity& entity, float force);
    void on_entity_fall(const Entt_Entity& entity);
    void on_entity_save(const Entt_Entity& entity);
    void on_entities_update();
    void on_entities_render();
    void on_sensor_enter(const Entt_Entity& entity, size_t index, entityid_t oid);
    void on_sensor_exit(const Entt_Entity& entity, size_t index, entityid_t oid);
    void on_aim_on(const Entt_Entity& entity, Player* player);
    void on_aim_off(const Entt_Entity& entity, Player* player);
    void on_attacked(const Entt_Entity& entity, Player* player, entityid_t attacker);
    void on_entity_used(const Entt_Entity& entity, Player* player);

    void on_ui_open(
        UIDocument* layout, 
        std::vector<dynamic::Value> args
    );
    void on_ui_progress(UIDocument* layout, int workDone, int totalWork);
    void on_ui_close(UIDocument* layout, Inventory* inventory);

    void load_block_script(
        const scriptenv& env,
        const std::string& prefix,
        const std::filesystem::path& file,
        block_funcs_set& funcsset
    );
    void load_item_script(
        const scriptenv& env,
        const std::string& prefix,
        const std::filesystem::path& file,
        item_funcs_set& funcsset
    );
    void load_world_script(
        const scriptenv& env,
        const std::string& packid,
        const std::filesystem::path& file,
        world_funcs_set& funcsset
    );
    void load_layout_script(
        const scriptenv& env,
        const std::string& prefix,
        const std::filesystem::path& file,
        uidocscript& script
    );
    void load_entity_component(
        const std::string& name,
        const std::filesystem::path& file
    );

    void close();
}

#endif // LOGIC_SCRIPTING_SCRIPTING_H_
