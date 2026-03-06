#ifndef ASSETS_ASSET_LOADERS_H_
#define ASSETS_ASSET_LOADERS_H_

#include <string>

class ResPaths;
class Assets;
class Atlas;

// Пространство имён, содержащее функции загрузки различных ресурсов
namespace asset_loader {
    // Загружает текстуру из файла filename и сохраняет её в Assets под именем name
    bool texture(Assets* assets, const ResPaths* paths, const std::string filename, const std::string name);

    // Загружает шейдерную программу из файлов filename.vert и filename.frag и сохраняет его в Assets под именем name
    bool shader(Assets* assets, const ResPaths* paths, const std::string filename, const std::string name);

    // Загружает атлас текстур из всех файлов в директории directory и сохраняет под именем name
    bool atlas(Assets* assets, const ResPaths* paths, const std::string directory, const std::string name);

    // Загружает шрифт из файлов filename_idx.png и сохраняет его в Assets под именем name
    bool font(Assets* assets, const ResPaths* paths, const std::string filename, const std::string name);

    bool animation(Assets* assets, const ResPaths* paths, const std::string directory, const std::string name, Atlas* dstAtlas);
}

#endif // ASSETS_ASSET_LOADERS_H_
