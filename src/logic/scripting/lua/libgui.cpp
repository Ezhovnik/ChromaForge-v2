#include "lua_commons.h"
#include "api_lua.h"
#include "../scripting.h"
#include "lua_util.h"
#include "../../../engine.h"
#include "../../../assets/Assets.h"
#include "../../../graphics/ui/elements/UINode.h"
#include "../../../frontend/UiDocument.h"
#include "../../../util/stringutil.h"
#include "../../../graphics/ui/gui_util.h"
#include "LuaState.h"
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

struct DocumentNode {
    UIDocument* document;
    std::shared_ptr<gui::UINode> node;
};

namespace scripting {
    extern lua::LuaState* state;
}

static DocumentNode getDocumentNode(lua_State*, const std::string& name, const std::string& nodeName) {
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

static DocumentNode getDocumentNode(lua_State* L, int idx=1) {
    lua_getfield(L, idx, "docname");
    lua_getfield(L, idx, "name");
    auto docname = lua_tostring(L, -2);
    auto name = lua_tostring(L, -1);
    auto node = getDocumentNode(L, docname, name);
    lua_pop(L, 2);
    return node;
}

static int l_menu_back(lua_State* L) {
    auto node = getDocumentNode(L);
    auto menu = dynamic_cast<gui::Menu*>(node.node.get());
    menu->back();
    return 0;
}

static int l_menu_reset(lua_State* L) {
    auto node = getDocumentNode(L);
    auto menu = dynamic_cast<gui::Menu*>(node.node.get());
    menu->reset();
    return 0;
}

static int l_container_add(lua_State* L) {
    auto docnode = getDocumentNode(L);
    auto node = dynamic_cast<gui::Container*>(docnode.node.get());
    auto xmlsrc = lua_tostring(L, 2);
    try {
        auto subnode = guiutil::create(xmlsrc, docnode.document->getEnvironment());
        node->add(subnode);
        gui::UINode::getIndices(subnode, docnode.document->getMapWriteable());
    } catch (const std::exception& err) {
        throw std::runtime_error(err.what());
    }
    return 0;
}

static int l_container_clear(lua_State* L) {
    auto node = getDocumentNode(L, 1);
    if (auto container = std::dynamic_pointer_cast<gui::Container>(node.node)) container->clear();
    return 0;
}

static int l_uinode_move_into(lua_State* L) {
    auto node = getDocumentNode(L, 1);
    auto dest = getDocumentNode(L, 2);
    gui::UINode::moveInto(node.node, std::dynamic_pointer_cast<gui::Container>(dest.node));
    return 0;
}

static int l_textbox_paste(lua_State* L) {
    auto node = getDocumentNode(L);
    auto box = dynamic_cast<gui::TextBox*>(node.node.get());
    auto text = lua_tostring(L, 2);
    box->paste(util::str2wstr_utf8(text));
    return 0;
}

static int p_get_inventory(gui::UINode* node) {
    if (auto inventory = dynamic_cast<gui::InventoryView*>(node)) {
        auto inv = inventory->getInventory();
        return scripting::state->pushinteger(inv ? inv->getId() : 0);
    }
    return 0;
}

static int p_get_reset(gui::UINode* node) {
    if (dynamic_cast<gui::Menu*>(node)) {
        return scripting::state->pushcfunction(l_menu_reset);
    }
    return 0;
}

static int p_get_back(gui::UINode* node) {
    if (dynamic_cast<gui::Menu*>(node)) {
        return scripting::state->pushcfunction(l_menu_back);
    }
    return 0;
}

static int p_get_page(gui::UINode* node) {
    if (auto menu = dynamic_cast<gui::Menu*>(node)) {
        return scripting::state->pushstring(menu->getCurrent().name);
    }
    return 0;
}

static int p_is_checked(gui::UINode* node) {
    if (auto box = dynamic_cast<gui::CheckBox*>(node)) {
        return scripting::state->pushboolean(box->isChecked());
    } else if (auto box = dynamic_cast<gui::FullCheckBox*>(node)) {
        return scripting::state->pushboolean(box->isChecked());
    }
    return 0;
}

static int p_get_value(gui::UINode* node) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return scripting::state->pushnumber(bar->getValue());
    }
    return 0;
}

