#include <assets/AssetsLoader.h>

#include <memory>

#include <assets/Assets.h>
#include <graphics/core/ShaderProgram.h>
#include <graphics/core/Texture.h>
#include <coders/imageio.h>
#include <graphics/core/Font.h>
#include <graphics/core/Atlas.h>
#include <debug/Logger.h>
#include <constants.h>
#include <graphics/core/ImageData.h>
#include <assets/asset_loaders.h>
#include <io/engine_paths.h>
#include <core_content_defs.h>
#include <content/Content.h>
#include <logic/scripting/scripting.h>
#include <io/io.h>
#include <util/ThreadPool.h>
#include <voxels/Block.h>
#include <objects/rigging.h>
#include <items/Item.h>
#include <engine/Engine.h>

AssetsLoader::AssetsLoader(
    Engine& engine, Assets& assets, const ResPaths* paths
) : engine(engine), assets(assets), paths(paths) {
	// Регистрируем встроенные загрузчики из asset_loaders.h
	addLoader(AssetType::Shader, asset_loader::shader);
	addLoader(AssetType::Texture, asset_loader::texture);
	addLoader(AssetType::Font, asset_loader::font);
    addLoader(AssetType::Atlas, asset_loader::atlas);
	addLoader(AssetType::Layout, asset_loader::layout);
	addLoader(AssetType::Sound, asset_loader::sound);
    addLoader(AssetType::Model, asset_loader::model);
}

void AssetsLoader::addLoader(AssetType tag, aloader_func func) {
	loaders[tag] = std::move(func);
}

