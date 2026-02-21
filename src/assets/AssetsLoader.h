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
class ResPaths;

typedef std::function<bool(Assets*, const ResPaths*, const std::string&, const std::string&)> aloader_func;

struct aloader_entry {
	AssetType tag;
	const std::string filename;
	const std::string alias;
};

class AssetsLoader {
	Assets* assets;
	std::map<AssetType, aloader_func> loaders;
	std::queue<aloader_entry> entries;

	const ResPaths* paths;
public:
	AssetsLoader(Assets* assets, const ResPaths* paths);
	void addLoader(AssetType tag, aloader_func func);
	void add(AssetType tag, const std::string filename, const std::string alias);

	bool hasNext() const;
	bool loadNext();

	static void createDefaults(AssetsLoader& loader);
    static void addDefaults(AssetsLoader& loader);

	const ResPaths* getPaths() const;
};

#endif // ASSETS_ASSETS_LOADER_H_
