#include "asset_loaders.h"

#include <filesystem>

#include "Assets.h"
#include "../coders/png.h"
#include "../files/files.h"
#include "../graphics/ShaderProgram.h"
#include "../graphics/Texture.h"
#include "../graphics/ImageData.h"
#include "../graphics/Atlas.h"
#include "../graphics/Font.h"
#include "../logger/Logger.h"
#include "../files/engine_paths.h"
#include "../coders/json.h"
#include "../graphics/TextureAnimation.h"

// Загружает и регистрирует шейдерную программу в менеджере ресурсов.
bool asset_loader::shader(Assets* assets, const ResPaths* paths, const std::string filename, const std::string name){
    std::filesystem::path vertexFile = paths->find(filename + ".vert");
    std::filesystem::path fragmentFile = paths->find(filename + ".frag");

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
bool asset_loader::texture(Assets* assets, const ResPaths* paths, const std::string filename, const std::string name){
	std::unique_ptr<Texture> texture(png::loadTexture(paths->find(filename).string()));
	if (texture == nullptr){
		LOG_CRITICAL("Failed to load texture '{}'", name);
        Logger::getInstance().flush();
		return false;
	}

	return assets->store(texture.release(), name);
}

// Загружает и регистрирует шрифт в менеджере ресурсов
bool asset_loader::font(Assets* assets, const ResPaths* paths, const std::string filename, const std::string name){
    std::vector<std::unique_ptr<Texture>> pages;
	for (size_t i = 0; i <= 4; ++i) {
		std::string name = filename + "_" + std::to_string(i) + ".png"; 
        name = paths->find(name).string();
		std::unique_ptr<Texture> texture(png::loadTexture(name));
		if (texture == nullptr){
            LOG_CRITICAL("Failed to load bitmap font '{}' (missing page {})", name, std::to_string(i));
            Logger::getInstance().flush();
			return false;
		}
		pages.push_back(std::move(texture));
	}

	int res = pages[0]->height / 16;
	return assets->store(new Font(std::move(pages), res, 4), name);
}

static bool appendAtlas(AtlasBuilder& atlas, const std::filesystem::path& file) {
	if (file.extension() != ".png") return false;
	std::string name = file.stem().string();
	if (atlas.has(name)) return false;
	std::shared_ptr<ImageData> image(png::loadImage(file.string()));
	if (image == nullptr) {
		LOG_ERROR("Failed to load atlas entry '{}'", name);
		Logger::getInstance().flush();
		return false;
	}

	if (image->getFormat() != ImageFormat::rgba8888) image.reset(toRGBA(image.get()));

	image->fixAlphaColor();
	atlas.add(name, std::move(image));

	return true;
}

// Загружает и регистрирует атлас в менеджере ресурсов
bool asset_loader::atlas(Assets* assets, const ResPaths* paths, const std::string directory, const std::string name) {
	AtlasBuilder builder;
	for (auto const& file : paths->listdir(directory)) {
		if (!appendAtlas(builder, file)) continue;
	}

	Atlas* atlas = builder.build(2);
	assets->store(atlas, name);

	for (const auto& file : builder.getNames()) {
		asset_loader::animation(assets, paths, "textures", file, atlas);
	}

	return true;
}

bool asset_loader::animation(Assets* assets, const ResPaths* paths, const std::string directory, const std::string name, Atlas* dstAtlas) {
	std::string animsDir = directory + "/animations";
	std::string blocksDir = directory + "/blocks";

	for (const auto& folder : paths->listdir(animsDir)) {
		if (!std::filesystem::is_directory(folder)) continue;
		if (folder.filename().string() != name) continue;
		if (std::filesystem::is_empty(folder)) continue;

		AtlasBuilder builder;
		appendAtlas(builder, paths->find(blocksDir + "/" + name + ".png"));

		std::string animFile = folder.string() + "/animation.json";

		std::vector<std::pair<std::string, float>> frameList;

		if (std::filesystem::exists(animFile)) {
			auto root = files::read_json(animFile);

			auto frameArr = root->list("frames");

			float frameDuration = DEFAULT_FRAME_DURATION;
			std::string frameName;

			if (frameArr) {
				for (size_t i = 0; i < frameArr->size(); ++i) {
					auto currentFrame = frameArr->list(i);

					frameName = currentFrame->str(0);
					if (currentFrame->size() > 1) frameDuration = static_cast<float>(currentFrame->integer(1)) / 1000;

					frameList.emplace_back(frameName, frameDuration);
				}
			}
		}
		for (const auto& file : paths->listdir(animsDir + "/" + name)) {
			if (!frameList.empty()) {
				bool contains = false;
				for (const auto& elem : frameList) {
					if (file.stem() == elem.first) {
						contains = true;
						break;
					}
				}
				if (!contains) continue;
			}
			if (!appendAtlas(builder, file)) continue;
		}

		std::unique_ptr<Atlas> srcAtlas(builder.build(2));

		Texture* srcTex = srcAtlas->getTexture();
		Texture* dstTex = dstAtlas->getTexture();

		TextureAnimation animation(srcTex, dstTex);
		Frame frame;
		UVRegion region = dstAtlas->get(name);

		frame.dstPos = glm::ivec2(region.u1 * dstTex->width, region.v1 * dstTex->height);
		frame.size = glm::ivec2(region.u2 * dstTex->width, region.v2 * dstTex->height) - frame.dstPos;

		if (frameList.empty()) {
			for (const auto& elem : builder.getNames()) {
				region = srcAtlas->get(elem);
				frame.srcPos = glm::ivec2(region.u1 * srcTex->width, srcTex->height - region.v2 * srcTex->height);
				animation.addFrame(frame);
			}
		} else {
			for (const auto& elem : frameList) {
				if (!srcAtlas->has(elem.first)) {
					LOG_ERROR("Unknown frame name: {}", elem.first);
					continue;
				}
				region = srcAtlas->get(elem.first);
				frame.duration = elem.second;
				frame.srcPos = glm::ivec2(region.u1 * srcTex->width, srcTex->height - region.v2 * srcTex->height);
				animation.addFrame(frame);
			}
		}

		assets->store(srcAtlas.release(), name + "_animation");
		assets->store(animation);

		return true;
	}
	return true;
}
