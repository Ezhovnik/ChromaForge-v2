#include <voxels/Pathfinding.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <queue>
#include <unordered_set>
#include <unordered_map>

#include <world/Level.h>
#include <voxels/GlobalChunks.h>
#include <voxels/Chunk.h>
#include <voxels/blocks_agent.h>
#include <content/Content.h>

using namespace voxels;

struct Node {
    glm::ivec3 pos;
    glm::ivec3 parent;
    float gScore;
    float fScore;
};

struct NodeLess {
    bool operator()(const Node& l, const Node& r) const {
        return l.fScore > r.fScore;
    }
};

static float distance(const glm::ivec3& a, const glm::ivec3& b) {
    return glm::distance(glm::vec3(a), glm::vec3(b));
}

Pathfinding::Pathfinding(const Level& level) : level(level) {}

Route Pathfinding::perform(const glm::ivec3& start, const glm::ivec3& end) {
    Route route {};

    std::priority_queue<Node, std::vector<Node>, NodeLess> queue;
    queue.push({start, {}, 0, distance(start, end)});

    std::unordered_set<glm::ivec3> blocked;
    std::unordered_map<glm::ivec3, Node> parents;

    const auto& chunks = *level.chunks;

    while (!queue.empty()) {
        auto node = queue.top();
        queue.pop();

        if (blocked.find(node.pos) != blocked.end()) continue;

        if (node.pos.x == end.x && node.pos.z == end.z) {
            auto prev = glm::ivec3();
            auto pos = node.pos;
            while (pos != start) {
                const auto& found = parents.find(pos);
                if (found == parents.end()) {
                    route.nodes.push_back({pos});
                    break;
                }
                route.nodes.push_back({pos});

                prev = pos;
                pos = found->second.pos;
            }
            route.nodes.push_back({start});
            route.found = true;
            break;
        }

        blocked.emplace(node.pos);
        glm::ivec2 neighbors[8] {
            {0, 1}, {1, 0}, {0, -1}, {-1, 0},
            {-1, -1}, {1, -1}, {1, 1}, {-1, 1},
        };

        for (int i = 0; i < sizeof(neighbors) / sizeof(glm::ivec2); i++) {
            auto offset = neighbors[i];
            auto point = node.pos + glm::ivec3(offset.x, 0, offset.y);
            if (blocks_agent::is_obstacle_at(
                    chunks, point.x + 0.5f, point.y + 0.5f, point.z + 0.5f
                )) {
                continue;
            }
            if (i >= 4) {
                auto a = node.pos + glm::ivec3(offset.x, 0, offset.y);
                auto b = node.pos + glm::ivec3(offset.x, 0, offset.y);
                if (blocks_agent::is_obstacle_at(chunks, a.x, a.y, a.z)) continue;
                if (blocks_agent::is_obstacle_at(chunks, b.x, b.y, b.z)) continue;
            }
            if (blocked.find(point) != blocked.end()) continue;
            float gScore =
                node.gScore + glm::abs(offset.x) + glm::abs(offset.y);
            const auto& foundParent = parents.find(point);
            bool queued = foundParent != parents.end();
            if (!queued || gScore < foundParent->second.gScore) {
                float hScore = distance(point, end);
                float fScore = gScore + hScore;
                Node nNode {point, node.pos, gScore, fScore};
                parents[point] = Node {node.pos, node.parent, gScore, fScore};
                queue.push(nNode);
            }
        }
    }
    return route;
}
