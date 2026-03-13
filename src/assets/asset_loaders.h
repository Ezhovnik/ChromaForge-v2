#ifndef ASSETS_ASSET_LOADERS_H_
#define ASSETS_ASSET_LOADERS_H_

#include <string>
#include <memory>

class ResPaths;
class Assets;
class Atlas;
class AssetsLoader;
struct AssetsConfig;

// Пространство имён, содержащее функции загрузки различных ресурсов
namespace asset_loader {
    /**
     * @brief Загружает текстуру из файла и сохраняет её в менеджере ресурсов.
     * @param loader Загрузчик ассетов (не используется).
     * @param assets Указатель на менеджер ресурсов, в который будет сохранена текстура.
     * @param paths Указатель на объект с путями для поиска файлов.
     * @param filename Имя файла текстуры (с расширением .png).
     * @param name Имя, под которым текстура будет сохранена в менеджере ресурсов.
     * @param settings Дополнительные настройки (не используются).
     * @return true, если текстура успешно загружена и сохранена, иначе false.
     */
    bool texture(
        AssetsLoader&,
        Assets* assets, 
        const ResPaths* paths, 
        const std::string filename, 
        const std::string name,
        std::shared_ptr<AssetsConfig> settings
    );

    /**
     * @brief Загружает шейдерную программу из файлов .vert и .frag и сохраняет её в менеджере ресурсов.
     * @param loader Загрузчик ассетов (не используется).
     * @param assets Указатель на менеджер ресурсов.
     * @param paths Указатель на объект с путями.
     * @param filename Базовое имя файла шейдера (без расширения). К нему добавляются .vert и .frag.
     * @param name Имя, под которым шейдер будет сохранён.
     * @param settings Дополнительные настройки (не используются).
     * @return true при успехе, false при ошибке.
     */
    bool shader(
        AssetsLoader&,
        Assets* assets, 
        const ResPaths* paths, 
        const std::string filename, 
        const std::string name,
        std::shared_ptr<AssetsConfig> settings
    );

    /**
     * @brief Загружает атлас текстур из всех PNG-файлов в указанной директории и сохраняет его в менеджере ресурсов.
     * @param loader Загрузчик ассетов (не используется).
     * @param assets Указатель на менеджер ресурсов.
     * @param paths Указатель на объект с путями.
     * @param directory Путь к директории, содержащей изображения для атласа.
     * @param name Имя, под которым атлас будет сохранён.
     * @param settings Дополнительные настройки (не используются).
     * @return true при успехе, false при ошибке.
     */
    bool atlas(
        AssetsLoader&,
        Assets* assets, 
        const ResPaths* paths, 
        const std::string directory, 
        const std::string name,
        std::shared_ptr<AssetsConfig> settings
    );

    /**
     * @brief Загружает растровый шрифт из файлов (например, filename_0.png ... filename_4.png) и сохраняет его в менеджере ресурсов.
     * @param loader Загрузчик ассетов (не используется).
     * @param assets Указатель на менеджер ресурсов.
     * @param paths Указатель на объект с путями.
     * @param filename Базовое имя файлов страниц шрифта (к нему добавляются "_0.png", "_1.png" и т.д.).
     * @param name Имя, под которым шрифт будет сохранён.
     * @param settings Дополнительные настройки (не используются).
     * @return true при успехе, false при ошибке.
     */
    bool font(
        AssetsLoader&,
        Assets* assets, 
        const ResPaths* paths, 
        const std::string filename, 
        const std::string name,
        std::shared_ptr<AssetsConfig> settings
    );

    /**
     * @brief Загружает макет интерфейса из XML-файла и сохраняет его в менеджере ресурсов.
     * @param loader Загрузчик ассетов (используется для загрузки вложенных ресурсов).
     * @param assets Указатель на менеджер ресурсов.
     * @param paths Указатель на объект с путями.
     * @param file Имя файла макета (путь относительно ресурсов).
     * @param name Имя, под которым макет будет сохранён.
     * @param config Дополнительные настройки окружения.
     * @return true при успехе, false при ошибке парсинга.
     */
    bool layout(
        AssetsLoader& loader,
        Assets* assets, 
        const ResPaths* paths, 
        const std::string file, 
        const std::string name,
        std::shared_ptr<AssetsConfig> settings
    );

    /**
     * @brief Загружает звук из .wav файла.
     * @param loader Загрузчик ассетов.
     * @param assets Указатель на менеджер ресурсов.
     * @param paths Указатель на объект с путями.
     * @param file Имя файла звук (путь относительно ресурсов).
     * @param name Имя, под которым звук будет сохранён.
     * @param config Дополнительные настройки.
     * @return true при успехе, false при ошибке.
     */
    bool sound(
        AssetsLoader& loader,
        Assets* assets,
        const ResPaths* paths,
        const std::string file,
        const std::string name,
        std::shared_ptr<AssetsConfig> settings
    );
}

#endif // ASSETS_ASSET_LOADERS_H_
