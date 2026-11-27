// src/skills/dep_graph.cpp — YOU IMPLEMENT. Spec §6.2.

#include "skills/dep_graph.h"
#include <algorithm>
#include <unordered_set>

bool DepGraph::add_node(const std::string& name) {
    // TODO
    if (requires_.find(name) != requires_.end()) {
        return false;
    }
    requires_.emplace(name, std::vector<std::string>());
    return true;
}

bool DepGraph::add_edge(const std::string& from, const std::string& to) {
    // TODO
    auto it = requires_.find(from);
    if (it == requires_.end()) {
        return false;
    }
    for (const std::string& r : it->second) {
        if (r == to) {
            return false;
        }
    }
    it->second.push_back(to);
    required_by_[to].push_back(from);
    return true;
}

bool DepGraph::has_node(const std::string& name) const {
    // TODO
    return requires_.find(name) != requires_.end();
}

std::size_t DepGraph::node_count() const noexcept {
    // TODO
    return requires_.size();
}

const std::vector<std::string>& DepGraph::requirements(const std::string& name) const {
    // TODO: return the stored list; for an unknown name return a reference
    // to an empty vector that outlives the call (e.g. a function-local
    // static const).
    static const std::vector<std::string> kNone;
    auto it = requires_.find(name);
    if (it == requires_.end()) {
        return kNone;
    }
    return it->second;
}

bool DepGraph::order_from(const std::string& root, std::vector<std::string>* out,
                          std::string* error) const {
    // TODO: DFS with three colors (white / gray = on the current path /
    // black = finished). Spec §6.2 gives the exact error strings, and
    // Appendix D traces the diamond.
    // Clues:
    //  * Keep a vector<string> path of the gray nodes: push_back when a node
    //    turns gray, pop_back when it turns black. When an edge reaches a
    //    gray node x, the cycle is the part of path from x to the end, then
    //    x again. Check yourself with a>b, a>c, c>a: the answer is
    //    "cycle: a -> c -> a" (b is already finished and must not appear).
    //  * Append a node to *out when it turns BLACK (after its requirements),
    //    never when it turns gray.
    //  * Order of checks for each edge: missing node, then gray (cycle),
    //    then white (visit). Black: skip, that is not a cycle.
    //  * Separators are exactly " -> " (space, dash, greater-than, space).
    if (!has_node(root)) {
        *error = "unknown: " + root;
        return false;
    }
 
    std::unordered_map<std::string, int> color;
    std::vector<std::string> path;
    out->clear();
    return dfs(root, color, path, out, error);
}

bool DepGraph::dfs(const std::string& node, std::unordered_map<std::string, int>& color,
                   std::vector<std::string>& path, std::vector<std::string>* out,
                   std::string* error) const {
    color[node] = 1;
    path.push_back(node);
 
    auto it = requires_.find(node);
    for (const std::string& next : it->second) {
        if (requires_.find(next) == requires_.end()) {
            *error = "missing: " + next + " (required by " + node + ")";
            return false;
        }
        if (color[next] == 1) {
            std::size_t start = 0;
            while (path[start] != next) {
                start++;
            }
            std::string cycle = "cycle: ";
            for (std::size_t i = start; i < path.size(); i++) {
                cycle += path[i] + " -> ";
            }
            cycle += next;
            *error = cycle;
            return false;
        }
        if (color[next] == 0) {
            if (!dfs(next, color, path, out, error)) {
                return false;
            }
        }
    }
    path.pop_back();
    color[node] = 2;
    out->push_back(node);
    return true;
}

std::vector<std::string> DepGraph::dependents(const std::string& name) const {
    // TODO: BFS over the reversed edges.
    // Clues:
    //  * Mark `name` itself as seen before you start, so a cycle can't add
    //    it to the result.
    //  * The order is by (distance, name). Easiest: sort each BFS level
    //    alphabetically before appending it. Sorting the whole result by
    //    name alone loses the nearest-first order.
    //  * std::queue<std::pair<std::string, int>> (node, distance) works, or
    //    process one level (a vector) at a time.
    std::vector<std::string> result;
    std::unordered_set<std::string> seen;
    seen.insert(name);
    std::vector<std::string> level;
    level.push_back(name);
    while (!level.empty()) {
        std::vector<std::string> next_level;
        for (const std::string& cur : level) {
            auto it = required_by_.find(cur);
            if (it == required_by_.end()) {
                continue;
            }
            for (const std::string& dep : it->second) {
                if (seen.count(dep) == 0) {
                    seen.insert(dep);
                    next_level.push_back(dep);
                }
            }
        }
        std::sort(next_level.begin(), next_level.end());
        for (const std::string& s : next_level) {
            result.push_back(s);
        }
        level = next_level;
    }
    return result;
}
