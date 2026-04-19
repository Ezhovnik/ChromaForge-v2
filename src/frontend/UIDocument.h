#ifndef FRONTEND_UI_DOCUMENT_H_
#define FRONTEND_UI_DOCUMENT_H_

#include <string>
#include <memory>
#include <filesystem>
#include <unordered_map>

#include <typedefs.h>

namespace gui {
    class UINode;
}

struct uidocscript {
    bool onopen : 1;
    bool onprogress : 1;
    bool onclose : 1;
};

using uinodes_map = std::unordered_map<std::string, std::shared_ptr<gui::UINode>>;

class UIDocument {
private:
    std::string id;
    uidocscript script;
    uinodes_map map;
    std::shared_ptr<gui::UINode> root;

    scriptenv env;
public:
    UIDocument(
        std::string id, 
        uidocscript script, 
        const std::shared_ptr<gui::UINode>& root, 
        scriptenv env
    );

    void rebuildIndices();

    const uinodes_map& getMap() const;
    uinodes_map& getMapWriteable();
    const std::string& getId() const;
    std::shared_ptr<gui::UINode> getRoot() const;
    const uidocscript& getScript() const;
    scriptenv getEnvironment() const;
    std::shared_ptr<gui::UINode> get(const std::string& id) const;

    static std::unique_ptr<UIDocument> read(
        const scriptenv& parent_env,
        const std::string& name,
        const std::filesystem::path& file
    );
    static std::shared_ptr<gui::UINode> readElement(
        const std::filesystem::path& file
    );
};

#endif // FRONTEND_UI_DOCUMENT_H_
