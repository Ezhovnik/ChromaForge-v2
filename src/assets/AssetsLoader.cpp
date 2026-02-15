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

AssetsLoader::AssetsLoader(Assets* assets, std::filesystem::path resdir) : assets(assets), resdir(resdir) {
}

void AssetsLoader::addLoader(AssetType tag, aloader_func func) {
	loaders[tag] = func;
}

void AssetsLoader::add(AssetType tag, const std::filesystem::path filename, const std::string alias) {
	entries.push(aloader_entry{tag, filename, alias});
}

bool AssetsLoader::hasNext() const {
	return !entries.empty();
}

bool AssetsLoader::loadNext() {
	const aloader_entry& entry = entries.front();
    LOG_INFO("Loading {} as {}", entry.filename.string(), entry.alias);
	Logger::getInstance().flush();
	auto found = loaders.find(entry.tag);
	if (found == loaders.end()) {
        LOG_ERROR("Unknown asset tag {}", (int)entry.tag);
        Logger::getInstance().flush();
		return false;
	}
	aloader_func loader = found->second;
	bool status = loader(assets, entry.filename, entry.alias);
	entries.pop();
	return status;
}

// Загружает и регистрирует шейдерную программу в менеджере ресурсов.
bool _load_shader(Assets* assets, const std::filesystem::path& filename, const std::string& name){
	ShaderProgram* shader = loadShaderProgram(filename.string() + ".vert", filename.string() + ".frag");
	if (shader == nullptr){
        LOG_CRITICAL("Failed to load shader '{}'", name);
        Logger::getInstance().flush();
		return false;
	}

	return assets->store(shader, name);
}

// Загружает и регистрирует текстуру в менеджере ресурсов.
bool _load_texture(Assets* assets, const std::filesystem::path& filename, const std::string& name){
	Texture* texture = png::loadTexture(filename.string());
	if (texture == nullptr){
		LOG_CRITICAL("Failed to load texture '{}'", name);
        Logger::getInstance().flush();
		return false;
	}

	return assets->store(texture, name);
}

bool _load_font(Assets* assets, const std::filesystem::path& filename, const std::string& name){
    std::vector<Texture*> pages;
	for (size_t i = 0; i <= 4; ++i){
		Texture* texture = png::loadTexture(filename.string() + "_" + std::to_string(i)+  ".png");
		if (texture == nullptr){
            LOG_CRITICAL("Failed to load bitmap font '{}' (missing page {})", name, std::to_string(i));
            Logger::getInstance().flush();
			return false;
		}
		pages.push_back(texture);
	}
	Font* font = new Font(pages, pages[0]->height / 16);

	return assets->store(font, name);;
}

bool _load_atlas(Assets* assets, const std::filesystem::path& directory, const std::string& name) {
	if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        LOG_ERROR("Directory named '{}' not found", directory.string());
		Logger::getInstance().flush();
        return false;
    }

	AtlasBuilder builder;
	for (auto const& entry : std::filesystem::directory_iterator(directory)) {
		std::filesystem::path file = entry.path();
		if (file.extension() == ".png") {
			std::string entry_name = file.stem().string();
			std::shared_ptr<ImageData> image(png::loadImage(file.string()));
			if (image == nullptr) {
				LOG_ERROR("Failed to load atlas entry '{}'", entry_name);
				Logger::getInstance().flush();
				continue;
			}

			if (image->getFormat() != ImageFormat::rgba8888) image.reset(toRGBA(image.get()));

			image->fixAlphaColor();
			builder.add(entry_name, std::move(image));
		}
	}

	Atlas* atlas = builder.build(2);
	return assets->store(atlas, name);
}

void AssetsLoader::createDefaults(AssetsLoader& loader) {
	loader.addLoader(AssetType::Shader, _load_shader);
	loader.addLoader(AssetType::Texture, _load_texture);
	loader.addLoader(AssetType::Font, _load_font);
    loader.addLoader(AssetType::Atlas, _load_atlas);
}

void AssetsLoader::addDefaults(AssetsLoader& loader) {
	std::filesystem::path resdir = loader.getDirectory();

	loader.add(AssetType::Shader, resdir/std::filesystem::path(SHADERS_FOLDER"/default"), "default");
	loader.add(AssetType::Shader, resdir/std::filesystem::path(SHADERS_FOLDER"/lines"), "lines");
	loader.add(AssetType::Shader, resdir/std::filesystem::path(SHADERS_FOLDER"/ui"), "ui");
	loader.add(AssetType::Shader, resdir/std::filesystem::path(SHADERS_FOLDER"/skybox_gen"), "skybox_gen");
	loader.add(AssetType::Shader, resdir/std::filesystem::path(SHADERS_FOLDER"/background"), "background");

	loader.add(AssetType::Atlas, resdir/std::filesystem::path(TEXTURES_FOLDER"/blocks"), "blocks");
    loader.add(AssetType::Texture, resdir/std::filesystem::path(TEXTURES_FOLDER"/menubg.png"), "menubg");

	loader.add(AssetType::Font, resdir/std::filesystem::path(FONTS_FOLDER"/font"), "normal");
}

std::filesystem::path AssetsLoader::getDirectory() const {
	return resdir;
}
