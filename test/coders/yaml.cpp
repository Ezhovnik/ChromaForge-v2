#include <gtest/gtest.h>
#include <iostream>

#include <coders/yaml.h>
#include <coders/commons.h>

TEST(YAML, EncodeDecode) {
    std::string code =
        "name: ChromaForge\n"
        "version: 1.0\n"
        "enabled: true\n"
        "values:\n"
        "  - one\n"
        "  - two\n"
        "  - three\n";
    try {
        auto value = yaml::parse(code);
        std::cout << yaml::stringify(value) << std::endl;
    } catch (const parsing_error& error) {
        std::cerr << error.errorLog() << std::endl;
        throw;
    }
}
