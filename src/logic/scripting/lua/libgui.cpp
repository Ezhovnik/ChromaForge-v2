#include "api_lua.h"
#include "../../../engine.h"
#include "../../../assets/Assets.h"
#include "../../../graphics/ui/elements/UINode.h"
#include "../../../frontend/UiDocument.h"
#include "../../../util/stringutil.h"
#include "../../../graphics/ui/gui_util.h"
#include "../../../frontend/locale/langs.h"
#include "../../../graphics/ui/elements/control/Button.h"
#include "../../../graphics/ui/elements/control/CheckBox.h"
#include "../../../graphics/ui/elements/control/TextBox.h"
#include "../../../graphics/ui/elements/control/TrackBar.h"
#include "../../../graphics/ui/elements/layout/Panel.h"
#include "../../../graphics/ui/elements/layout/Menu.h"
#include "../../../items/Inventories.h"
#include "../../../graphics/ui/elements/display/InventoryView.h"
#include "../../../world/Level.h"
#include "../../../graphics/ui/elements/display/Image.h"

struct DocumentNode {
    UIDocument* document;
    std::shared_ptr<gui::UINode> node;
};

static DocumentNode getDocumentNode(lua::State*, const std::string& name, const std::string& nodeName) {
    auto doc = scripting::engine->getAssets()->getLayout(name);
    if (doc == nullptr) {
        throw std::runtime_error("Document '" + name + "' not found");
    }
    auto node = doc->get(nodeName);
    if (node == nullptr) {
        throw std::runtime_error("Document '" + name + "' has no element with id '" + nodeName + "'");
    }
    return {doc, node};
}

static DocumentNode getDocumentNode(lua::State* L, int idx=1) {
    lua::getfield(L, "docname", idx);
    lua::getfield(L, "name", idx);
    auto docname = lua::require_string(L, -2);
    auto name = lua::require_string(L, -1);
    auto node = getDocumentNode(L, docname, name);
    lua::pop(L, 2);
    return node;
}

static int l_menu_back(lua::State* L) {
    auto node = getDocumentNode(L);
    auto menu = dynamic_cast<gui::Menu*>(node.node.get());
    menu->back();
    return 0;
}

static int l_menu_reset(lua::State* L) {
    auto node = getDocumentNode(L);
    auto menu = dynamic_cast<gui::Menu*>(node.node.get());
    menu->reset();
    return 0;
}

static int l_container_add(lua::State* L) {
    auto docnode = getDocumentNode(L);
    auto node = dynamic_cast<gui::Container*>(docnode.node.get());
    auto xmlsrc = lua::require_string(L, 2);
    try {
        auto subnode = guiutil::create(xmlsrc, docnode.document->getEnvironment());
        node->add(subnode);
        gui::UINode::getIndices(subnode, docnode.document->getMapWriteable());
    } catch (const std::exception& err) {
        throw std::runtime_error(err.what());
    }
    return 0;
}

static int l_container_clear(lua::State* L) {
    auto node = getDocumentNode(L, 1);
    if (auto container = std::dynamic_pointer_cast<gui::Container>(node.node)) container->clear();
    return 0;
}

static int l_node_destruct(lua::State* L) {
    auto docnode = getDocumentNode(L);
    auto node = std::dynamic_pointer_cast<gui::Container>(docnode.node);
    scripting::engine->getGUI()->postRunnable([node]() {
        auto parent = node->getParent();
        if (auto container = dynamic_cast<gui::Container*>(parent)) {
            container->remove(node);
        }
    });
    return 0;
}

static int l_container_set_interval(lua::State* L) {
    auto node = getDocumentNode(L, 1);
    auto interval = lua::tointeger(L, 2) / 1000.0f;
    if (auto container = std::dynamic_pointer_cast<gui::Container>(node.node)) {
        lua::pushvalue(L, 3);
        auto runnable = lua::create_runnable(L);
        container->listenInterval(interval, runnable);
    }
    return 0;
}

static int l_move_into(lua::State* L) {
    auto node = getDocumentNode(L, 1);
    auto dest = getDocumentNode(L, 2);
    gui::UINode::moveInto(node.node, std::dynamic_pointer_cast<gui::Container>(dest.node));
    return 0;
}

