#include <graphics/render/NamedSkeletons.h>

#include <objects/rigging.h>

NamedSkeletons::NamedSkeletons() = default;

std::shared_ptr<rigging::Skeleton> NamedSkeletons::createSkeleton(
    const std::string& name, const rigging::SkeletonConfig* config
) {
    auto skeleton = std::make_shared<rigging::Skeleton>(config);
    skeletons[name] = skeleton;
    return skeleton;
}

rigging::Skeleton* NamedSkeletons::getSkeleton(const std::string& name) {
    const auto& found = skeletons.find(name);
    if (found == skeletons.end()) return nullptr;
    return found->second.get();
}
