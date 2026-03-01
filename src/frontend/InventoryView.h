#ifndef FRONTEND_INVENTORY_VIEW_H_
#define FRONTEND_INVENTORY_VIEW_H_

#include <vector>
#include <functional>

#include <glm/glm.hpp>

#include "../typedefs.h"

class Player;
class Assets;
class GfxContext;
class ContentIndices;
class LevelFrontend;
class Content;

typedef std::function<void(itemid_t)> slotconsumer;

class InventoryView {
    const Content* content;
    slotconsumer consumer = nullptr;
    const ContentIndices* indices;
    std::vector<itemid_t> items;
    LevelFrontend* levelFrontend;

    int scroll = 0;
    int columns;
    uint iconSize = 48;
    uint interval = 4;
    glm::ivec2 padding {interval, interval};
    glm::ivec2 position {0, 0};
public:
    InventoryView(int columns, const Content* content, std::vector<itemid_t> items, LevelFrontend* levelFrontend);
    virtual ~InventoryView();

    virtual void activateAndDraw(const GfxContext* ctx);

    void setPosition(int x, int y);
    int getWidth() const;
    int getHeight() const;
    void setSlotConsumer(slotconsumer consumer);
    void setItems(std::vector<itemid_t> items);
};

#endif // FRONTEND_INVENTORY_VIEW_H_