static int l_textbox_paste(lua::State* L) {
    auto node = getDocumentNode(L);
    auto box = dynamic_cast<gui::TextBox*>(node.node.get());
    auto text = lua::require_string(L, 2);
    box->paste(util::str2wstr_utf8(text));
    return 0;
}

static int p_get_inventory(gui::UINode* node, lua::State* L) {
    if (auto inventory = dynamic_cast<gui::InventoryView*>(node)) {
        auto inv = inventory->getInventory();
        return lua::pushinteger(L, inv ? inv->getId() : 0);
    }
    return 0;
}

static int p_get_reset(gui::UINode* node, lua::State* L) {
    if (dynamic_cast<gui::Menu*>(node)) {
        return lua::pushcfunction(L, lua::wrap<l_menu_reset>);
    }
    return 0;
}

static int p_get_back(gui::UINode* node, lua::State* L) {
    if (dynamic_cast<gui::Menu*>(node)) {
        return lua::pushcfunction(L, lua::wrap<l_menu_back>);
    }
    return 0;
}

static int p_get_page(gui::UINode* node, lua::State* L) {
    if (auto menu = dynamic_cast<gui::Menu*>(node)) {
        return lua::pushstring(L, menu->getCurrent().name);
    }
    return 0;
}

static int p_is_checked(gui::UINode* node, lua::State* L) {
    if (auto box = dynamic_cast<gui::CheckBox*>(node)) {
        return lua::pushboolean(L, box->isChecked());
    } else if (auto box = dynamic_cast<gui::FullCheckBox*>(node)) {
        return lua::pushboolean(L, box->isChecked());
    }
    return 0;
}

static int p_get_value(gui::UINode* node, lua::State* L) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return lua::pushnumber(L, bar->getValue());
    }
    return 0;
}

static int p_get_min(gui::UINode* node, lua::State* L) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return lua::pushnumber(L, bar->getMin());
    }
    return 0;
}

static int p_get_max(gui::UINode* node, lua::State* L) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return lua::pushnumber(L, bar->getMax());
    }
    return 0;
}

static int p_get_step(gui::UINode* node, lua::State* L) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return lua::pushnumber(L, bar->getStep());
    }
    return 0;
}

static int p_get_track_width(gui::UINode* node, lua::State* L) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return lua::pushnumber(L, bar->getTrackWidth());
    }
    return 0;
}

static int p_get_track_color(gui::UINode* node, lua::State* L) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return lua::pushcolor_arr(L, bar->getTrackColor());
    }
    return 0;
}

static int p_is_valid(gui::UINode* node, lua::State* L) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        return lua::pushboolean(L, box->validate());
    }
    return 0;
}

static int p_get_placeholder(gui::UINode* node, lua::State* L) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        return lua::pushwstring(L, box->getPlaceholder());
    }
    return 0;
}

static int p_get_text(gui::UINode* node, lua::State* L) {
    if (auto button = dynamic_cast<gui::Button*>(node)) {
        return lua::pushwstring(L, button->getText());
    } else if (auto label = dynamic_cast<gui::Label*>(node)) {
        return lua::pushwstring(L, label->getText());
    } else if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        return lua::pushwstring(L, box->getText());
    }
    return 0;
}

static int p_get_add(gui::UINode* node, lua::State* L) {
    if (dynamic_cast<gui::Container*>(node)) {
        return lua::pushcfunction(L, lua::wrap<l_container_add>);
    }
    return 0;
}

static int p_get_clear(gui::UINode* node, lua::State* L) {
    if (dynamic_cast<gui::Container*>(node)) {
        return lua::pushcfunction(L, lua::wrap<l_container_clear>);
    }
    return 0;
}