static int p_get_min(gui::UINode* node) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return scripting::state->pushnumber(bar->getMin());
    }
    return 0;
}

static int p_get_max(gui::UINode* node) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return scripting::state->pushnumber(bar->getMax());
    }
    return 0;
}

static int p_get_step(gui::UINode* node) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return scripting::state->pushnumber(bar->getStep());
    }
    return 0;
}

static int p_get_track_width(gui::UINode* node) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return scripting::state->pushnumber(bar->getTrackWidth());
    }
    return 0;
}

static int p_get_track_color(gui::UINode* node) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        return lua::pushcolor_arr(scripting::state->getLua(), bar->getTrackColor());
    }
    return 0;
}

static int p_is_valid(gui::UINode* node) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        return scripting::state->pushboolean(box->validate());
    }
    return 0;
}

static int p_get_placeholder(gui::UINode* node) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        return scripting::state->pushstring(util::wstr2str_utf8(box->getPlaceholder()));
    }
    return 0;
}

static int p_get_text(gui::UINode* node) {
    if (auto button = dynamic_cast<gui::Button*>(node)) {
        return scripting::state->pushstring(util::wstr2str_utf8(button->getText()));
    } else if (auto label = dynamic_cast<gui::Label*>(node)) {
        return scripting::state->pushstring(util::wstr2str_utf8(label->getText()));
    } else if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        return scripting::state->pushstring(util::wstr2str_utf8(box->getText()));
    }
    return 0;
}

static int p_get_add(gui::UINode* node) {
    if (dynamic_cast<gui::Container*>(node)) {
        return scripting::state->pushcfunction(l_container_add);
    }
    return 0;
}

static int p_get_clear(gui::UINode* node) {
    if (dynamic_cast<gui::Container*>(node)) {
        return scripting::state->pushcfunction(l_container_clear);
    }
    return 0;
}

static int p_get_color(gui::UINode* node) {
    return lua::pushcolor_arr(scripting::state->getLua(), node->getColor());
}
static int p_get_hover_color(gui::UINode* node) {
    return lua::pushcolor_arr(scripting::state->getLua(), node->getHoverColor());
}
static int p_get_pressed_color(gui::UINode* node) {
    return lua::pushcolor_arr(scripting::state->getLua(), node->getPressedColor());
}
static int p_get_pos(gui::UINode* node) {
    return lua::pushvec2_arr(scripting::state->getLua(), node->getPos());
}
static int p_get_size(gui::UINode* node) {
    return lua::pushvec2_arr(scripting::state->getLua(), node->getSize());
}
static int p_is_interactive(gui::UINode* node) {
    return scripting::state->pushboolean(node->isInteractive());
}
static int p_is_visible(gui::UINode* node) {
    return scripting::state->pushboolean(node->isVisible());
}
static int p_is_enabled(gui::UINode* node) {
    return scripting::state->pushboolean(node->isEnabled());
}
static int p_move_into(gui::UINode*) {
    return scripting::state->pushcfunction(l_uinode_move_into);
}

static int p_get_editable(gui::UINode* node) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        return scripting::state->pushboolean(box->isEditable());
    }
    return 0;
}

static int p_get_focused(gui::UINode* node) {
    return scripting::state->pushboolean(node->isFocused());
}

static int p_get_paste(gui::UINode* node) {
    if (dynamic_cast<gui::TextBox*>(node)) {
        return scripting::state->pushcfunction(l_textbox_paste);
    }
    return 0;
}

static int p_get_caret(gui::UINode* node) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        return scripting::state->pushinteger(static_cast<integer_t>(box->getCaret()));
    }
    return 0;
}

