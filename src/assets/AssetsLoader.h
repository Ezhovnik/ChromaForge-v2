#ifndef ASSETS_ASSETS_LOADER_H
#define ASSETS_ASSETS_LOADER_H

#include <string>
#include <functional>
#include <map>
#include <queue>

enum class AssetType {
    Texture, Shader, Font, Atlas
};

class Assets;

typedef std::function<bool(Assets*, const std::string&, const std::string&)> aloader_func;

struct aloader_entry {
	AssetType tag;
	const std::string filename;
	const std::string alias;
};

class AssetsLoader {
	Assets* assets;
	std::map<AssetType, aloader_func> loaders;
	std::queue<aloader_entry> entries;
public:
	AssetsLoader(Assets* assets);
	void addLoader(AssetType tag, aloader_func func);
	void add(AssetType tag, const std::string filename, const std::string alias);

	bool hasNext() const;
	bool loadNext();

	static void createDefaults(AssetsLoader& loader);
    static void addDefaults(AssetsLoader& loader);
};

#endif // ASSETS_ASSETS_LOADER_H
