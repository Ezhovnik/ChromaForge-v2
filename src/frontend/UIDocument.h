#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <typedefs.h>
#include <io/fwd.h>

namespace gui {
    class GUI;
    class UINode;
}

struct uidocscript {
    bool onopen : 1;
    bool onprogress : 1;
    bool onclose : 1;
};

using UINodesMap = std::unordered_map<std::string, std::shared_ptr<gui::UINode>>;

class UIDocument {
private:
    std::string id;
    uidocscript script;
    UINodesMap map;
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

    const UINodesMap& getMap() const;
    UINodesMap& getMapWriteable();
    const std::string& getId() const;
    std::shared_ptr<gui::UINode> getRoot() const;
    const uidocscript& getScript() const;
    scriptenv getEnvironment() const;
    std::shared_ptr<gui::UINode> get(const std::string& id) const;

    static std::unique_ptr<UIDocument> read(
        gui::GUI&,
        const scriptenv& parent_env,
        const std::string& name,
        const io::path& file,
        const std::string& fileName
    );
    static std::shared_ptr<gui::UINode> readElement(
        gui::GUI&, const io::path& file, const std::string& fileName
    );
};