void AssetsLoader::add(
    AssetType tag,
    const std::string& filename,
    const std::string& alias,
    std::shared_ptr<AssetsConfig> config
) {
    if (enqueued.find({tag, alias}) != enqueued.end()) return;
	entries.push(aloader_entry{tag, filename, alias, std::move(config)});
    enqueued.insert({tag, alias});
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

void AssetsLoader::loadNext() {
	// Берём первый элемент очереди (не удаляя его пока)
	const aloader_entry& entry = entries.front();
    LOG_DEBUG("Loading {} as {}", entry.filename, entry.alias);
	Logger::getInstance().flush();

	// Ищем загрузчик по типу ресурса
	auto found = loaders.find(entry.tag);
	if (found == loaders.end()) {
        LOG_ERROR("Unknown asset tag {}", static_cast<int>(entry.tag));
		entries.pop();
	}
	aloader_func loader = found->second;
	try {
        aloader_func loader = getLoader(entry.tag);
        auto postfunc = loader(this, paths, entry.filename, entry.alias, entry.config);
        postfunc(&assets);
        entries.pop();
    } catch (std::runtime_error& err) {
        LOG_ERROR("{}", err.what());
        auto type = entry.tag;
        std::string filename = entry.filename;
        std::string reason = err.what();
        entries.pop();
        throw asset_loader::error(type, std::move(filename), std::move(reason));
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
static void add_layouts(
    const scriptenv& env,
    const std::string& prefix,
    const io::path& folder,
    AssetsLoader& loader
) {
    if (!io::is_directory(folder)) return;

    for (const auto& file : io::directory_iterator(folder)) {
        if (file.extension() != ".xml") continue;
        std::string name = prefix + ":" + file.stem();

		// Для каждого макета создаём LayoutConfig с указанным окружением
        loader.add(
            AssetType::Layout,
            file.string(),
            name,
            std::make_shared<LayoutConfig>(&loader.getEngine().getGUI(), env)
        );
    }
}

void AssetsLoader::tryAddSound(const std::string& name) {
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
        case AssetType::Model: return MODELS_FOLDER;
    }
    return "<unknown>";
}

void AssetsLoader::processPreload(
    AssetType tag, 
    const std::string& name, 
    const dv::value& map
) {
    std::string defFolder = assets_def_folder(tag);
    std::string path = defFolder + "/" + name;
    if (map == nullptr) {
        add(tag, path, name);
        return;
    }
    map.at("path").get(path);
    switch (tag) {
        case AssetType::Sound: {
            bool keepPCM = false;
            add(tag,
                path,
                name,
                std::make_shared<SoundConfig>(map.at("keep-pcm").get(keepPCM))
            );
            break;
        }
        case AssetType::Atlas: {
            std::string typeName = "atlas";
            map.at("type").get(typeName);
            auto type = AtlasType::Atlas;
            if (typeName == "separate") type = AtlasType::Separate;
            add(tag, path, name, std::make_shared<AtlasConfig>(type));
            break;
        }
        default:
            add(tag, path, name);
            break;
    }
}

void AssetsLoader::processPreloadList(AssetType tag, const dv::value& list) {
    if (list == nullptr) return;
    for (const auto& value : list) {
        switch (value.getType()) {
            case dv::value_type::String:
                processPreload(tag, value.asString(), nullptr);
                break;
			case dv::value_type::Object:
                processPreload(tag, value["name"].asString(), value);
                break;
            default:
				LOG_ERROR("Invalid entry type");
                throw std::runtime_error("invalid entry type");
        }
    }
}

void AssetsLoader::processPreloadConfig(const io::path& file) {
    auto root = io::read_json(file);
    processPreloadList(AssetType::Atlas, root["atlases"]);
    processPreloadList(AssetType::Font, root["fonts"]);
    processPreloadList(AssetType::Shader, root["shaders"]);
    processPreloadList(AssetType::Texture, root["textures"]);
    processPreloadList(AssetType::Sound, root["sounds"]);
    processPreloadList(AssetType::Model, root["models"]);
    // Макеты загружаются автоматически
}

void AssetsLoader::processPreloadConfigs(const Content* content) {
    auto preloadFile = paths->getMainRoot() / "preload.json";
    if (io::exists(preloadFile)) {
        processPreloadConfig(preloadFile);
    }
    if (content == nullptr) return;
    for (auto& entry : content->getPacks()) {
        if (entry.first == BUILTIN_CONTENT_NAMESPACE) continue;
        const auto& pack = entry.second;
        auto preloadFile = pack->getInfo().folder / "preload.json";
        if (io::exists(preloadFile)) {
            processPreloadConfig(preloadFile);
        }
    }
}

void AssetsLoader::addDefaults(AssetsLoader& loader, const Content* content) {
	loader.processPreloadConfigs(content);

	if (content) {
		for (auto& entry : content->getBlockMaterials()) {
            auto& material = *entry.second;
            loader.tryAddSound(material.stepsSound);
            loader.tryAddSound(material.placeSound);
            loader.tryAddSound(material.breakSound);
            loader.tryAddSound(material.hitSound);
        }

		// Макеты из каждого установленного пака
        for (auto& entry : content->getPacks()) {
			auto pack = entry.second.get();
            auto& info = pack->getInfo();
            io::path folder = info.folder / LAYOUTS_FOLDER;
            add_layouts(pack->getEnvironment(), info.id, folder, loader);
        }

        for (auto& entry : content->getSkeletons()) {
            auto& skeleton = *entry.second;
            for (auto& bone : skeleton.getBones()) {
                std::string model = bone->model.name;
                size_t pos = model.rfind('.');
                if (pos != std::string::npos) {
                    model = model.substr(0, pos);
                }
                if (!model.empty()) {
                    loader.add(AssetType::Model, MODELS_FOLDER + "/" + model, model);
                }
            }
        }
        for (const auto& [_, def] : content->blocks.getDefs()) {
            if (!def->modelName.empty() && def->modelName.find(':') == std::string::npos) {
                loader.add(
                    AssetType::Model,
                    MODELS_FOLDER + "/" + def->modelName,
                    def->modelName
                );
            }
        }
        for (const auto& [_, def] : content->items.getDefs()) {
            if (def->modelName.find(':') == std::string::npos) {
                loader.add(
                    AssetType::Model,
                    MODELS_FOLDER + "/" + def->modelName,
                    def->modelName
                );
            }
        }
	}
}

bool AssetsLoader::loadExternalTexture(
    Assets* assets,
    const std::string& name,
    const std::vector<io::path>& alternatives)
{
    if (assets->get<Texture>(name) != nullptr) return true;

    for (auto& path : alternatives) {
        if (io::exists(path)) {
            try {
                auto image = imageio::read(path);
                assets->store(Texture::from(image.get()), name);
                return true;
            } catch (const std::exception& err) {
                LOG_ERROR("Error while loading external '{}': {}", path.string(), err.what());
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

    asset_loader::postfunc operator()(const aloader_entry& entry) override {
        aloader_func loadfunc = loader->getLoader(entry.tag);
        return loadfunc(
            loader,
            loader->getPaths(),
            entry.filename,
            entry.alias,
            entry.config
        );
    }
};

std::shared_ptr<Task> AssetsLoader::startTask(runnable onDone) {
    auto pool = std::make_shared<
        util::ThreadPool<aloader_entry, asset_loader::postfunc>
    >(
        "assets-loader-pool", 
        [=](){return std::make_shared<LoaderWorker>(this);},
        [this](const asset_loader::postfunc& func) {
            func(&assets);
        }
    );
    pool->setOnComplete(std::move(onDone));
    while (!entries.empty()) {
        aloader_entry entry = std::move(entries.front());
        entries.pop();
        pool->enqueueJob(std::move(entry));
    }
    return pool;
}

Engine& AssetsLoader::getEngine() {
    return engine;
}