static int p_get_color(gui::UINode* node, lua::State* L) {
    return lua::pushcolor_arr(L, node->getColor());
}
static int p_get_hover_color(gui::UINode* node, lua::State* L) {
    return lua::pushcolor_arr(L, node->getHoverColor());
}
static int p_get_pressed_color(gui::UINode* node, lua::State* L) {
    return lua::pushcolor_arr(L, node->getPressedColor());
}
static int p_get_pos(gui::UINode* node, lua::State* L) {
    return lua::pushvec2_arr(L, node->getPos());
}
static int p_get_size(gui::UINode* node, lua::State* L) {
    return lua::pushvec2_arr(L, node->getSize());
}
static int p_is_interactive(gui::UINode* node, lua::State* L) {
    return lua::pushboolean(L, node->isInteractive());
}
static int p_is_visible(gui::UINode* node, lua::State* L) {
    return lua::pushboolean(L, node->isVisible());
}
static int p_is_enabled(gui::UINode* node, lua::State* L) {
    return lua::pushboolean(L, node->isEnabled());
}
static int p_move_into(gui::UINode*, lua::State* L) {
    return lua::pushcfunction(L, lua::wrap<l_move_into>);
}

static int p_get_wpos(gui::UINode* node, lua::State* L) {
    return lua::pushvec2_arr(L, node->calcPos());
}

static int p_get_editable(gui::UINode* node, lua::State* L) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        return lua::pushboolean(L, box->isEditable());
    }
    return 0;
}

static int p_get_focused(gui::UINode* node, lua::State* L) {
    return lua::pushboolean(L, node->isFocused());
}

static int p_get_paste(gui::UINode* node, lua::State* L) {
    if (dynamic_cast<gui::TextBox*>(node)) {
        return lua::pushcfunction(L, lua::wrap<l_textbox_paste>);
    }
    return 0;
}

static int p_get_caret(gui::UINode* node, lua::State* L) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        return lua::pushinteger(L, static_cast<integer_t>(box->getCaret()));
    }
    return 0;
}

static int p_get_destruct(gui::UINode*, lua::State* L) {
    return lua::pushcfunction(L, lua::wrap<l_node_destruct>);
}

static int p_get_src(gui::UINode* node, lua::State* L) {
    if (auto image = dynamic_cast<gui::Image*>(node)) {
        return lua::pushstring(L, image->getTexture());
    }
    return 0;
}

static int p_get_tooltip(gui::UINode* node, lua::State* L) {
    return lua::pushwstring(L, node->getTooltip());
}

static int p_get_tooltip_delay(gui::UINode* node, lua::State* L) {
    return lua::pushnumber(L, node->getTooltipDelay());
}

static int p_set_interval(gui::UINode* node, lua::State* L) {
    if (dynamic_cast<gui::Container*>(node)) {
        return lua::pushcfunction(L, lua::wrap<l_container_set_interval>);
    }
    return 0;
}

static int l_gui_getattr(lua::State* L) {
    auto docname = lua::require_string(L, 1);
    auto element = lua::require_string(L, 2);
    auto attr = lua::require_string(L, 3);
    auto docnode = getDocumentNode(L, docname, element);
    auto node = docnode.node;

    static const std::unordered_map<std::string_view, std::function<int(gui::UINode*, lua::State*)>> getters {
        {"color", p_get_color},
        {"hoverColor", p_get_hover_color},
        {"pressedColor", p_get_pressed_color},
        {"pos", p_get_pos},
        {"wpos", p_get_wpos},
        {"size", p_get_size},
        {"interactive", p_is_interactive},
        {"visible", p_is_visible},
        {"enabled", p_is_enabled},
        {"move_into", p_move_into},
        {"add", p_get_add},
        {"clear", p_get_clear},
        {"placeholder", p_get_placeholder},
        {"valid", p_is_valid},
        {"text", p_get_text},
        {"editable", p_get_editable},
        {"value", p_get_value},
        {"min", p_get_min},
        {"max", p_get_max},
        {"step", p_get_step},
        {"trackWidth", p_get_track_width},
        {"trackColor", p_get_track_color},
        {"checked", p_is_checked},
        {"page", p_get_page},
        {"back", p_get_back},
        {"reset", p_get_reset},
        {"inventory", p_get_inventory},
        {"focused", p_get_focused},
        {"paste", p_get_paste},
        {"caret", p_get_caret},
        {"src", p_get_src},
        {"tooltip", p_get_tooltip},
        {"tooltipDelay", p_get_tooltip_delay},
        {"setInterval", p_set_interval},
        {"destruct", p_get_destruct}
    };
    auto func = getters.find(attr);
    if (func != getters.end()) return func->second(node.get(), L);
    return 0;
}

