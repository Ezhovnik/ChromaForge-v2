#ifndef ASSETS_ASSETS_LOADER_H_
#define ASSETS_ASSETS_LOADER_H_

#include <string>
#include <functional>
#include <map>
#include <queue>
#include <filesystem>

enum class AssetType {
    Texture,
	Shader,
	Font,
	Atlas,
	Layout
};

class Assets;
class ResPaths;
class Content;

using aloader_func = std::function<bool(Assets*, const ResPaths*, const std::string&, const std::string&, std::shared_ptr<void>)>;

struct aloader_entry {
	AssetType tag;
	const std::string filename;
	const std::string alias;
	std::shared_ptr<void> config;
};

class AssetsLoader {
	Assets* assets;
	std::map<AssetType, aloader_func> loaders;
	std::queue<aloader_entry> entries;

	const ResPaths* paths;
public:
	AssetsLoader(Assets* assets, const ResPaths* paths);
	void addLoader(AssetType tag, aloader_func func);
	void add(
		AssetType tag, 
		const std::string filename, 
		const std::string alias,
		std::shared_ptr<void> config=nullptr
	);

	bool hasNext() const;
	bool loadNext();

    static void addDefaults(AssetsLoader& loader, const Content* content);

	const ResPaths* getPaths() const;
};

#endif // ASSETS_ASSETS_LOADER_H_
