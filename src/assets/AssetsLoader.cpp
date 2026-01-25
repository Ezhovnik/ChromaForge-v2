#include "AssetsLoader.h"

#include <memory>

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

// Регистрирует функцию-загрузчик для определенного типа ресурсов
void AssetsLoader::addLoader(AssetsType tag, aloader_func func) {
	loaders[tag] = func;
}

// Добавляет ресурс в очередь на загрузку
void AssetsLoader::add(AssetsType tag, const std::string filename, const std::string alias) {
	entries.push(aloader_entry{tag, filename, alias});
}

// Проверяет, есть ли еще ресурсы в очереди на загрузку.
bool AssetsLoader::hasNext() const {
	return !entries.empty();
}

bool AssetsLoader::loadNext() {
    if (entries.empty()) {
        LOG_WARN("Attempted to load next asset from empty queue");
        return false;
    }

	const aloader_entry& entry = entries.front();
    LOG_INFO("Loading {} as {}", entry.filename, entry.alias);
	Logger::getInstance().flush();
	auto loaderIt = loaders.find(entry.tag);
	if (loaderIt == loaders.end()) {
        LOG_ERROR("Unknown asset tag {} for file '{}'", static_cast<int>(entry.tag), entry.filename);
		return false;
	}
	aloader_func loader = loaderIt->second;
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
    constexpr size_t FONT_PAGE_COUNT = 5;
    std::vector<Texture*> pages;
    pages.reserve(FONT_PAGE_COUNT);

	for (size_t i = 0; i < FONT_PAGE_COUNT; ++i){
		Texture* texture = png::loadTexture(filename + "_" + std::to_string(i)+  ".png");
		if (texture == nullptr){
            LOG_CRITICAL("Failed to load bitmap font '{}' (missing page {})", name, std::to_string(i));

            for (auto* page : pages) {
                delete page;
            }

            return false;
		}
		pages.push_back(texture);
        LOG_TRACE("Loaded font page {} for '{}'", i, name);
	}
	Font* font = new Font(pages, pages[0]->height / 16);

	return assets->store(font, name);
}

// Загружает и создает текстуру-атлас с добавлением отступов.
bool _load_atlas(Assets* assets, const std::string& filename, const std::string& name) {
    std::unique_ptr<ImageData> image(png::loadImage(filename));
    if (image == nullptr) {
        LOG_CRITICAL("Failed to load atlas image '{}'", filename);
        return false;
    }

    for (int i = 0; i < ATLAS_MARGIN_SIZE; ++i) {
        ImageData* newImage = add_atlas_margins(image.get(), 16);
        image.reset(newImage);
    }

    Texture* texture = Texture::from(image.get());
    return assets->store(texture, name);
}

// Настраивает стандартные загрузчики для всех типов ресурсов.
void AssetsLoader::createDefaults(AssetsLoader& loader) {
	loader.addLoader(AssetsType::Shader, _load_shader);
	loader.addLoader(AssetsType::Texture, _load_texture);
	loader.addLoader(AssetsType::Font, _load_font);
    loader.addLoader(AssetsType::Atlas, _load_atlas);

    LOG_DEBUG("Created default asset loaders");
}

// Добавляет стандартные ресурсы в очередь загрузки.
void AssetsLoader::addDefaults(AssetsLoader& loader) {
    loader.add(AssetsType::Shader, SHADERS_FOLDER"/default", "default");
    loader.add(AssetsType::Shader, SHADERS_FOLDER"/lines", "lines");
    loader.add(AssetsType::Shader, SHADERS_FOLDER"/ui", "ui");

    loader.add(AssetsType::Atlas, TEXTURES_FOLDER"/atlas.png", "blocks");
    loader.add(AssetsType::Texture, TEXTURES_FOLDER"/atlas.png", "blocks_tex");
    loader.add(AssetsType::Texture, TEXTURES_FOLDER"/menubg.png", "menubg");

    loader.add(AssetsType::Texture, TEXTURES_FOLDER"/slot.png", "slot");

    loader.add(AssetsType::Font, FONTS_FOLDER"/font", "normal");

    LOG_DEBUG("Added default assets to loading queue");
}
