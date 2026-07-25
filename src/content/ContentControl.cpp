#include <content/ContentControl.h>

#include <io/io.h>
#include <io/engine_paths.h>
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

static void load_configs(Input& input, const io::path& root) {
    auto configFolder = root / "config";
}

ContentControl::ContentControl(
    const Project& project,
    EnginePaths& paths,
    Input& input,
    std::function<void()> postContent
) : paths(paths),
    input(input),
    postContent(std::move(postContent)),
    basePacks(project.basePacks),
    manager(std::make_unique<PacksManager>())
{
    manager->setSources({
        "world:content",
        "user:content",
        "res:content",
    });
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

void ContentControl::resetContent() {
    paths.setCurrentWorldFolder("");

    scripting::cleanup();
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

    postContent();
}

void ContentControl::loadContent(const std::vector<std::string>& names) {
    manager->scan();
    contentPacks = manager->getAll(manager->assemble(names));
    loadContent();
}

void ContentControl::loadContent() {
    scripting::cleanup();

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