static void p_set_color(gui::UINode* node, lua::State* L, int idx) {
    node->setColor(lua::tocolor(L, idx));
}
static void p_set_hover_color(gui::UINode* node, lua::State* L, int idx) {
    node->setHoverColor(lua::tocolor(L, idx));
}
static void p_set_pressed_color(gui::UINode* node, lua::State* L, int idx) {
    node->setPressedColor(lua::tocolor(L, idx));
}
static void p_set_pos(gui::UINode* node, lua::State* L, int idx) {
    node->setPos(lua::tovec2(L, idx));
}
static void p_set_size(gui::UINode* node, lua::State* L, int idx) {
    node->setSize(lua::tovec2(L, idx));
}
static void p_set_interactive(gui::UINode* node, lua::State* L, int idx) {
    node->setInteractive(lua::toboolean(L, idx));
}
static void p_set_visible(gui::UINode* node, lua::State* L, int idx) {
    node->setVisible(lua::toboolean(L, idx));
}
static void p_set_enabled(gui::UINode* node, lua::State* L, int idx) {
    node->setEnabled(lua::toboolean(L, idx));
}
static void p_set_placeholder(gui::UINode* node, lua::State* L, int idx) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        box->setPlaceholder(lua::require_wstring(L, idx));
    }
}
static void p_set_text(gui::UINode* node, lua::State* L, int idx) {
    if (auto label = dynamic_cast<gui::Label*>(node)) {
        label->setText(lua::require_wstring(L, idx));
    } else if (auto button = dynamic_cast<gui::Button*>(node)) {
        button->setText(lua::require_wstring(L, idx));
    } else if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        box->setText(lua::require_wstring(L, idx));
    }
}
static void p_set_value(gui::UINode* node, lua::State* L, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setValue(lua::tonumber(L, idx));
    }
}
static void p_set_min(gui::UINode* node, lua::State* L, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setMin(lua::tonumber(L, idx));
    }
}
static void p_set_max(gui::UINode* node, lua::State* L, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setMax(lua::tonumber(L, idx));
    }
}
static void p_set_step(gui::UINode* node, lua::State* L, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setStep(lua::tonumber(L, idx));
    }
}
static void p_set_track_width(gui::UINode* node, lua::State* L, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setTrackWidth(lua::tointeger(L, idx));
    }
}
static void p_set_track_color(gui::UINode* node, lua::State* L, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setTrackColor(lua::tocolor(L, idx));
    }
}
static void p_set_checked(gui::UINode* node, lua::State* L, int idx) {
    if (auto box = dynamic_cast<gui::CheckBox*>(node)) {
        box->setChecked(lua::toboolean(L, idx));
    } else if (auto box = dynamic_cast<gui::FullCheckBox*>(node)) {
        box->setChecked(lua::toboolean(L, idx));
    }
}
static void p_set_page(gui::UINode* node, lua::State* L, int idx) {
    if (auto menu = dynamic_cast<gui::Menu*>(node)) {
        menu->setPage(lua::require_string(L, idx));
    }
}
static void p_set_inventory(gui::UINode* node, lua::State* L, int idx) {
    if (auto view = dynamic_cast<gui::InventoryView*>(node)) {
        auto inventory = scripting::level->inventories->get(lua::tointeger(L, idx));
        if (inventory == nullptr) {
            view->unbind();
        } else {
            view->bind(inventory, scripting::content);
        }
    }
}

static void p_set_editable(gui::UINode* node, lua::State* L, int idx) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        box->setEditable(lua::toboolean(L, idx));
    }
}

static void p_set_focused(const std::shared_ptr<gui::UINode>& node, lua::State* L, int idx) {
    if (lua::toboolean(L, idx) && !node->isFocused()) {
        scripting::engine->getGUI()->setFocus(node);
    } else if (node->isFocused()){
        node->defocus();
    }
}

static void p_set_caret(gui::UINode* node, lua::State* L, int idx) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        box->setCaret(static_cast<ptrdiff_t>(lua::tointeger(L, idx)));
    }
}

static void p_set_src(gui::UINode* node, lua::State* L, int idx) {
    if (auto image = dynamic_cast<gui::Image*>(node)) {
        image->setTexture(lua::require_string(L, idx));
    }
}

