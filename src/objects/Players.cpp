#include <objects/Players.h>

#include <objects/Player.h>
#include <items/Inventories.h>
#include <world/Level.h>
#include <world/World.h>

inline constexpr glm::vec3 DEFAULT_SPAWNPOINT = {0, 256, 0}; ///< Точка появления игрока
inline constexpr float DEFAULT_PLAYER_SPEED = 4.0f; ///< Базовая скорость перемещения игрока
inline constexpr int DEFAULT_PLAYER_INVENTORY_SIZE = 40; ///< Размер инвентаря игрока (количество слотов)

Players::Players(Level& level) : level(level) {
}

void Players::add(std::unique_ptr<Player> player) {
    players[player->getId()] = std::move(player);
}

Player* Players::getPlayer(int64_t id) const {
    const auto& found = players.find(id);
    if (found == players.end()) {
        return nullptr;
    }
    return found->second.get();
}

Player* Players::create() {
    auto playerPtr = std::make_unique<Player>(
        level,
        level.getWorld()->getInfo().nextPlayerId++,
        "",
        DEFAULT_SPAWNPOINT,
        DEFAULT_PLAYER_SPEED,
        level.inventories->create(DEFAULT_PLAYER_INVENTORY_SIZE),
        0
    );
    auto player = playerPtr.get();
    add(std::move(playerPtr));

    level.inventories->store(player->getInventory());
    return player;
}

void Players::remove(int64_t id) {
    players.erase(id);
}

dv::value Players::serialize() const {
    auto root = dv::object();
    auto& list = root.list("players");

    for (const auto& [id, player] : players) {
        list.add(player->serialize());
    }
    return root;
}

void Players::deserialize(const dv::value& src) {
    players.clear();

    const auto& players = src["players"];
    for (auto& playerMap : players) {
        auto playerPtr = std::make_unique<Player>(
            level,
            0,
            "",
            DEFAULT_SPAWNPOINT,
            DEFAULT_PLAYER_SPEED,
            level.inventories->create(DEFAULT_PLAYER_INVENTORY_SIZE),
            0
        );
        auto player = playerPtr.get();
        player->deserialize(playerMap);
        add(std::move(playerPtr));
        auto& inventory = player->getInventory();
        if (inventory->getId() == 0) {
            inventory->setId(level.getWorld()->getNextInventoryId());
        }
        level.inventories->store(player->getInventory());
    }
}
