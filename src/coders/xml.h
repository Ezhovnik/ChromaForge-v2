#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include <coders/commons.h>

namespace xml {
    class Node;
    class Attribute;
    class Document;

    class Attribute {
    private:
        std::string name;
        std::string text;
    public:
        Attribute() = default;
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
        std::unordered_map<std::string, Attribute> attrs;
        std::vector<std::unique_ptr<Node>> elements;
    public:
        Node(std::string tag);
        Node(const Node&) = delete;

        void add(std::unique_ptr<Node> element);

        void set(const std::string& name, const std::string& text);

        const std::string& getTag() const;

        inline bool isText() const {
            return getTag() == "#";
        }

        inline const std::string& text() const {
            return attr("#").getText();
        }

        const Attribute& attr(const std::string& name) const;
        Attribute attr(const std::string& name, const std::string& def) const;

        bool has(const std::string& name) const;

        Node& sub(size_t index);
        const Node& sub(size_t index) const;

        size_t size() const;

        const std::vector<std::unique_ptr<Node>>& getElements() const;
        const std::unordered_map<std::string, Attribute>& getAttributes() const;
    };

    class Document {
    private:
        std::unique_ptr<Node> root = nullptr;
        std::string version;
        std::string encoding;
    public:
        Document(std::string version, std::string encoding);

        void setRoot(std::unique_ptr<Node> element);
        const Node* getRoot() const;

        const std::string& getVersion() const;
        const std::string& getEncoding() const;
    };

    std::string stringify(
        const Document& document,
        bool nice=true,
        const std::string& indentStr="    "
    );

    std::unique_ptr<Document> parse(
        std::string_view filename, std::string_view source
    );

    using xmlelement = Node;
}
