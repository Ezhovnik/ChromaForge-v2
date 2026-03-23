#ifndef FRONTEND_INVENTORY_VIEW_H_
#define FRONTEND_INVENTORY_VIEW_H_

#include <vector>
#include <functional>

#include <glm/glm.hpp>

#include "../graphics/ui/elements/UINode.h"
#include "../graphics/ui/elements/layout/Container.h"
#include "../constants.h"
#include "../typedefs.h"
#include "../constants.h"

class Assets;
class GfxContext;
class Content;
class ContentIndices;
class LevelFrontend;
class Inventory;
class ItemStack;

using slotcallback = std::function<void(uint, ItemStack&)>;

namespace gui {
    class UIXmlReader;
}

struct SlotLayout {
    int index;
    int padding = 0;
    glm::vec2 position;
    bool background;
    bool itemSource;
    slotcallback updateFunc;
    slotcallback shareFunc;
    slotcallback rightClick;

    SlotLayout(
        int index,
        glm::vec2 position, 
        bool background,
        bool itemSource,
        slotcallback updateFunc,
        slotcallback shareFunc,
        slotcallback rightClick
    );
};

class SlotView : public gui::UINode {
private:
    const Content* content;
    bool highlighted = false;

    int64_t inventoryId = 0;

    SlotLayout layout;
    ItemStack* bound = nullptr;
public:
    SlotView(SlotLayout layout);

    virtual void draw(const GfxContext* parent_context, Assets* assets) override;

    void setHighlighted(bool flag);
    bool isHighlighted() const;

    virtual void clicked(gui::GUI*, mousecode) override;
    virtual void onFocus(gui::GUI*) override;

    void bind(
        int64_t inventoryId,
        ItemStack& stack,
        const Content* content
    );

    ItemStack& getStack();
    const SlotLayout& getLayout() const;

    static inline std::string EXCHANGE_SLOT_NAME = "exchange-slot";
};

class InventoryView : public gui::Container {
private:
    const Content* content;

    std::shared_ptr<Inventory> inventory;

    std::vector<SlotView*> slots;
    glm::vec2 origin {};
public:
    InventoryView();
    virtual ~InventoryView();

    std::shared_ptr<Inventory> getInventory() const;

    virtual void setPos(glm::vec2 pos) override;

    void setOrigin(glm::vec2 origin);
    glm::vec2 getOrigin() const;

    void setSelected(int index);

    void bind(
        std::shared_ptr<Inventory> inventory,
        const Content* content
    );

    void unbind();

    std::shared_ptr<SlotView> addSlot(SlotLayout layout);

    size_t getSlotsCount() const;

    static void createReaders(gui::UIXmlReader& reader);

    static const int SLOT_INTERVAL = 4;
    static const int SLOT_SIZE = ITEM_ICON_SIZE;
};

class InventoryBuilder {
private:
    std::shared_ptr<InventoryView> view;
public:
    InventoryBuilder();

    void addGrid(
        int cols, 
        int count, 
        glm::vec2 pos, 
        int padding,
        bool addpanel,
        SlotLayout slotLayout
    );

    void add(SlotLayout slotLayout);
    std::shared_ptr<InventoryView> build();
};

#endif // FRONTEND_INVENTORY_VIEW_H_
