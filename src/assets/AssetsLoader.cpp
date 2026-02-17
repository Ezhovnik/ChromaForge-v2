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
#include "asset_loaders.h"

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
    LOG_DEBUG("Loading {} as {}", entry.filename.string(), entry.alias);
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

void AssetsLoader::createDefaults(AssetsLoader& loader) {
	loader.addLoader(AssetType::Shader, asset_loader::shader);
	loader.addLoader(AssetType::Texture, asset_loader::texture);
	loader.addLoader(AssetType::Font, asset_loader::font);
    loader.addLoader(AssetType::Atlas, asset_loader::atlas);
}

void AssetsLoader::addDefaults(AssetsLoader& loader) {
	std::filesystem::path resdir = loader.getDirectory();

	loader.add(AssetType::Shader, resdir/std::filesystem::path(SHADERS_FOLDER"/default"), "default");
	loader.add(AssetType::Shader, resdir/std::filesystem::path(SHADERS_FOLDER"/lines"), "lines");
	loader.add(AssetType::Shader, resdir/std::filesystem::path(SHADERS_FOLDER"/ui"), "ui");
	loader.add(AssetType::Shader, resdir/std::filesystem::path(SHADERS_FOLDER"/skybox_gen"), "skybox_gen");
	loader.add(AssetType::Shader, resdir/std::filesystem::path(SHADERS_FOLDER"/background"), "background");
	loader.add(AssetType::Shader, resdir/std::filesystem::path(SHADERS_FOLDER"/ui3d"), "ui3d");

	loader.add(AssetType::Atlas, resdir/std::filesystem::path(TEXTURES_FOLDER"/blocks"), "blocks");
    loader.add(AssetType::Texture, resdir/std::filesystem::path(TEXTURES_FOLDER"/menubg.png"), "menubg");

	loader.add(AssetType::Font, resdir/std::filesystem::path(FONTS_FOLDER"/font"), "normal");
}

std::filesystem::path AssetsLoader::getDirectory() const {
	return resdir;
}
