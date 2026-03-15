#include "UIDocument.h"

#include "../graphics/ui/elements/UINode.h"
#include "../graphics/ui/elements/containers.h"
#include "InventoryView.h"
#include "../logic/scripting/scripting.h"
#include "../files/files.h"
#include "../graphics/ui/gui_xml.h"

UIDocument::UIDocument(
    std::string id, 
    uidocscript script, 
    std::shared_ptr<gui::UINode> root, 
    std::unique_ptr<scripting::Environment> env
) : id(id), 
    script(script), 
    root(root), 
    env(std::move(env)) 
{
    collect(map, root);
}

const uinodes_map& UIDocument::getMap() const {
    return map;
}

const std::string& UIDocument::getId() const {
    return id;
}

const std::shared_ptr<gui::UINode> UIDocument::getRoot() const {
    return root;
}

const uidocscript& UIDocument::getScript() const {
    return script;
}

int UIDocument::getEnvironment() const {
    return env->getId();
}

const std::shared_ptr<gui::UINode> UIDocument::get(const std::string& id) const {
    auto found = map.find(id);
    if (found == map.end()) return nullptr;
    return found->second;
}

void UIDocument::collect(uinodes_map& map, std::shared_ptr<gui::UINode> node) {
    const std::string& id = node->getId();
    if (!id.empty()) map[id] = node;
    auto container = std::dynamic_pointer_cast<gui::Container>(node);
    if (container) {
        for (auto subnode : container->getNodes()) {
            collect(map, subnode);
        }
    }
}

std::unique_ptr<UIDocument> UIDocument::read(int parent_env, std::string name, std::filesystem::path file) {
    const std::string text = files::read_string(file);
    auto xmldoc = xml::parse(file.u8string(), text);

    auto env = parent_env == -1 ? std::make_unique<scripting::Environment>(0) : scripting::create_doc_environment(parent_env, name);
    gui::UIXmlReader reader(*env);
    InventoryView::createReaders(reader);
    auto view = reader.readXML(file.u8string(), xmldoc->getRoot());
    view->setId("root");
    uidocscript script {};
    auto scriptFile = std::filesystem::path(file.u8string() + ".lua");
    if (std::filesystem::is_regular_file(scriptFile)) scripting::load_layout_script(env->getId(), name, scriptFile, script);
    return std::make_unique<UIDocument>(name, script, view, std::move(env));
}

std::shared_ptr<gui::UINode> UIDocument::readElement(std::filesystem::path file) {
    auto document = read(-1, file.filename().u8string(), file);
    return document->getRoot();
}
