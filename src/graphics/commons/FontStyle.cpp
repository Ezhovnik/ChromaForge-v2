#include <graphics/commons/FontStyle.h>

#include <data/dv.h>
#include <data/dv_util.h>
#include <devtools/SyntaxProcessor.h>

FontStyle FontStyle::parse(const dv::value& src) {
    FontStyle style {};
    src.at("bold").get(style.bold);
    src.at("italic").get(style.italic);
    src.at("strikethrough").get(style.strikethrough);
    src.at("underline").get(style.underline);
    dv::get_vec(src, "color", style.color);
    return style;
}

static void parse_style(
    const dv::value& src,
    FontStylesScheme& scheme,
    const std::string& name,
    devtools::SyntaxStyles tag
) {
    if (src.has(name)) {
        scheme.palette[static_cast<int>(tag)] = FontStyle::parse(src[name]);
    }
}

FontStylesScheme FontStylesScheme::parse(const dv::value& src) {
    FontStylesScheme scheme {};
    scheme.palette.resize(8);
    parse_style(src, scheme, "default", devtools::SyntaxStyles::Default);
    parse_style(src, scheme, "keyword", devtools::SyntaxStyles::Keyword);
    parse_style(src, scheme, "literal", devtools::SyntaxStyles::Literal);
    parse_style(src, scheme, "comment", devtools::SyntaxStyles::Comment);
    parse_style(src, scheme, "error", devtools::SyntaxStyles::Error);
    return scheme;
}
