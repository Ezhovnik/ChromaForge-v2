#ifndef GRAPHICS_UI_GUI_XML_H_
#define GRAPHICS_UI_GUI_XML_H_

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <stack>

#include "GUI.h"
#include "../../coders/xml.h"

namespace scripting {
    class Environment;
}

namespace gui {
    class UIXmlReader;

    using uinode_reader = std::function<std::shared_ptr<UINode>(UIXmlReader&, xml::xmlelement)>;

    class UIXmlReader {
    private:
        std::string filename;
        std::unordered_map<std::string, uinode_reader> readers;
        std::unordered_set<std::string> ignored;
        std::stack<std::string> contextStack;
        const scripting::Environment& env;
    public:
        UIXmlReader(const scripting::Environment& env);

        void add(const std::string& tag, uinode_reader reader);

        bool hasReader(const std::string& tag) const;

        void addIgnore(const std::string& tag);

        std::shared_ptr<UINode> readUINode(xml::xmlelement element);
        void readUINode(UIXmlReader& reader, xml::xmlelement element, UINode& node );
        void readUINode(UIXmlReader& reader, xml::xmlelement element, Container& container);

        std::shared_ptr<UINode> readXML(const std::string& filename, const std::string& source);
        std::shared_ptr<UINode> readXML(const std::string& filename, xml::xmlelement root);

        const std::string& getContext() const;
        const scripting::Environment& getEnvironment() const;
        const std::string& getFilename() const;
    };
}

#endif // GRAPHICS_UI_GUI_XML_H_