static int l_gui_getattr(lua_State* L) {
    auto docname = lua_tostring(L, 1);
    auto element = lua_tostring(L, 2);
    auto attr = lua_tostring(L, 3);
    auto docnode = getDocumentNode(L, docname, element);
    auto node = docnode.node;

    static const std::unordered_map<std::string_view, std::function<int(gui::UINode*)>> getters {
        {"color", p_get_color},
        {"hoverColor", p_get_hover_color},
        {"pressedColor", p_get_pressed_color},
        {"pos", p_get_pos},
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
        {"caret", p_get_caret}
    };
    auto func = getters.find(attr);
    if (func != getters.end()) return func->second(node.get());
    return 0;
}

static void p_set_color(gui::UINode* node, int idx) {
    node->setColor(scripting::state->tocolor(idx));
}
static void p_set_hover_color(gui::UINode* node, int idx) {
    node->setHoverColor(scripting::state->tocolor(idx));
}
static void p_set_pressed_color(gui::UINode* node, int idx) {
    node->setPressedColor(scripting::state->tocolor(idx));
}
static void p_set_pos(gui::UINode* node, int idx) {
    node->setPos(scripting::state->tovec2(idx));
}
static void p_set_size(gui::UINode* node, int idx) {
    node->setSize(scripting::state->tovec2(idx));
}
static void p_set_interactive(gui::UINode* node, int idx) {
    node->setInteractive(scripting::state->toboolean(idx));
}
static void p_set_visible(gui::UINode* node, int idx) {
    node->setVisible(scripting::state->toboolean(idx));
}
static void p_set_enabled(gui::UINode* node, int idx) {
    node->setEnabled(scripting::state->toboolean(idx));
}
static void p_set_placeholder(gui::UINode* node, int idx) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        box->setPlaceholder(util::str2wstr_utf8(scripting::state->tostring(idx)));
    }
}
static void p_set_text(gui::UINode* node, int idx) {
    if (auto label = dynamic_cast<gui::Label*>(node)) {
        label->setText(util::str2wstr_utf8(scripting::state->tostring(idx)));
    } else if (auto button = dynamic_cast<gui::Button*>(node)) {
        button->setText(util::str2wstr_utf8(scripting::state->tostring(idx)));
    } else if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        box->setText(util::str2wstr_utf8(scripting::state->tostring(idx)));
    }
}
static void p_set_value(gui::UINode* node, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setValue(scripting::state->tonumber(idx));
    }
}
static void p_set_min(gui::UINode* node, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setMin(scripting::state->tonumber(idx));
    }
}
static void p_set_max(gui::UINode* node, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setMax(scripting::state->tonumber(idx));
    }
}
static void p_set_step(gui::UINode* node, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setStep(scripting::state->tonumber(idx));
    }
}
static void p_set_track_width(gui::UINode* node, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setTrackWidth(scripting::state->tointeger(idx));
    }
}
static void p_set_track_color(gui::UINode* node, int idx) {
    if (auto bar = dynamic_cast<gui::TrackBar*>(node)) {
        bar->setTrackColor(scripting::state->tocolor(idx));
    }
}
static void p_set_checked(gui::UINode* node, int idx) {
    if (auto box = dynamic_cast<gui::CheckBox*>(node)) {
        box->setChecked(scripting::state->toboolean(idx));
    } else if (auto box = dynamic_cast<gui::FullCheckBox*>(node)) {
        box->setChecked(scripting::state->toboolean(idx));
    }
}
static void p_set_page(gui::UINode* node, int idx) {
    if (auto menu = dynamic_cast<gui::Menu*>(node)) {
        menu->setPage(scripting::state->tostring(idx));
    }
}
static void p_set_inventory(gui::UINode* node, int idx) {
    if (auto view = dynamic_cast<gui::InventoryView*>(node)) {
        auto inventory = scripting::level->inventories->get(scripting::state->tointeger(idx));
        if (inventory == nullptr) {
            view->unbind();
        } else {
            view->bind(inventory, scripting::content);
        }
    }
}

