#pragma once

#include <string>
#include <memory>

#include <assets/Assets.h>

class ResPaths;
class Assets;
class Atlas;
class AssetsLoader;
struct AssetsConfig;

// Пространство имён, содержащее функции загрузки различных ресурсов
namespace asset_loader {
    postfunc texture(
        AssetsLoader* loader,
        const ResPaths* paths, 
        const std::string& filename, 
        const std::string& name,
        const std::shared_ptr<AssetsConfig>& settings
    );

    postfunc shader(
        AssetsLoader* loader,
        const ResPaths* paths, 
        const std::string& filename, 
        const std::string& name,
        const std::shared_ptr<AssetsConfig>& settings
    );

    postfunc atlas(
        AssetsLoader* loader,
        const ResPaths* paths, 
        const std::string& directory, 
        const std::string& name,
        const std::shared_ptr<AssetsConfig>& settings
    );

    postfunc font(
        AssetsLoader* loader,
        const ResPaths* paths, 
        const std::string& filename, 
        const std::string& name,
        const std::shared_ptr<AssetsConfig>& settings
    );

    postfunc layout(
        AssetsLoader* loader,
        const ResPaths* paths, 
        const std::string& file, 
        const std::string& name,
        const std::shared_ptr<AssetsConfig>& settings
    );

    postfunc sound(
        AssetsLoader* loader,
        const ResPaths* paths,
        const std::string& file,
        const std::string& name,
        const std::shared_ptr<AssetsConfig>& settings
    );

    postfunc model(
        AssetsLoader*,
        const ResPaths* paths,
        const std::string& file,
        const std::string& name,
        const std::shared_ptr<AssetsConfig>& settings
    );
}
