#include "AssetsLoader.h"

#include <filesystem>
#include <memory>
#include <iostream>

#include "Assets.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Texture.h"
#include "../coders/png.h"
#include "../graphics/Font.h"
#include "../graphics/Atlas.h"
#include "../logger/Logger.h"
#include "../constants.h"
#include "../graphics/ImageData.h"

AssetsLoader::AssetsLoader(Assets* assets) : assets(assets) {
}

void AssetsLoader::addLoader(AssetType tag, aloader_func func) {
	loaders[tag] = func;
}

void AssetsLoader::add(AssetType tag, const std::string filename, const std::string alias) {
	entries.push(aloader_entry{tag, filename, alias});
}

bool AssetsLoader::hasNext() const {
	return !entries.empty();
}

bool AssetsLoader::loadNext() {
	const aloader_entry& entry = entries.front();
    LOG_INFO("Loading {} as {}", entry.filename, entry.alias);
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
bool _load_shader(Assets* assets, const std::string& filename, const std::string& name){
	ShaderProgram* shader = loadShaderProgram(filename + ".vert", filename + ".frag");
	if (shader == nullptr){
        LOG_CRITICAL("Failed to load shader '{}'", name);
        Logger::getInstance().flush();
		return false;
	}

	return assets->store(shader, name);
}

// Загружает и регистрирует текстуру в менеджере ресурсов.
bool _load_texture(Assets* assets, const std::string& filename, const std::string& name){
	Texture* texture = png::loadTexture(filename);
	if (texture == nullptr){
		LOG_CRITICAL("Failed to load texture '{}'", name);
        Logger::getInstance().flush();
		return false;
	}

	return assets->store(texture, name);
}

bool _load_font(Assets* assets, const std::string& filename, const std::string& name){
    std::vector<Texture*> pages;
	for (size_t i = 0; i <= 4; ++i){
		Texture* texture = png::loadTexture(filename + "_" + std::to_string(i)+  ".png");
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

bool _load_atlas(Assets* assets, const std::string& directory, const std::string& name) {
	if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        LOG_ERROR("Directory named '{}' not found", directory);
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
	loader.add(AssetType::Shader, SHADERS_FOLDER"/default", "default");
	loader.add(AssetType::Shader, SHADERS_FOLDER"/lines", "lines");
	loader.add(AssetType::Shader, SHADERS_FOLDER"/ui", "ui");

	loader.add(AssetType::Atlas, TEXTURES_FOLDER"/blocks", "blocks");
    loader.add(AssetType::Texture, TEXTURES_FOLDER"/menubg.png", "menubg");

	loader.add(AssetType::Font, FONTS_FOLDER"/font", "normal");
}
