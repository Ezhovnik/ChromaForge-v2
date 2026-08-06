#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <optional>

#include <graphics/commons/Model.h>
#include <objects/rigging.h>

namespace cfmodel {
    struct CFModel {
        std::unordered_map<std::string, model::Model> parts;
        std::optional<rigging::SkeletonConfig> skeleton;

        model::Model& squash();
        model::Model squashed() const;
    };

    CFModel parse(
        std::string_view file,
        std::string_view src,
        bool usexml
    );
}
