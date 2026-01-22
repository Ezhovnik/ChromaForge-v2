#include "AssetsLoader.h"

#include <iostream>

#include "Assets.h"
#include "../constants.h"
#include "../graphics/ImageData.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Texture.h"
#include "../coders/png.h"
#include "../graphics/Font.h"
#include "../logger/Logger.h"

AssetsLoader::AssetsLoader(Assets* assets) : assets(assets) {
}

void AssetsLoader::addLoader(AssetsType tag, aloader_func func) {
	loaders[tag] = func;
}

void AssetsLoader::add(AssetsType tag, const std::string filename, const std::string alias) {
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
        LOG_ERROR("Unknown asset tag {}", static_cast<int>(entry.tag));
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
		return false;
	}

	return assets->store(shader, name);
}

// Загружает и регистрирует текстуру в менеджере ресурсов.
bool _load_texture(Assets* assets, const std::string& filename, const std::string& name){
	Texture* texture = png::loadTexture(filename);
	if (texture == nullptr){
		LOG_CRITICAL("Failed to load texture '{}'", name);
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
			return false;
		}
		pages.push_back(texture);
	}
	Font* font = new Font(pages, pages[0]->height / 16);

	return assets->store(font, name);
}

bool _load_atlas(Assets* assets, const std::string& filename, const std::string& name) {
    ImageData* image = png::loadImage(filename);
    if (image == nullptr) {
        LOG_CRITICAL("Failed to load image '{}'", filename);
        return false;
    }

    for (int i = 0; i < ATLAS_MARGIN_SIZE; ++i) {
        ImageData* newImage = add_atlas_margins(image, 16);
        delete image;
        image = newImage;
    }

    Texture* texture = Texture::from(image);
    return assets->store(texture, name);
}

void AssetsLoader::createDefaults(AssetsLoader& loader) {
	loader.addLoader(AssetsType::Shader, _load_shader);
	loader.addLoader(AssetsType::Texture, _load_texture);
	loader.addLoader(AssetsType::Font, _load_font);
    loader.addLoader(AssetsType::Atlas, _load_atlas);
}

void AssetsLoader::addDefaults(AssetsLoader& loader) {
    loader.add(AssetsType::Shader, "../res/shaders/default", "default");
    loader.add(AssetsType::Shader, "../res/shaders/lines", "lines");
    loader.add(AssetsType::Shader, "../res/shaders/ui", "ui");

    loader.add(AssetsType::Atlas, "../res/textures/atlas.png", "blocks");
    loader.add(AssetsType::Texture, "../res/textures/atlas.png", "blocks_tex");

    loader.add(AssetsType::Texture, "../res/textures/slot.png", "slot");

    loader.add(AssetsType::Font, "../res/fonts/font", "normal");
}
