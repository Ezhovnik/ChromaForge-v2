#include "AssetsLoader.h"

#include <memory>

#include "Assets.h"
#include "../graphics/core/ShaderProgram.h"
#include "../graphics/core/Texture.h"
#include "../coders/imageio.h"
#include "../graphics/core/Font.h"
#include "../graphics/core/Atlas.h"
#include "../debug/Logger.h"
#include "../constants.h"
#include "../graphics/core/ImageData.h"
#include "asset_loaders.h"
#include "../files/engine_paths.h"
#include "../core_content_defs.h"
#include "../content/Content.h"
#include "../logic/scripting/scripting.h"
#include "../data/dynamic.h"
#include "../files/files.h"
#include "../util/ThreadPool.h"

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

aloader_func AssetsLoader::getLoader(AssetType tag) {
    auto found = loaders.find(tag);
    if (found == loaders.end()) {
        LOG_ERROR("Unknown asset tag {}", static_cast<int>(tag));
        throw std::runtime_error("Unknown asset tag " + std::to_string(static_cast<int>(tag)));
    }
    return found->second;
}

bool AssetsLoader::loadNext() {
	// Берём первый элемент очереди (не удаляя его пока)
	const aloader_entry& entry = entries.front();
    LOG_DEBUG("Loading {} as {}", entry.filename, entry.alias);
	Logger::getInstance().flush();

	// Ищем загрузчик по типу ресурса
	auto found = loaders.find(entry.tag);
	if (found == loaders.end()) {
        LOG_ERROR("Unknown asset tag {}", static_cast<int>(entry.tag));
		entries.pop();
		return false;
	}
	aloader_func loader = found->second;
	try {
        aloader_func loader = getLoader(entry.tag);
        auto postfunc = loader(this, paths, entry.filename, entry.alias, entry.config);
        postfunc(assets);
        entries.pop();
        return true;
    } catch (std::runtime_error& err) {
        LOG_ERROR("{}", err.what());
        entries.pop();
        return false;
    }
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
void addLayouts(scriptenv env, const std::string& prefix, const std::filesystem::path& folder, AssetsLoader& loader) {
    if (!std::filesystem::is_directory(folder)) return;

    for (auto& entry : std::filesystem::directory_iterator(folder)) {
        const std::filesystem::path file = entry.path();
        if (file.extension().u8string() != ".xml") continue;
        std::string name = prefix + ":" + file.stem().u8string();

		// Для каждого макета создаём LayoutConfig с указанным окружением
        loader.add(AssetType::Layout, file.u8string(), name, std::make_shared<LayoutConfig>(env));
    }
}

void AssetsLoader::tryAddSound(std::string name) {
    if (name.empty()) return;
    std::string file = SOUNDS_FOLDER + "/" + name;
    add(AssetType::Sound, file, name);
}

static std::string assets_def_folder(AssetType tag) {
    switch (tag) {
        case AssetType::Font: return FONTS_FOLDER;
        case AssetType::Shader: return SHADERS_FOLDER;
        case AssetType::Texture: return TEXTURES_FOLDER;
        case AssetType::Atlas: return TEXTURES_FOLDER;
        case AssetType::Layout: return LAYOUTS_FOLDER;
        case AssetType::Sound: return SOUNDS_FOLDER;
    }
    return "<unknown>";
}

void AssetsLoader::processPreload(
    AssetType tag, 
    const std::string& name, 
    dynamic::Map* map
) {
    std::string defFolder = assets_def_folder(tag);
    std::string path = defFolder + "/" + name;
    if (map == nullptr) {
        add(tag, path, name);
        return;
    }
    map->str("path", path);
    switch (tag) {
        case AssetType::Sound:
            add(tag, path, name, std::make_shared<SoundConfig>(
                map->get("keep-pcm", false)
            ));
            break;
        default:
            add(tag, path, name);
            break;
    }
}

void AssetsLoader::processPreloadList(AssetType tag, dynamic::List* list) {
    if (list == nullptr) return;
    for (uint i = 0; i < list->size(); ++i) {
        auto value = list->get(i);
        switch (static_cast<dynamic::ValueType>(value.index())) {
            case dynamic::ValueType::String: {
                processPreload(tag, std::get<std::string>(value), nullptr);
                break;
			} case dynamic::ValueType::Map: {
                auto map = std::get<dynamic::Map_sptr>(value);
                auto name = map->get<std::string>("name");
                processPreload(tag, name, map.get());
                break;
            } default: {
				LOG_ERROR("Invalid entry type");
                throw std::runtime_error("invalid entry type");
			}
        }
    }
}

void AssetsLoader::processPreloadConfig(std::filesystem::path file) {
    auto root = files::read_json(file);
    processPreloadList(AssetType::Font, root->list("fonts"));
    processPreloadList(AssetType::Shader, root->list("shaders"));
    processPreloadList(AssetType::Texture, root->list("textures"));
    processPreloadList(AssetType::Sound, root->list("sounds"));
    // Макеты загружаются автоматически
}

void AssetsLoader::processPreloadConfigs(const Content* content) {
    for (auto& entry : content->getPacks()) {
        const auto& pack = entry.second;
        auto preloadFile = pack->getInfo().folder/std::filesystem::path("preload.json");
        if (std::filesystem::exists(preloadFile)) {
            processPreloadConfig(preloadFile);
        }
    }
    auto preloadFile = paths->getMainRoot()/std::filesystem::path("preload.json");
    if (std::filesystem::exists(preloadFile)) {
        processPreloadConfig(preloadFile);
    }
}

void AssetsLoader::addDefaults(AssetsLoader& loader, const Content* content) {
	// Шрифт
	loader.add(AssetType::Font, FONTS_FOLDER + "/font", "normal");

	// Базовые шейдеры
	loader.add(AssetType::Shader, SHADERS_FOLDER + "/default", "default");
	loader.add(AssetType::Shader, SHADERS_FOLDER + "/ui", "ui");
	loader.add(AssetType::Shader, SHADERS_FOLDER + "/lines", "lines");

	// Интерфейсные текстуры
	loader.add(AssetType::Texture, TEXTURES_FOLDER + "/gui/menubg", "gui/menubg");
	loader.add(AssetType::Texture, TEXTURES_FOLDER + "/gui/delete_icon", "gui/delete_icon");
	loader.add(AssetType::Texture, TEXTURES_FOLDER + "/gui/no_icon", "gui/no_icon");
	loader.add(AssetType::Texture, TEXTURES_FOLDER + "/gui/warning", "gui/warning");
    loader.add(AssetType::Texture, TEXTURES_FOLDER + "/gui/error", "gui/error");
    loader.add(AssetType::Texture, TEXTURES_FOLDER + "/gui/cross", "gui/cross");
    loader.add(AssetType::Texture, TEXTURES_FOLDER + "/gui/no_world_icon", "gui/no_world_icon");
    loader.add(AssetType::Texture, TEXTURES_FOLDER + "/gui/refresh", "gui/refresh");
    loader.add(AssetType::Texture, TEXTURES_FOLDER + "/gui/circle", "gui/circle");

	if (content) {
		loader.processPreloadConfigs(content);

		for (auto& entry : content->getBlockMaterials()) {
            auto& material = entry.second;
            loader.tryAddSound(material.stepsSound);
            loader.tryAddSound(material.placeSound);
            loader.tryAddSound(material.breakSound);
        }

		// Макеты интерфейса из корневой папки "layouts" (встроенный контент)
		addLayouts(
			0, 
			BUILTIN_CONTENT_NAMESPACE, 
			loader.getPaths()->getMainRoot()/std::filesystem::path(LAYOUTS_FOLDER), 
			loader
		);

		// Макеты из каждого установленного пака
        for (auto& entry : content->getPacks()) {
			auto pack = entry.second.get();
            auto& info = pack->getInfo();
            std::filesystem::path folder = info.folder/std::filesystem::path(LAYOUTS_FOLDER);
            addLayouts(pack->getEnvironment(), info.id, folder, loader);
        }
	}

	// Атласы блоков и предметов
	loader.add(AssetType::Atlas, TEXTURES_FOLDER + "/blocks", "blocks");
	loader.add(AssetType::Atlas, TEXTURES_FOLDER + "/items", "items");
}

bool AssetsLoader::loadExternalTexture(
    Assets* assets,
    const std::string& name,
    std::vector<std::filesystem::path> alternatives)
{
    if (assets->getTexture(name) != nullptr) return true;

    for (auto& path : alternatives) {
        if (std::filesystem::exists(path)) {
            try {
                auto image = imageio::read(path.string());
                assets->store(Texture::from(image.get()).release(), name);
                return true;
            } catch (const std::exception& err) {
                LOG_ERROR("Error while loading external '{}': {}", path.u8string(), err.what());
            }
        }
    }
    return false;
}

const ResPaths* AssetsLoader::getPaths() const {
	return paths;
}

class LoaderWorker : public util::Worker<aloader_entry, asset_loader::postfunc> {
    AssetsLoader* loader;
public:
    LoaderWorker(AssetsLoader* loader) : loader(loader) {
    }

    asset_loader::postfunc operator()(const std::shared_ptr<aloader_entry>& entry) override {
        aloader_func loadfunc = loader->getLoader(entry->tag);
        return loadfunc(loader, loader->getPaths(), entry->filename, entry->alias, entry->config);
    }
};

std::shared_ptr<Task> AssetsLoader::startTask(runnable onDone) {
    auto pool = std::make_shared<
        util::ThreadPool<aloader_entry, asset_loader::postfunc>
    >(
        "assets-loader-pool", 
        [=](){return std::make_shared<LoaderWorker>(this);},
        [=](asset_loader::postfunc& func) {
            func(assets);
        }
    );
    pool->setOnComplete(onDone);
    while (!entries.empty()) {
        const aloader_entry& entry = entries.front();
        auto ptr = std::make_shared<aloader_entry>(entry);
        pool->enqueueJob(ptr);
        entries.pop();
    }
    return pool;
}
