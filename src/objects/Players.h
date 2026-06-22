#pragma once

#include <memory>
#include <unordered_map>

#include <typedefs.h>
#include <interfaces/Serializable.h>

class Level;
class Player;

class Players : public Serializable {
public:
    static inline int64_t NONE = -1;
private:
    Level& level;
    std::unordered_map<int64_t, std::unique_ptr<Player>> players;

    void add(std::unique_ptr<Player> player);
public:
    Players(Level& level);

    Player* getPlayer(int64_t id) const;

    Player* create(int64_t id=NONE);
    void remove(int64_t id);

    void suspend(int64_t id);
    void resume(int64_t id);

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;

    auto begin() const {
        return players.begin();
    }

    auto end() const {
        return players.end();
    }

    size_t size() const {
        return players.size();
    }
};
