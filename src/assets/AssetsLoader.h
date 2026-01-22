#ifndef SRC_ASSETS_LOADER_H
#define SRC_ASSETS_LOADER_H

#include <string>
#include <functional>
#include <map>
#include <queue>

enum class AssetsType {
    Texture,
    Shader,
    Font,
    Atlas
};

class Assets;

typedef std::function<bool(Assets*, const std::string&, const std::string&)> aloader_func;

struct aloader_entry {
	AssetsType tag;
	const std::string filename;
	const std::string alias;
};

class AssetsLoader {
	Assets* assets;
	std::map<AssetsType, aloader_func> loaders;
	std::queue<aloader_entry> entries;
public:
	AssetsLoader(Assets* assets);
	void addLoader(AssetsType tag, aloader_func func);
	void add(AssetsType tag, const std::string filename, const std::string alias);

	bool hasNext() const;
	bool loadNext();

	static void createDefaults(AssetsLoader& loader);
    static void addDefaults(AssetsLoader& loader);
};

#endif // SRC_ASSETS_LOADER_H
