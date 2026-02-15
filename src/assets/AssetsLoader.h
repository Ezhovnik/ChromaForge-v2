#ifndef ASSETS_ASSETS_LOADER_H_
#define ASSETS_ASSETS_LOADER_H_

#include <string>
#include <functional>
#include <map>
#include <queue>
#include <filesystem>

enum class AssetType {
    Texture, Shader, Font, Atlas
};

class Assets;

typedef std::function<bool(Assets*, const std::filesystem::path&, const std::string&)> aloader_func;

struct aloader_entry {
	AssetType tag;
	const std::filesystem::path filename;
	const std::string alias;
};

class AssetsLoader {
	Assets* assets;
	std::map<AssetType, aloader_func> loaders;
	std::queue<aloader_entry> entries;

	std::filesystem::path resdir;
public:
	AssetsLoader(Assets* assets, std::filesystem::path resdir);
	void addLoader(AssetType tag, aloader_func func);
	void add(AssetType tag, const std::filesystem::path filename, const std::string alias);

	bool hasNext() const;
	bool loadNext();

	static void createDefaults(AssetsLoader& loader);
    static void addDefaults(AssetsLoader& loader);

	std::filesystem::path getDirectory() const;
};

#endif // ASSETS_ASSETS_LOADER_H_
