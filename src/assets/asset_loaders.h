#ifndef ASSETS_ASSET_LOADERS_H_
#define ASSETS_ASSET_LOADERS_H_

#include <string>
#include <memory>

class ResPaths;
class Assets;
class Atlas;
class AssetsLoader;

// Пространство имён, содержащее функции загрузки различных ресурсов
namespace asset_loader {
    // Загружает текстуру из файла filename и сохраняет её в Assets под именем name
    bool texture(
        AssetsLoader&,
        Assets* assets, 
        const ResPaths* paths, 
        const std::string filename, 
        const std::string name,
        std::shared_ptr<void> settings
    );

    // Загружает шейдерную программу из файлов filename.vert и filename.frag и сохраняет его в Assets под именем name
    bool shader(
        AssetsLoader&,
        Assets* assets, 
        const ResPaths* paths, 
        const std::string filename, 
        const std::string name,
        std::shared_ptr<void> settings
    );

    // Загружает атлас текстур из всех файлов в директории directory и сохраняет под именем name
    bool atlas(
        AssetsLoader&,
        Assets* assets, 
        const ResPaths* paths, 
        const std::string directory, 
        const std::string name,
        std::shared_ptr<void> settings
    );

    // Загружает шрифт из файлов filename_idx.png и сохраняет его в Assets под именем name
    bool font(
        AssetsLoader&,
        Assets* assets, 
        const ResPaths* paths, 
        const std::string filename, 
        const std::string name,
        std::shared_ptr<void> settings
    );

    bool animation(
        Assets* assets, 
        const ResPaths* paths, 
        const std::string directory, 
        const std::string name, 
        Atlas* dstAtlas
    );

    bool layout(
        AssetsLoader& loader,
        Assets* assets, 
        const ResPaths* paths, 
        const std::string file, 
        const std::string name,
        std::shared_ptr<void> settings
    );
}

#endif // ASSETS_ASSET_LOADERS_H_
