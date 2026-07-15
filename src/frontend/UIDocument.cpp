#include <frontend/UIDocument.h>

#include <utility>

#include <graphics/ui/elements/UINode.h>
#include <graphics/ui/elements/InventoryView.h>
#include <logic/scripting/scripting.h>
#include <io/io.h>
#include <graphics/ui/gui_xml.h>

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
    rebuildIndices();
}

void UIDocument::rebuildIndices() {
    map.clear();
    gui::UINode::getIndices(root, map);
    map["root"] = root;
}

const UINodesMap& UIDocument::getMap() const {
    return map;
}

UINodesMap& UIDocument::getMapWriteable() {
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
    gui::GUI& gui,
    const scriptenv& parent_env,
    const std::string& name,
    const io::path& file,
    const std::string& fileName
) {
    const std::string text = io::read_string(file);
    auto xmldoc = xml::parse(file.string(), text);

    auto env = parent_env == nullptr 
        ? scripting::create_doc_environment(scripting::get_root_environment(), name)
        : scripting::create_doc_environment(parent_env, name);
    gui::UIXmlReader reader(gui, scriptenv(env));
    auto view = reader.readXML(file.string(), *xmldoc->getRoot());
    view->setId("root");
    uidocscript script {};
    auto scriptFile = io::path(file.string() + ".lua");
    if (io::is_regular_file(scriptFile)) {
        scripting::load_layout_script(env, name, scriptFile, fileName + ".lua", script);
    }
    return std::make_unique<UIDocument>(name, script, view, env);
}

std::shared_ptr<gui::UINode> UIDocument::readElement(
    gui::GUI& gui, const io::path& file, const std::string& fileName
) {
    return read(gui, nullptr, file.name(), file, fileName)->getRoot();
}
