#include "asset_loaders.h"

#include "Assets.h"
#include "../coders/png.h"
#include "../files/files.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Texture.h"
#include "../graphics/ImageData.h"
#include "../graphics/Atlas.h"
#include "../graphics/Font.h"
#include "../logger/Logger.h"

// Загружает и регистрирует шейдерную программу в менеджере ресурсов.
bool asset_loader::shader(Assets* assets, const std::filesystem::path filename, const std::string name){
    std::filesystem::path vertexFile = std::filesystem::path(filename.string() + ".vert");
    std::filesystem::path fragmentFile = std::filesystem::path(filename.string() + ".frag");

    std::string vertexSource = files::read_string(vertexFile);
    std::string fragmentSource = files::read_string(fragmentFile);

	ShaderProgram* shader = ShaderProgram::loadShaderProgram(vertexFile.string(), fragmentFile.string(), vertexSource, fragmentSource);
	if (shader == nullptr){
        LOG_CRITICAL("Failed to load shader '{}'", name);
        Logger::getInstance().flush();
		return false;
	}

	return assets->store(shader, name);
}

// Загружает и регистрирует текстуру в менеджере ресурсов.
bool asset_loader::texture(Assets* assets, const std::filesystem::path filename, const std::string name){
	Texture* texture = png::loadTexture(filename.string());
	if (texture == nullptr){
		LOG_CRITICAL("Failed to load texture '{}'", name);
        Logger::getInstance().flush();
		return false;
	}

	return assets->store(texture, name);
}

bool asset_loader::font(Assets* assets, const std::filesystem::path filename, const std::string name){
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

bool asset_loader::atlas(Assets* assets, const std::filesystem::path directory, const std::string name) {
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