static void p_set_editable(gui::UINode* node, int idx) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        box->setEditable(scripting::state->toboolean(idx));
    }
}

static void p_set_focused(std::shared_ptr<gui::UINode> node, int idx) {
    if (scripting::state->toboolean(idx) && !node->isFocused()) {
        scripting::engine->getGUI()->setFocus(node);
    } else if (node->isFocused()){
        node->defocus();
    }
}

static void p_set_caret(gui::UINode* node, int idx) {
    if (auto box = dynamic_cast<gui::TextBox*>(node)) {
        box->setCaret(static_cast<ssize_t>(scripting::state->tointeger(idx)));
    }
}

static int l_gui_setattr(lua_State* L) {
    auto docname = lua_tostring(L, 1);
    auto element = lua_tostring(L, 2);
    auto attr = lua_tostring(L, 3);

    auto docnode = getDocumentNode(L, docname, element);
    auto node = docnode.node;

    static const std::unordered_map<std::string_view, std::function<void(gui::UINode*,int)>> setters {
        {"color", p_set_color},
        {"hoverColor", p_set_hover_color},
        {"pressedColor", p_set_pressed_color},
        {"pos", p_set_pos},
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
        {"caret", p_set_caret}
    };
    auto func = setters.find(attr);
    if (func != setters.end()) func->second(node.get(), 4);

    static const std::unordered_map<std::string_view, std::function<void(std::shared_ptr<gui::UINode>, int)>> setters2 {
        {"focused", p_set_focused},
    };
    auto func2 = setters2.find(attr);
    if (func2 != setters2.end()) func2->second(node, 4);
    return 0;
}

static int l_gui_get_env(lua_State* L) {
    auto name = lua_tostring(L, 1);
    auto doc = scripting::engine->getAssets()->getLayout(name);
    if (doc == nullptr) {
        throw std::runtime_error("Document '" + std::string(name) + "' not found");
    }
    lua_getglobal(L, lua::LuaState::envName(*doc->getEnvironment()).c_str());
    return 1;
}

static int l_gui_str(lua_State* L) {
    auto text = util::str2wstr_utf8(lua_tostring(L, 1));
    if (!lua_isnoneornil(L, 2)) {
        auto context = util::str2wstr_utf8(lua_tostring(L, 2));
        lua_pushstring(L, util::wstr2str_utf8(langs::get(text, context)).c_str());
    } else {
        lua_pushstring(L, util::wstr2str_utf8(langs::get(text)).c_str());
    }
    return 1;
}

static int l_gui_reindex(lua_State* L) {
    auto name = lua_tostring(L, 1);
    auto doc = scripting::engine->getAssets()->getLayout(name);
    if (doc == nullptr) {
        throw std::runtime_error("Document '" + std::string(name) + "' not found");
    }
    doc->rebuildIndices();
    return 0;
}

static int l_gui_get_locales_info(lua_State* L) {
    auto& locales = langs::locales_info;
    lua_createtable(L, 0, locales.size());
    for (auto& entry : locales) {
        lua_createtable(L, 0, 1);
        lua_pushstring(L, entry.second.name.c_str());
        lua_setfield(L, -2, "name");
        lua_setfield(L, -2, entry.first.c_str());
    }
    return 1;
}

static int l_gui_get_viewport(lua_State* L) {
    return lua::pushvec2_arr(L, scripting::engine->getGUI()->getContainer()->getSize());;
}

const luaL_Reg guilib [] = {
    {"get_viewport", lua_wrap_errors<l_gui_get_viewport>},
    {"getattr", lua_wrap_errors<l_gui_getattr>},
    {"setattr", lua_wrap_errors<l_gui_setattr>},
    {"get_env", lua_wrap_errors<l_gui_get_env>},
    {"str", lua_wrap_errors<l_gui_str>},
    {"reindex", lua_wrap_errors<l_gui_reindex>},
    {"get_locales_info", lua_wrap_errors<l_gui_get_locales_info>},
    {NULL, NULL}
};
