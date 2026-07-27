#include <gtest/gtest.h>
#include <iostream>

#include <coders/xml.h>
#include <coders/commons.h>

TEST(XML, CFModel) {
    std::string code = ""
        "@line x1 y1 z1\n"
        "# @tst ysfdrg\n"
        "@box {\n"
        "  @rect texture \"$2\"\n"
        "  @utro a 53.1\n"
        "}\n"
    ;

    try {
        auto document = xml::parse_cfmodel("<test>", code, "test");
        std::cout << xml::stringify(*document);
    } catch (const parsing_error& err) {
        std::cerr << err.errorLog() << std::endl;
        throw;
    }
}
