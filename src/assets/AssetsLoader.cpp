#include "AssetsLoader.h"

#include <filesystem>
#include <memory>

#include "Assets.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Texture.h"
#include "../coders/png.h"
#include "../graphics/Font.h"
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

bool _load_atlas(Assets* assets, const std::string& filename, const std::string& name) {
	std::unique_ptr<ImageData> image (png::loadImage(filename));
	if (image == nullptr) {
		LOG_ERROR("Failed to load image '{}'", name);
		return false;
	}
	for (int i = 0; i < ATLAS_MARGIN_SIZE; i++) {
		ImageData* newimage = add_atlas_margins(image.get(), 16);
		image.reset(newimage);
	}

	Texture* texture = Texture::from(image.get());
	return assets->store(texture, name);
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

	loader.add(AssetType::Atlas, TEXTURES_FOLDER"/atlas.png", "blocks");
	loader.add(AssetType::Texture, TEXTURES_FOLDER"/atlas.png", "blocks_tex");
	loader.add(AssetType::Texture, TEXTURES_FOLDER"/slot.png", "slot");
    loader.add(AssetType::Texture, TEXTURES_FOLDER"/menubg.png", "menubg");

	loader.add(AssetType::Font, FONTS_FOLDER"/font", "normal");
}
