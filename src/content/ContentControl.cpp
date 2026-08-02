#include <content/ContentControl.h>

#include <io/io.h>
#include <engine/EnginePaths.h>
#include <content/Content.h>
#include <content/ContentPack.h>
#include <content/ContentBuilder.h>
#include <content/ContentLoader.h>
#include <content/PacksManager.h>
#include <objects/rigging.h>
#include <logic/scripting/scripting.h>
#include <core_content_defs.h>
#include <devtools/Project.h>
#include <debug/Logger.h>

static void load_configs(Input* input, const io::path& root) {
    auto configFolder = root / "config";
}

static std::vector<io::path> default_content_sources {
    "world:content",
    "user:content",
    "project:content",
    "res:content",
};

ContentControl::ContentControl(
    const Project& project,
    EnginePaths& paths,
    Input* input,
    std::function<void()> postContent
) : paths(paths),
    input(input),
    postContent(std::move(postContent)),
    basePacks(project.basePacks),
    manager(std::make_unique<PacksManager>())
{
    manager->setSources(default_content_sources);
}

ContentControl::~ContentControl() = default;

Content* ContentControl::get() {
    return content.get();
}

const Content* ContentControl::get() const {
    return content.get();
}

std::vector<std::string>& ContentControl::getBasePacks() {
    return basePacks;
}

void ContentControl::resetContent(
    const std::vector<std::string>& nonReset
) {
    scripting::cleanup(nonReset);
    std::vector<PathsRoot> resRoots;
    {
        auto pack = ContentPack::createBuiltin();
        resRoots.push_back({BUILTIN_CONTENT_NAMESPACE, pack.folder});
        load_configs(input, pack.folder);
    }
    manager->scan();
    for (const auto& pack : manager->getAll(basePacks)) {
        resRoots.push_back({pack.id, pack.folder});
    }
    paths.resPaths = ResPaths(resRoots);
    content.reset();
    scripting::on_content_reset();

    setContentPacksRaw(manager->getAll(basePacks));
    resetContentSources();

    postContent();
}

void ContentControl::loadContent(const std::vector<std::string>& names) {
    manager->scan();
    contentPacks = manager->getAll(manager->assemble(names));
    loadContent();
}

void ContentControl::loadContent() {
    std::vector<std::string> names;
    for (auto& pack : contentPacks) {
        names.push_back(pack.id);
    }

    manager->scan();
    names = manager->assemble(names);
    contentPacks = manager->getAll(names);

    std::vector<PathsRoot> entryPoints;
    for (auto& pack : contentPacks) {
        entryPoints.emplace_back(pack.id, pack.folder);
    }
    paths.setEntryPoints(std::move(entryPoints));

    ContentBuilder contentBuilder;
    CoreContent::setup(input, contentBuilder);

    allPacks = contentPacks;
    allPacks.insert(allPacks.begin(), ContentPack::createBuiltin());

    std::vector<PathsRoot> resRoots;
    for (auto& pack : allPacks) {
        resRoots.push_back({pack.id, pack.folder});
    }
    paths.resPaths = ResPaths(resRoots);

    for (auto& pack : allPacks) {
        ContentLoader(&pack, contentBuilder, paths.resPaths).load();
        load_configs(input, pack.folder);
    }
    content = contentBuilder.build();
    scripting::on_content_load(content.get());

    ContentLoader::loadScripts(*content);

    postContent();
}

void ContentControl::setContentPacksRaw(std::vector<ContentPack>&& packs) {
    if (content) {
        THROW_ERR("'setContentPacksRaw' called with content loaded");
    }
    contentPacks = std::move(packs);
    allPacks = contentPacks;
    allPacks.insert(allPacks.begin(), ContentPack::createBuiltin());
}

const std::vector<ContentPack>& ContentControl::getContentPacks() const {
    return contentPacks;
}

const std::vector<ContentPack>& ContentControl::getAllContentPacks() const {
    return allPacks;
}

PacksManager& ContentControl::scan() {
    manager->scan();
    return *manager;
}

void ContentControl::setContentSources(std::vector<io::path> sources) {
    manager->setSources(std::move(sources));
}

void ContentControl::resetContentSources() {
    manager->setSources(default_content_sources);
}

const std::vector<io::path>& ContentControl::getContentSources() const {
    return manager->getSources();
}