static void p_set_wpos(gui::UINode* node, lua::State* L, int idx) {
    node->setPos(lua::tovec2(L, idx) - node->calcPos());
}

static void p_set_tooltip(gui::UINode* node, lua::State* L, int idx) {
    node->setTooltip(util::str2wstr_utf8(lua::require_string(L, idx)));
}

static void p_set_tooltip_delay(gui::UINode* node, lua::State* L, int idx) {
    node->setTooltipDelay(lua::tonumber(L, idx));
}

static int l_gui_setattr(lua::State* L) {
    auto docname = lua::require_string(L, 1);
    auto element = lua::require_string(L, 2);
    auto attr = lua::require_string(L, 3);

    auto docnode = getDocumentNode(L, docname, element);
    auto node = docnode.node;

    static const std::unordered_map<std::string_view, std::function<void(gui::UINode*, lua::State*, int)>> setters {
        {"color", p_set_color},
        {"hoverColor", p_set_hover_color},
        {"pressedColor", p_set_pressed_color},
        {"tooltip", p_set_tooltip},
        {"tooltipDelay", p_set_tooltip_delay},
        {"pos", p_set_pos},
        {"wpos", p_set_wpos},
        {"size", p_set_size},
        {"interactive", p_set_interactive},
        {"visible", p_set_visible},
        {"enabled", p_set_enabled},
        {"placeholder", p_set_placeholder},
        {"text", p_set_text},
        {"editable", p_set_editable},
        {"value", p_set_value},
        {"min", p_set_min},
        {"max", p_set_max},
        {"step", p_set_step},
        {"trackWidth", p_set_track_width},
        {"trackColor", p_set_track_color},
        {"checked", p_set_checked},
        {"page", p_set_page},
        {"inventory", p_set_inventory},
        {"caret", p_set_caret},
        {"src", p_set_src}
    };
    auto func = setters.find(attr);
    if (func != setters.end()) func->second(node.get(), L, 4);

    static const std::unordered_map<std::string_view, std::function<void(std::shared_ptr<gui::UINode>, lua::State*, int)>> setters2 {
        {"focused", p_set_focused},
    };
    auto func2 = setters2.find(attr);
    if (func2 != setters2.end()) func2->second(node, L, 4);
    return 0;
}

static int l_gui_get_env(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto doc = scripting::engine->getAssets()->getLayout(name);
    if (doc == nullptr) {
        throw std::runtime_error("Document '" + std::string(name) + "' not found");
    }
    lua::getglobal(L, lua::env_name(*doc->getEnvironment()));
    return 1;
}

static int l_gui_str(lua::State* L) {
    auto text = lua::require_wstring(L, 1);
    if (!lua::isnoneornil(L, 2)) {
        auto context = lua::require_wstring(L, 2);
        lua::pushwstring(L, langs::get(text, context));
    } else {
        lua::pushwstring(L, langs::get(text));
    }
    return 1;
}

static int l_gui_reindex(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto doc = scripting::engine->getAssets()->getLayout(name);
    if (doc == nullptr) {
        throw std::runtime_error("Document '" + std::string(name) + "' not found");
    }
    doc->rebuildIndices();
    return 0;
}

static int l_gui_get_locales_info(lua::State* L) {
    auto& locales = langs::locales_info;
    lua::createtable(L, 0, locales.size());
    for (auto& entry : locales) {
        lua::createtable(L, 0, 1);
        lua::pushstring(L, entry.second.name);
        lua::setfield(L, "name");
        lua::setfield(L, entry.first);
    }
    return 1;
}

static int l_gui_get_viewport(lua::State* L) {
    return lua::pushvec2_arr(L, scripting::engine->getGUI()->getContainer()->getSize());;
}

const luaL_Reg guilib [] = {
    {"get_viewport", lua::wrap<l_gui_get_viewport>},
    {"getattr", lua::wrap<l_gui_getattr>},
    {"setattr", lua::wrap<l_gui_setattr>},
    {"get_env", lua::wrap<l_gui_get_env>},
    {"str", lua::wrap<l_gui_str>},
    {"get_locales_info", lua::wrap<l_gui_get_locales_info>},
    {"__reindex", lua::wrap<l_gui_reindex>},
    {NULL, NULL}
};
