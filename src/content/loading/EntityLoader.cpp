#define CHROMA_ENABLE_REFLECTION
#include <content/loading/ContentUnitLoader.h>

#include <content/ContentBuilder.h>
#include <coders/json.h>
#include <core_content_defs.h>
#include <data/dv.h>
#include <debug/Logger.h>
#include <io/io.h>
#include <util/stringutil.h>
#include <objects/Entity.h>

template<> void ContentUnitLoader<Entity>::loadUnit(
    Entity& def, const std::string& name, const io::path& file
) {
    auto root = io::read_json(file);

    if (root.has("parent")) {
        const auto& parentName = root["parent"].asString();
        auto parentDef = builder.get(parentName);
        if (parentDef == nullptr) {
            THROW_ERR("Failed to find parent ({}) for {}", parentName, name);
        }
        parentDef->cloneTo(def);
    }

    if (auto found = root.at("components")) {
        for (const auto& elem : *found) {
            std::string name;
            dv::value params;
            if (elem.isObject()) {
                name = elem["name"].asString();
                if (elem.has("args")) {
                    params = elem["args"];
                }
            } else {
                name = elem.asString();
            }
            def.components.push_back(ComponentInstance {std::move(name), std::move(params)});
        }
    }
    if (auto found = root.at("hitbox")) {
        const auto& arr = *found;
        def.hitbox = glm::vec3(
            arr[0].asNumber(), arr[1].asNumber(), arr[2].asNumber()
        );
    }
    if (auto found = root.at("sensors")) {
        const auto& arr = *found;
        for (size_t i = 0; i < arr.size(); ++i) {
            const auto& sensorarr = arr[i];
            const auto& sensorType = sensorarr[0].asString();
            if (sensorType == "aabb") {
                def.boxSensors.emplace_back(
                    i,
                    AABB {
                        {sensorarr[1].asNumber(),
                            sensorarr[2].asNumber(),
                            sensorarr[3].asNumber()},
                        {sensorarr[4].asNumber(),
                            sensorarr[5].asNumber(),
                            sensorarr[6].asNumber()}
                    }
                );
            } else if (sensorType == "radius") {
                def.radialSensors.emplace_back(i, sensorarr[1].asNumber());
            } else {
                LOG_ERROR("Entity {}: sensor №{} — unknown type {}", name, i, util::quote(sensorType));
            }
        }
    }
    root.at("save").get(def.save.enabled);
    root.at("save-skeleton-pose").get(def.save.skeleton.pose);
    root.at("save-skeleton-textures").get(def.save.skeleton.textures);
    root.at("save-body-velocity").get(def.save.body.velocity);
    root.at("save-body-settings").get(def.save.body.settings);

    std::string bodyTypeName;
    root.at("body-type").get(bodyTypeName);
    BodyTypeMeta.getItem(bodyTypeName, def.bodyType);

    root.at("skeleton-name").get(def.skeletonName);
    root.at("blocking").get(def.blocking);
}
