#pragma once

#include <memory>

#include <logic/scripting/lua/libs/api_lua.h>
#include <frontend/UIDocument.h>
#include <graphics/ui/elements/UINode.h>

struct DocumentNode {
    UIDocument* document;
    std::shared_ptr<gui::UINode> node;
};

DocumentNode get_document_node(lua::State* L, int idx = 1);
