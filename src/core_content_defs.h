#pragma once

#include <string>

inline const std::string BUILTIN_CONTENT_NAMESPACE = "builtin";
inline const std::string CHROMAFORGE_CONTENT_NAMESPACE = "chromaforge";

inline const std::string BUILTIN_EMPTY = BUILTIN_CONTENT_NAMESPACE + ":empty";
inline const std::string BUILTIN_AIR = BUILTIN_CONTENT_NAMESPACE + ":air";

inline const std::string TEXTURE_NOTFOUND = "notfound";

class ContentBuilder;
class EnginePaths;

namespace CoreContent {
    void setup(EnginePaths* paths, ContentBuilder* builder);
}
