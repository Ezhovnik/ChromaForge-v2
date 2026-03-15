#ifndef FRONTEND_UI_DOCUMENT_H_
#define FRONTEND_UI_DOCUMENT_H_

#include <string>
#include <memory>
#include <filesystem>
#include <unordered_map>

namespace gui {
    class UINode;
}

namespace scripting {
    class Environment;
}

struct uidocscript {
    int environment;
    bool onopen : 1;
    bool onclose : 1;
};

using uinodes_map = std::unordered_map<std::string, std::shared_ptr<gui::UINode>>;

class UIDocument {
    std::string id;
    uidocscript script;
    uinodes_map map;
    std::shared_ptr<gui::UINode> root;

    std::unique_ptr<scripting::Environment> env;
public:
    UIDocument(
        std::string id, 
        uidocscript script, 
        std::shared_ptr<gui::UINode> root, 
        std::unique_ptr<scripting::Environment> env
    );

    const uinodes_map& getMap() const;
    const std::string& getId() const;
    const std::shared_ptr<gui::UINode> getRoot() const;
    const uidocscript& getScript() const;
    int getEnvironment() const;
    const std::shared_ptr<gui::UINode> get(const std::string& id) const;

    static void collect(uinodes_map& map, std::shared_ptr<gui::UINode> node);

    static std::unique_ptr<UIDocument> read(int parent_env, std::string name, std::filesystem::path file);
    static std::shared_ptr<gui::UINode> readElement(std::filesystem::path file);
};

#endif // FRONTEND_UI_DOCUMENT_H_
