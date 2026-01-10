#include "AssetsLoader.h"

#include <iostream>

#include "Assets.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "graphics/Font.h"
#include "logger/Logger.h"

AssetsLoader::AssetsLoader(Assets* assets) : assets(assets) {
}

void AssetsLoader::addLoader(int tag, aloader_func func) {
	loaders[tag] = func;
}

void AssetsLoader::add(int tag, const std::string filename, const std::string alias) {
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
        LOG_ERROR("Unknown asset tag {}", entry.tag);
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
	Texture* texture = loadTexture(filename);
	if (texture == nullptr){
		LOG_CRITICAL("Failed to load texture '{}'", name);
		return false;
	}

	return assets->store(texture, name);
}

bool _load_font(Assets* assets, const std::string& filename, const std::string& name){
    std::vector<Texture*> pages;
	for (size_t i = 0; i <= 4; ++i){
		Texture* texture = loadTexture(filename + "_" + std::to_string(i)+  ".png");
		if (texture == nullptr){
            LOG_CRITICAL("Failed to load bitmap font '{}' (missing page {})", name, std::to_string(i));
			return false;
		}
		pages.push_back(texture);
	}
	Font* font = new Font(pages);

	return assets->store(font, name);;
}

void AssetsLoader::createDefaults(AssetsLoader& loader) {
	loader.addLoader(ASSETS_TYPE::SHADER, _load_shader);
	loader.addLoader(ASSETS_TYPE::TEXTURE, _load_texture);
	loader.addLoader(ASSETS_TYPE::FONT, _load_font);
}
