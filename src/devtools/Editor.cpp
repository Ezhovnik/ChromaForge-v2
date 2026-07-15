#include <devtools/Editor.h>

#include <engine/Engine.h>
#include <io/engine_paths.h>
#include <coders/syntax_parser.h>
#include <devtools/SyntaxProcessor.h>

devtools::Editor::Editor(Engine& engine)
    : engine(engine), syntaxProcessor(std::make_unique<SyntaxProcessor>()) {
}

devtools::Editor::~Editor() = default;

void devtools::Editor::loadTools() {
    const auto& paths = engine.getResPaths();
    auto files = paths.listdir("devtools/syntax");
    for (const auto& file : files) {
        auto config = io::read_object(file);
        auto syntax = std::make_unique<Syntax>();
        syntax->deserialize(config);
        syntaxProcessor->addSyntax(std::move(syntax));
    }
}

devtools::SyntaxProcessor& devtools::Editor::getSyntaxProcessor() {
    return *syntaxProcessor;
}
