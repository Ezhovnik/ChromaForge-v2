#include "AssetsLoader.h"

#include <memory>

#include "Assets.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Texture.h"
#include "../coders/png.h"
#include "../graphics/Font.h"
#include "../graphics/Atlas.h"
#include "../logger/Logger.h"
#include "../constants.h"
#include "../graphics/ImageData.h"
#include "asset_loaders.h"
#include "../files/engine_paths.h"
#include "../core_defs.h"
#include "../content/Content.h"
#include "../logic/scripting/scripting.h"

AssetsLoader::AssetsLoader(Assets* assets, const ResPaths* paths) : assets(assets), paths(paths) {
	// Регистрируем встроенные загрузчики из asset_loaders.h
	addLoader(AssetType::Shader, asset_loader::shader);
	addLoader(AssetType::Texture, asset_loader::texture);
	addLoader(AssetType::Font, asset_loader::font);
    addLoader(AssetType::Atlas, asset_loader::atlas);
	addLoader(AssetType::Layout, asset_loader::layout);
	addLoader(AssetType::Sound, asset_loader::sound);
}

void AssetsLoader::addLoader(AssetType tag, aloader_func func) {
	loaders[tag] = func;
}

void AssetsLoader::add(AssetType tag, const std::string filename, const std::string alias, std::shared_ptr<AssetsConfig> config) {
	entries.push(aloader_entry{tag, filename, alias, config});
}

bool AssetsLoader::hasNext() const {
	return !entries.empty();
}

bool AssetsLoader::loadNext() {
	// Берём первый элемент очереди (не удаляя его пока)
	const aloader_entry& entry = entries.front();
    LOG_DEBUG("Loading {} as {}", entry.filename, entry.alias);
	Logger::getInstance().flush();

	// Ищем загрузчик по типу ресурса
	auto found = loaders.find(entry.tag);
	if (found == loaders.end()) {
        LOG_ERROR("Unknown asset tag {}", (int)entry.tag);
		entries.pop();
		return false;
	}
	aloader_func loader = found->second;
	bool status = loader(*this, assets, paths, entry.filename, entry.alias, entry.config);
	entries.pop();
	return status;
}

/**
 * @brief Вспомогательная функция для добавления всех .xml макетов из указанной папки.
 * @param env Идентификатор окружения (для LayoutConfig).
 * @param prefix Префикс имени (обычно namespace контента).
 * @param folder Путь к папке с макетами.
 * @param loader Загрузчик, в который добавляются задания.
 *
 * Функция проходит по всем файлам с расширением .xml в папке folder и добавляет их как Layout.
 * Имя формируется как "prefix:stem", где stem — имя файла без расширения.
 */
void addLayouts(int env, const std::string& prefix, const std::filesystem::path& folder, AssetsLoader& loader) {
    if (!std::filesystem::is_directory(folder)) return;

    for (auto& entry : std::filesystem::directory_iterator(folder)) {
        const std::filesystem::path file = entry.path();
        if (file.extension().u8string() != ".xml") continue;
        std::string name = prefix + ":" + file.stem().u8string();

		// Для каждого макета создаём LayoutConfig с указанным окружением
        loader.add(AssetType::Layout, file.u8string(), name, std::make_shared<LayoutConfig>(env));
    }
}

void AssetsLoader::addDefaults(AssetsLoader& loader, const Content* content) {
	// Шрифт
	loader.add(AssetType::Font, FONTS_FOLDER"/font", "normal");

	// Базовые шейдеры
	loader.add(AssetType::Shader, SHADERS_FOLDER"/default", "default");
	loader.add(AssetType::Shader, SHADERS_FOLDER"/ui", "ui");
	loader.add(AssetType::Shader, SHADERS_FOLDER"/lines", "lines");

	// Интерфейсные текстуры
	loader.add(AssetType::Texture, TEXTURES_FOLDER"/gui/menubg.png", "gui/menubg");
	loader.add(AssetType::Texture, TEXTURES_FOLDER"/gui/delete_icon.png", "gui/delete_icon");
	loader.add(AssetType::Texture, TEXTURES_FOLDER"/gui/no_icon.png", "gui/no_icon");
	loader.add(AssetType::Texture, TEXTURES_FOLDER"/gui/warning.png", "gui/warning");
    loader.add(AssetType::Texture, TEXTURES_FOLDER"/gui/error.png", "gui/error");
    loader.add(AssetType::Texture, TEXTURES_FOLDER"/gui/cross.png", "gui/cross");

	if (content) {
		// Дополнительные шейдеры
		loader.add(AssetType::Shader, SHADERS_FOLDER"/skybox_gen", "skybox_gen");
		loader.add(AssetType::Shader, SHADERS_FOLDER"/background", "background");
		loader.add(AssetType::Shader, SHADERS_FOLDER"/ui3d", "ui3d");
		loader.add(AssetType::Shader, SHADERS_FOLDER"/screen", "screen");

		// Дополнительные текстуры
		loader.add(AssetType::Texture, TEXTURES_FOLDER"/misc/moon.png", "misc/moon");
        loader.add(AssetType::Texture, TEXTURES_FOLDER"/misc/sun.png", "misc/sun");
		loader.add(AssetType::Texture, TEXTURES_FOLDER"/gui/crosshair.png", "gui/crosshair");

		// Макеты интерфейса из корневой папки "layouts" (встроенный контент)
		addLayouts(
			0, 
			BUILTIN_CONTENT_NAMESPACE, 
			loader.getPaths()->getMainRoot()/std::filesystem::path("layouts"), 
			loader
		);

		// Макеты из каждого установленного пака
        for (auto& entry : content->getPacks()) {
			auto pack = entry.second.get();
            auto& info = pack->getInfo();
            std::filesystem::path folder = info.folder/std::filesystem::path("layouts");
            addLayouts(pack->getEnvironment()->getId(), info.id, folder, loader);
        }
	}

	// Атласы блоков и предметов
	loader.add(AssetType::Atlas, TEXTURES_FOLDER"/blocks", "blocks");
	loader.add(AssetType::Atlas, TEXTURES_FOLDER"/items", "items");
}

const ResPaths* AssetsLoader::getPaths() const {
	return paths;
}
