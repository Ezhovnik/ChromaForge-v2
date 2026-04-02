#include "UIDocument.h"

#include <utility>

#include "../graphics/ui/elements/UINode.h"
#include "../graphics/ui/elements/display/InventoryView.h"
#include "../logic/scripting/scripting.h"
#include "../files/files.h"
#include "../graphics/ui/gui_xml.h"

UIDocument::UIDocument(
    std::string id, 
    uidocscript script, 
    const std::shared_ptr<gui::UINode>& root, 
    scriptenv env
) : id(std::move(id)), 
    script(script), 
    root(root), 
    env(std::move(env))
{
    gui::UINode::getIndices(root, map);
}

void UIDocument::rebuildIndices() {
    gui::UINode::getIndices(root, map);
}

const uinodes_map& UIDocument::getMap() const {
    return map;
}

uinodes_map& UIDocument::getMapWriteable() {
    return map;
}

const std::string& UIDocument::getId() const {
    return id;
}

std::shared_ptr<gui::UINode> UIDocument::getRoot() const {
    return root;
}

const uidocscript& UIDocument::getScript() const {
    return script;
}

scriptenv UIDocument::getEnvironment() const {
    return env;
}

std::shared_ptr<gui::UINode> UIDocument::get(const std::string& id) const {
    auto found = map.find(id);
    if (found == map.end()) return nullptr;
    return found->second;
}

std::unique_ptr<UIDocument> UIDocument::read(
    const scriptenv& parent_env,
    const std::string& name,
    const std::filesystem::path& file
) {
    const std::string text = files::read_string(file);
    auto xmldoc = xml::parse(file.u8string(), text);

    auto env = parent_env == nullptr 
        ? scripting::create_doc_environment(scripting::get_root_environment(), name)
        : scripting::create_doc_environment(parent_env, name);
    gui::UIXmlReader reader(env);
    auto view = reader.readXML(file.u8string(), xmldoc->getRoot());
    view->setId("root");
    uidocscript script {};
    auto scriptFile = std::filesystem::path(file.u8string() + ".lua");
    if (std::filesystem::is_regular_file(scriptFile)) scripting::load_layout_script(env, name, scriptFile, script);
    return std::make_unique<UIDocument>(name, script, view, env);
}

std::shared_ptr<gui::UINode> UIDocument::readElement(
    const std::filesystem::path& file
) {
    auto document = read(nullptr, file.filename().u8string(), file);
    return document->getRoot();
}
