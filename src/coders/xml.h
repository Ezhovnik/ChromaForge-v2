#ifndef CODERS_XML_H_
#define CODERS_XML_H_

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include "commons.h"

namespace xml {
    class Node;
    class Attribute;
    class Document;

    typedef Attribute xmlattribute;
    typedef std::shared_ptr<Node> xmlelement;
    typedef std::shared_ptr<Document> xmldocument;
    typedef std::unordered_map<std::string, xmlattribute> xmlelements_map;

    class Attribute {
    private:
        std::string name;
        std::string text;
    public:
        Attribute() {};
        Attribute(std::string name, std::string text);

        const std::string& getName() const;
        const std::string& getText() const;
        int64_t asInt() const;
        double asFloat() const;
        bool asBool() const;
        glm::vec2 asVec2() const;
        glm::vec3 asVec3() const;
        glm::vec4 asVec4() const;
        glm::vec4 asColor() const;
    };

    class Node {
    private:
        std::string tag;
        std::unordered_map<std::string, xmlattribute> attrs;
        std::vector<xmlelement> elements;
    public:
        Node(std::string tag);

        void add(xmlelement element);
        
        void set(std::string name, std::string text);

        const std::string& getTag() const;

        inline bool isText() const {
            return getTag() == "#";
        }

        inline const std::string& text() const {
            return attr("#").getText();
        }

        const xmlattribute attr(const std::string& name) const;
        const xmlattribute attr(const std::string& name, const std::string& def) const;

        bool has(const std::string& name) const;

        xmlelement sub(size_t index);

        size_t size() const;

        const std::vector<xmlelement>& getElements() const;
        const xmlelements_map& getAttributes() const;
    };

    class Document {
    private:
        xmlelement root = nullptr;
        std::string version;
        std::string encoding;
    public:
        Document(std::string version, std::string encoding);

        void setRoot(xmlelement element);
        xmlelement getRoot() const;

        const std::string& getVersion() const;
        const std::string& getEncoding() const;
    };

    class Parser : public BasicParser {
    private:
        xmldocument document;

        xmlelement parseOpenTag();
        xmlelement parseElement();
        void parseDeclaration();
        void parseComment();
        std::string parseText();
        std::string parseXMLName();
    public:
        Parser(std::string filename, std::string source);

        xmldocument parse();
    };

    extern std::string stringify(
        const xmldocument document,
        bool nice=true,
        const std::string& indentStr="    "
    );

    extern xmldocument parse(std::string filename, std::string source);
}

#endif // CODERS_XML_H_
