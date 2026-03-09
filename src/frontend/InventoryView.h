#ifndef FRONTEND_INVENTORY_VIEW_H_
#define FRONTEND_INVENTORY_VIEW_H_

#include <vector>
#include <functional>

#include <glm/glm.hpp>

#include "../frontend/gui/UINode.h"
#include "../frontend/gui/panels.h"
#include "../frontend/gui/controls.h"
#include "../items/ItemStack.h"
#include "../typedefs.h"
#include "../constants.h"

class Assets;
class GfxContext;
class Content;
class ContentIndices;
class LevelFrontend;
class Inventory;

using itemsharefunc = std::function<void(ItemStack&)>;
using slotcallback = std::function<void(ItemStack&, ItemStack&)>;

namespace scripting {
    class Environment;
}

namespace gui {
    class UiXmlReader;
}

class InventoryInteraction {
private:
    ItemStack grabbedItem;
public:
    InventoryInteraction() = default;

    ItemStack& getGrabbedItem() {
        return grabbedItem;
    }
};

struct SlotLayout {
    int index;
    int padding = 0;
    glm::vec2 position;
    bool background;
    bool itemSource;
    itemsharefunc shareFunc;
    slotcallback rightClick;

    SlotLayout(
        int index,
        glm::vec2 position, 
        bool background,
        bool itemSource,
        itemsharefunc shareFunc,
        slotcallback rightClick
    );
};

class SlotView : public gui::UINode {
private:
    LevelFrontend* frontend = nullptr;
    InventoryInteraction* interaction = nullptr;
    const Content* content;
    bool highlighted = false;

    SlotLayout layout;
    ItemStack* bound = nullptr;
public:
    SlotView(SlotLayout layout);

    virtual void draw(const GfxContext* parent_context, Assets* assets) override;

    void setHighlighted(bool flag);
    bool isHighlighted() const;

    virtual void clicked(gui::GUI*, int) override;
    virtual void focus(gui::GUI*) override;

    void bind(
        ItemStack& stack,
        LevelFrontend* frontend, 
        InventoryInteraction* interaction
    );

    const SlotLayout& getLayout() const;
};

class InventoryView : public gui::Container {
private:
    const Content* content;
    const ContentIndices* indices;

    std::shared_ptr<Inventory> inventory;
    LevelFrontend* frontend = nullptr;
    InventoryInteraction* interaction = nullptr;

    std::vector<SlotView*> slots;
    glm::vec2 origin {};
public:
    InventoryView();
    virtual ~InventoryView();

    void setInventory(std::shared_ptr<Inventory> inventory);
    std::shared_ptr<Inventory> getInventory() const;

    virtual void setCoord(glm::vec2 coord) override;

    void setOrigin(glm::vec2 origin);
    glm::vec2 getOrigin() const;

    void setSelected(int index);

    void bind(
        std::shared_ptr<Inventory> inventory,
        LevelFrontend* frontend, 
        InventoryInteraction* interaction
    );

    std::shared_ptr<SlotView> addSlot(SlotLayout layout);

    static std::shared_ptr<InventoryView> readXML(
        const std::string& src,
        const std::string& file,
        const scripting::Environment& env
    );

    static void createReaders(gui::UiXmlReader& reader);

    static const int SLOT_INTERVAL = 4;
    static const int SLOT_SIZE = ITEM_ICON_SIZE;
};

class InventoryBuilder {
private:
    std::shared_ptr<InventoryView> view;
public:
    InventoryBuilder();

    void addGrid(
        int cols, int count, 
        glm::vec2 coord, 
        int padding,
        bool addpanel,
        SlotLayout slotLayout
    );

    void add(SlotLayout slotLayout);
    std::shared_ptr<InventoryView> build();
};

#endif // FRONTEND_INVENTORY_VIEW_H_
