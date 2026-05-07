#include <content/ContentReport.h>

#include <memory>

#include <content/Content.h>
#include <constants.h>
#include <files/files.h>
#include <coders/json.h>
#include <voxels/Block.h>
#include <items/Item.h>
#include <world/World.h>
#include <files/WorldFiles.h>

ContentReport::ContentReport(
    const ContentIndices* indices,
    size_t blocksCount,
    size_t itemsCount,
    uint regionsVersion
) : blocks(blocksCount, indices->blocks, BLOCK_VOID, ContentType::Block),
    items(itemsCount, indices->items, ITEM_VOID, ContentType::Item),
    regionsVersion(regionsVersion) {}

template<class T> static constexpr size_t get_entries_count(
    const ContentUnitIndices<T>& indices, const dv::value& list
) {
    return list != nullptr 
        ? std::max(list.size(), indices.count()) 
        : indices.count();
}

static void process_blocks_data(
    const Content* content,
    ContentReport& report,
    const dv::value& root
) {
    for (const auto& [name, map] : root.asObject()) {
        data::StructLayout layout;
        layout.deserialize(map);
        auto def = content->blocks.find(name);
        if (def == nullptr) continue;

        if (def->dataStruct == nullptr) {
            ContentIssue issue {ContentIssueType::BlockDataLayoutsUpdate};
            report.issues.push_back(issue);
            report.dataLoss.push_back(name + ": discard data");
            continue;
        }
        if (layout != *def->dataStruct) {
            ContentIssue issue {ContentIssueType::BlockDataLayoutsUpdate};
            report.issues.push_back(issue);
            report.dataLayoutsUpdated = true;
        }

        auto incapatibility = layout.checkCompatibility(*def->dataStruct);
        if (!incapatibility.empty()) {
            for (const auto& error : incapatibility) {
                report.dataLoss.push_back(
                    "[" + name + "] field " + error.name + " - " +
                    data::to_string(error.type)
                );
            }
        }
        report.blocksDataLayouts[name] = std::move(layout);
    }
}

std::shared_ptr<ContentReport> ContentReport::create(
    const std::shared_ptr<WorldFiles>& worldFiles,
    const std::filesystem::path& filename, 
    const Content* content
) {
    auto worldInfo = worldFiles->readWorldInfo();
    if (!worldInfo.has_value()) {
        return nullptr;
    }

    auto root = files::read_json(filename);
    uint regionsVersion = static_cast<uint>(root["region-version"].asInteger(REGION_FORMAT_VERSION));
    auto& blocklist = root["blocks"];
    auto& itemlist = root["items"];

    auto* indices = content->getIndices();
    size_t blocks_c = get_entries_count(indices->blocks, blocklist);
    size_t items_c = get_entries_count(indices->items, itemlist);

    auto report = std::make_shared<ContentReport>(
        indices, blocks_c, items_c, regionsVersion
    );

    report->blocks.setup(blocklist, content->blocks);
    report->items.setup(itemlist, content->items);

    if (root.has("blocks-data")) {
        process_blocks_data(content, *report, root["blocks-data"]);
    }

    report->buildIssues();

    if (report->isUpgradeRequired() ||
        report->hasContentReorder() ||
        report->hasMissingContent() ||
        report->hasUpdatedLayouts()
    ) {
        return report;
    } else {
        return nullptr;
    }
}

template<class T, class U>
static void build_issues(
    std::vector<ContentIssue>& issues,
    const ContentUnitLUT<T, U>& report
) {
    auto type = report.getContentType();
    if (report.hasContentReorder()) {
        issues.push_back(ContentIssue {ContentIssueType::Reorder, type});
    }
    if (report.hasMissingContent()) {
        issues.push_back(ContentIssue {ContentIssueType::Missing, type});
    }
}

void ContentReport::buildIssues() {
    build_issues(issues, blocks);
    build_issues(issues, items);

    if (regionsVersion < REGION_FORMAT_VERSION) {
        for (int layer = REGION_LAYER_VOXELS; layer < REGION_LAYERS_COUNT; layer++) {
            ContentIssue issue {ContentIssueType::RegionFormatUpdate};
            issue.regionLayer = static_cast<RegionLayerIndex>(layer);
            issues.push_back(issue);
        }
    }
}

const std::vector<ContentIssue>& ContentReport::getIssues() const {
    return issues;
}

std::vector<ContentEntry> ContentReport::getMissingContent() const {
    std::vector<ContentEntry> entries;
    blocks.getMissingContent(entries);
    items.getMissingContent(entries);
    return entries;
}
