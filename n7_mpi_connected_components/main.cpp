#include <algorithm>
#include <atomic>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <omp.h>
#include <ostream>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

using NodeIndex = unsigned long long;

struct Node {
    std::vector<NodeIndex> neighbors;
};

struct ConnectedComponents {
    // group[i] = component ID for node i (Union-Find root index)
    std::vector<NodeIndex> group;
};

struct Graph {
    std::vector<Node> nodes;

    Graph(std::vector<Node> nodes) : nodes(std::move(nodes)) {
    }

    ConnectedComponents get_connected_components_serial() const {
        ConnectedComponents result;
        result.group.resize(nodes.size());
        std::vector<bool> visited(nodes.size(), false);

        for (NodeIndex i = 0; i < nodes.size(); i++) {
            if (!visited[i]) {
                std::vector<NodeIndex> stack;
                stack.push_back(i);
                visited[i] = true;

                // Use smallest node index as component ID
                NodeIndex component_id = i;

                while (!stack.empty()) {
                    NodeIndex current = stack.back();
                    stack.pop_back();
                    result.group[current] = component_id;

                    for (NodeIndex neighbor : nodes[current].neighbors) {
                        if (!visited[neighbor]) {
                            visited[neighbor] = true;
                            stack.push_back(neighbor);
                        }
                    }
                }
            }
        }

        return result;
    }

    ConnectedComponents get_connected_components_parallel_1(unsigned threads_count) const {
        std::vector<omp_lock_t> group_locks(nodes.size());
        for (NodeIndex i = 0; i < nodes.size(); i++) {
            omp_init_lock(&group_locks[i]);
        }

        struct GroupNode {
            GroupNode *parent;
            NodeIndex index;
        };

        std::vector<GroupNode *> groups(nodes.size());
        for (NodeIndex i = 0; i < nodes.size(); i++) {
            groups[i] = new GroupNode;
            groups[i]->parent = nullptr; // Initialize parent to nullptr (self is root)
            groups[i]->index = i;
        }

        // Helper lambda to find root (no path compression in parallel section to avoid races)
        auto find_root = [](GroupNode *node) -> GroupNode * {
            while (node->parent != nullptr) {
                node = node->parent;
            }
            return node;
        };

        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex node_index = 0; node_index < nodes.size(); node_index++) {
            for (NodeIndex adj_index : nodes[node_index].neighbors) {
                // Lock in consistent order to avoid deadlock
                NodeIndex lock_first = std::min(node_index, adj_index);
                NodeIndex lock_second = std::max(node_index, adj_index);

                omp_set_lock(&group_locks[lock_first]);
                omp_set_lock(&group_locks[lock_second]);

                // Find roots of both nodes
                GroupNode *root1 = find_root(groups[node_index]);
                GroupNode *root2 = find_root(groups[adj_index]);

                // Union by index (use smaller index as root for determinism)
                if (root1 != root2) {
                    if (root1->index < root2->index) {
                        root2->parent = root1;
                    } else {
                        root1->parent = root2;
                    }
                }

                omp_unset_lock(&group_locks[lock_second]);
                omp_unset_lock(&group_locks[lock_first]);
            }
        }

        ConnectedComponents result;
        result.group.resize(nodes.size());

        // Find root for each node - this IS the group ID
        for (NodeIndex i = 0; i < nodes.size(); i++) {
            GroupNode *current = groups[i];
            while (current->parent != nullptr) {
                current = current->parent;
            }
            result.group[i] = current->index;
        }

        return result;
    }

    ConnectedComponents get_connected_components_parallel_2(unsigned threads_count) const {
        const NodeIndex n = nodes.size();
        const NodeIndex UNCLAIMED = ULLONG_MAX;

        // group[i] = component label for node i (UNCLAIMED if not yet visited)
        std::vector<std::atomic<NodeIndex>> group(n);
        // group_neighbors[i] = set of labels that label i is connected to
        std::vector<std::set<NodeIndex>> group_neighbors(n);

        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex i = 0; i < n; i++) {
            group[i].store(UNCLAIMED, std::memory_order_relaxed);
        }

        // Phase 1: Parallel exploration, record label connections

        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex start = 0; start < n; start++) {
            NodeIndex current_label = start;
            std::vector<NodeIndex> stack;
            stack.push_back(start);

            while (!stack.empty()) {
                NodeIndex node = stack.back();
                stack.pop_back();

                // Try to claim this node
                NodeIndex expected = UNCLAIMED;
                if (group[node].compare_exchange_strong(expected, current_label,
                                                        std::memory_order_relaxed)) {
                    // Claimed! Add neighbors to stack
                    for (NodeIndex neighbor : nodes[node].neighbors) {
                        stack.push_back(neighbor);
                    }
                } else if (expected != current_label) {
                    // Found connection to another component - record in our set (no contention)
                    group_neighbors[current_label].insert(expected);
                }
            }
        }

        // Phase 2: Union-Find on labels using group_neighbors
        std::vector<NodeIndex> group_parent(n);
        for (NodeIndex i = 0; i < n; i++) {
            group_parent[i] = i;
        }

        auto find_root = [&](NodeIndex x) -> NodeIndex {
            while (group_parent[x] != x) {
                group_parent[x] = group_parent[group_parent[x]];
                x = group_parent[x];
            }
            return x;
        };

        auto unite = [&](NodeIndex a, NodeIndex b) {
            NodeIndex ra = find_root(a);
            NodeIndex rb = find_root(b);
            if (ra != rb) {
                if (ra < rb) {
                    group_parent[rb] = ra;
                } else {
                    group_parent[ra] = rb;
                }
            }
        };

        for (NodeIndex i = 0; i < n; i++) {
            for (NodeIndex neighbor : group_neighbors[i]) {
                unite(i, neighbor);
            }
        }

        // Flatten all parents
        for (NodeIndex i = 0; i < n; i++) {
            group_parent[i] = find_root(i);
        }

        // Phase 3: Build result (parallel)
        ConnectedComponents result;
        result.group.resize(n);

        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex i = 0; i < n; i++) {
            result.group[i] = group_parent[group[i].load(std::memory_order_relaxed)];
        }

        return result;
    }

    ConnectedComponents get_connected_components_parallel_3(unsigned threads_count,
                                                            bool print_timing = false) const {
        const NodeIndex n = nodes.size();
        const NodeIndex UNCLAIMED = ULLONG_MAX;

        // group[i] = component label for node i (UNCLAIMED if not yet visited)
        std::vector<std::atomic<NodeIndex>> group(n);
        // group_neighbors[i] = set of labels that label i is connected to
        std::vector<std::vector<NodeIndex>> group_neighbors(n);

        double phase_start, phase_end;
        double phase0_time, phase1_time, phase2_time, phase3_time;

        // Initialization phase
        phase_start = omp_get_wtime();
        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex i = 0; i < n; i++) {
            group[i].store(UNCLAIMED, std::memory_order_relaxed);
        }
        phase_end = omp_get_wtime();
        phase0_time = phase_end - phase_start;

        // Phase 1: Parallel exploration, record label connections
        phase_start = omp_get_wtime();
        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex start = 0; start < n; start++) {
            NodeIndex current_label = start;
            std::vector<NodeIndex> stack;
            stack.push_back(start);

            while (!stack.empty()) {
                NodeIndex node = stack.back();
                stack.pop_back();

                // Try to claim this node
                NodeIndex expected = UNCLAIMED;
                if (group[node].compare_exchange_strong(expected, current_label,
                                                        std::memory_order_relaxed)) {
                    // Claimed! Add neighbors to stack
                    for (NodeIndex neighbor : nodes[node].neighbors) {
                        stack.push_back(neighbor);
                    }
                } else if (expected != current_label) {
                    // Found connection to another component - record in our set (no contention)
                    group_neighbors[current_label].push_back(expected);
                }
            }
        }
        phase_end = omp_get_wtime();
        phase1_time = phase_end - phase_start;

        // Phase 2: Union-Find on labels using group_neighbors
        phase_start = omp_get_wtime();

        double subphase_start, subphase_end;
        double phase2_init_time, phase2_union_time, phase2_flatten_time;

        // Subphase 2a: Initialize group_parent
        subphase_start = omp_get_wtime();
        std::vector<NodeIndex> group_parent(n);
        for (NodeIndex i = 0; i < n; i++) {
            group_parent[i] = i;
        }
        subphase_end = omp_get_wtime();
        phase2_init_time = subphase_end - subphase_start;

        auto find_root = [&](NodeIndex x) -> NodeIndex {
            while (group_parent[x] != x) {
                group_parent[x] = group_parent[group_parent[x]];
                x = group_parent[x];
            }
            return x;
        };

        auto unite = [&](NodeIndex a, NodeIndex b) {
            NodeIndex ra = find_root(a);
            NodeIndex rb = find_root(b);
            if (ra != rb) {
                if (ra < rb) {
                    group_parent[rb] = ra;
                } else {
                    group_parent[ra] = rb;
                }
            }
        };

        // Subphase 2b: Union operations
        subphase_start = omp_get_wtime();
        for (NodeIndex i = 0; i < n; i++) {
            for (NodeIndex neighbor : group_neighbors[i]) {
                unite(i, neighbor);
            }
        }
        subphase_end = omp_get_wtime();
        phase2_union_time = subphase_end - subphase_start;

        // Subphase 2c: Flatten all parents
        subphase_start = omp_get_wtime();
        for (NodeIndex i = 0; i < n; i++) {
            group_parent[i] = find_root(i);
        }
        subphase_end = omp_get_wtime();
        phase2_flatten_time = subphase_end - subphase_start;

        phase_end = omp_get_wtime();
        phase2_time = phase_end - phase_start;

        // Phase 3: Build result (parallel)
        phase_start = omp_get_wtime();
        ConnectedComponents result;
        result.group.resize(n);

        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex i = 0; i < n; i++) {
            result.group[i] = group_parent[group[i].load(std::memory_order_relaxed)];
        }
        phase_end = omp_get_wtime();
        phase3_time = phase_end - phase_start;

        if (print_timing) {
            std::cout << "  [Phase 0 - Initialization]: " << phase0_time << " seconds\n";
            std::cout << "  [Phase 1 - Parallel Exploration]: " << phase1_time << " seconds\n";
            std::cout << "  [Phase 2 - Union-Find]: " << phase2_time << " seconds\n";
            std::cout << "    [Phase 2a - Init]: " << phase2_init_time << " seconds\n";
            std::cout << "    [Phase 2b - Union]: " << phase2_union_time << " seconds\n";
            std::cout << "    [Phase 2c - Flatten]: " << phase2_flatten_time << " seconds\n";
            std::cout << "  [Phase 3 - Build Result]: " << phase3_time << " seconds\n";
        }

        return result;
    }

    ConnectedComponents get_connected_components_Shiloach_Vishkin(unsigned threads_count) const {
        const NodeIndex n = nodes.size();

        std::vector<std::atomic<NodeIndex>> parent(n);

        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex i = 0; i < n; i++) {
            parent[i].store(i, std::memory_order_relaxed);
        }

        bool changed = true;
        while (changed) {
            changed = false;

            // Use local flag + reduction to fix Data Race and False Sharing
            bool hook_changed = false;

            // Hooking phase

            // clang-format off
            #pragma omp parallel for num_threads(threads_count) reduction(|| : hook_changed)
            // clang-format on
            for (NodeIndex u = 0; u < n; u++) {
                NodeIndex parent_u = parent[u].load(std::memory_order_relaxed);

                for (NodeIndex v : nodes[u].neighbors) {
                    NodeIndex parent_v = parent[v].load(std::memory_order_relaxed);

                    if (parent_u != parent_v) {
                        // We attempt to update the GRANDPARENT.
                        // This effectively merges the tree rooted at parent[u] into parent[v].

                        // Case 1: parent_u > parent_v. Try to pull parent_u down to parent_v.
                        if (parent_u > parent_v) {
                            NodeIndex p_p_u = parent[parent_u].load(std::memory_order_relaxed);
                            // Only update if it improves the value (min-index rule)
                            while (p_p_u > parent_v) {
                                if (parent[parent_u].compare_exchange_weak(
                                        p_p_u, parent_v, std::memory_order_relaxed)) {
                                    hook_changed = true;
                                    break;
                                }
                                // If CAS fails, p_p_u is automatically updated to the new value in
                                // memory. The loop condition (p_p_u > parent_v) re-checks if we
                                // still need to write.
                            }
                        }
                        // Case 2: parent_v > parent_u. Try to pull parent_v down to parent_u.
                        else {
                            NodeIndex p_p_v = parent[parent_v].load(std::memory_order_relaxed);
                            while (p_p_v > parent_u) {
                                if (parent[parent_v].compare_exchange_weak(
                                        p_p_v, parent_u, std::memory_order_relaxed)) {
                                    hook_changed = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            // Shortcutting phase
            bool shortcut_changed = false;

            // clang-format off
            #pragma omp parallel for num_threads(threads_count) reduction(|| : shortcut_changed)
            // clang-format on
            for (NodeIndex u = 0; u < n; u++) {
                NodeIndex parent_u = parent[u].load(std::memory_order_relaxed);
                NodeIndex grandparent_u = parent[parent_u].load(std::memory_order_relaxed);

                if (parent_u != grandparent_u) {
                    // Path compression: Point u directly to grandparent
                    parent[u].store(grandparent_u, std::memory_order_relaxed);
                    shortcut_changed = true;
                }
            }

            changed = hook_changed || shortcut_changed;
        }

        // Build result
        ConnectedComponents result;
        result.group.resize(n);

        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex i = 0; i < n; i++) {
            NodeIndex root = i;
            NodeIndex parent_val = parent[root].load(std::memory_order_relaxed);
            while (parent_val != root) {
                root = parent_val;
                parent_val = parent[root].load(std::memory_order_relaxed);
            }
            result.group[i] = root;
        }

        return result;
    }

    ConnectedComponents get_connected_components_SV_impr(unsigned threads_count) const {
        const NodeIndex n = nodes.size();

        // Afforest algorithm: SV with subgraph sampling
        std::vector<std::atomic<NodeIndex>> parent(n);

        // Initialize parent array

        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex i = 0; i < n; i++) {
            parent[i].store(i, std::memory_order_relaxed);
        }

        // Helper: link procedure
        auto link = [&](NodeIndex u, NodeIndex v) {
            NodeIndex p1 = parent[u].load(std::memory_order_relaxed);
            NodeIndex p2 = parent[v].load(std::memory_order_relaxed);

            while (p1 != p2) {
                NodeIndex h = std::max(p1, p2);
                NodeIndex l = std::min(p1, p2);

                NodeIndex expected = h;
                if (parent[h].compare_exchange_weak(expected, l, std::memory_order_relaxed)) {
                    break;
                }

                p1 = parent[parent[h].load(std::memory_order_relaxed)].load(
                    std::memory_order_relaxed);
                p2 = parent[l].load(std::memory_order_relaxed);
            }
        };

        // Helper: compress procedure
        auto compress = [&](NodeIndex v) {
            NodeIndex parent_v = parent[v].load(std::memory_order_relaxed);
            NodeIndex grandparent_v = parent[parent_v].load(std::memory_order_relaxed);

            while (parent_v != grandparent_v) {
                parent[v].store(grandparent_v, std::memory_order_relaxed);
                parent_v = grandparent_v;
                grandparent_v = parent[parent_v].load(std::memory_order_relaxed);
            }
        };

        // Phase 1: Neighbor sampling rounds (2 rounds)
        const int neighbor_rounds = 2;

        for (int round = 0; round < neighbor_rounds; round++) {
            // Link phase: process first 'round+1' neighbors

            // clang-format off
            #pragma omp parallel for num_threads(threads_count)
            // clang-format on
            for (NodeIndex u = 0; u < n; u++) {
                if (round < nodes[u].neighbors.size()) {
                    NodeIndex v = nodes[u].neighbors[round];
                    link(u, v);
                }
            }

            // Compress phase

            // clang-format off
            #pragma omp parallel for num_threads(threads_count)
            // clang-format on
            for (NodeIndex v = 0; v < n; v++) {
                compress(v);
            }
        }

        // Phase 2: Find largest component (probabilistic sampling)
        NodeIndex largest_component = 0;
        {
            std::unordered_map<NodeIndex, NodeIndex> component_count;
            const int sample_size = std::min(n, NodeIndex(10000));

            for (NodeIndex i = 0; i < sample_size; i++) {
                NodeIndex sample_idx = (i * n) / sample_size;
                NodeIndex comp = parent[sample_idx].load(std::memory_order_relaxed);
                component_count[comp]++;
            }

            NodeIndex max_count = 0;
            for (const auto &[comp, count] : component_count) {
                if (count > max_count) {
                    max_count = count;
                    largest_component = comp;
                }
            }
        }

        // Phase 3: Process remaining edges, skipping largest component

        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex u = 0; u < n; u++) {
            NodeIndex parent_u = parent[u].load(std::memory_order_relaxed);
            if (parent_u != largest_component) {
                for (size_t i = neighbor_rounds; i < nodes[u].neighbors.size(); i++) {
                    NodeIndex v = nodes[u].neighbors[i];
                    link(u, v);
                }
            }
        }

        // Final compress

        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex v = 0; v < n; v++) {
            compress(v);
        }

        // Build result
        ConnectedComponents result;
        result.group.resize(n);

        // clang-format off
        #pragma omp parallel for num_threads(threads_count)
        // clang-format on
        for (NodeIndex i = 0; i < n; i++) {
            NodeIndex root = i;
            NodeIndex parent_val = parent[root].load(std::memory_order_relaxed);
            while (parent_val != root) {
                root = parent_val;
                parent_val = parent[root].load(std::memory_order_relaxed);
            }
            result.group[i] = root;
        }

        return result;
    }
};

Graph *create_graph(NodeIndex count, long long edge_count) {
    std::vector<Node> nodes(count);

    // Use srand with a fixed seed for reproducibility
    const unsigned int RANDOM_SEED = 12345;
    std::srand(RANDOM_SEED);

    // Union-Find structure to track connected components cheaply
    std::vector<NodeIndex> parent(count);
    std::vector<NodeIndex> component_size(count);

    // Initialize each node as its own component
    for (NodeIndex idx = 0; idx < count; idx++) {
        parent[idx] = idx;
        component_size[idx] = 1;
    }

    // Find root with path compression
    std::function<NodeIndex(NodeIndex)> find_root = [&](NodeIndex node_idx) -> NodeIndex {
        if (parent[node_idx] != node_idx) {
            parent[node_idx] = find_root(parent[node_idx]);
        }
        return parent[node_idx];
    };

    // Union two components
    auto unite = [&](NodeIndex node_u, NodeIndex node_v) {
        NodeIndex root_u = find_root(node_u);
        NodeIndex root_v = find_root(node_v);

        if (root_u != root_v) {
            // Merge smaller into larger
            if (component_size[root_u] < component_size[root_v]) {
                parent[root_u] = root_v;
                component_size[root_v] += component_size[root_u];
            } else {
                parent[root_v] = root_u;
                component_size[root_u] += component_size[root_v];
            }
        }
    };

    // Generate edges with preference for isolated nodes
    for (long long idx = 0; idx < edge_count; idx++) {
        NodeIndex node_u, node_v;

        // Select nodes with probability inversely proportional to their component size
        // Use rejection sampling to favor smaller components
        auto select_node_weighted = [&]() -> NodeIndex {
            NodeIndex candidate;
            NodeIndex candidate_root;
            NodeIndex candidate_size;

            // Try up to 10 times to get a good candidate (from small component)
            NodeIndex best_candidate = static_cast<NodeIndex>(std::rand()) % count;
            NodeIndex best_size = component_size[find_root(best_candidate)];

            for (int attempt = 0; attempt < 10; attempt++) {
                candidate = static_cast<NodeIndex>(std::rand()) % count;
                candidate_root = find_root(candidate);
                candidate_size = component_size[candidate_root];

                // Keep candidate if it's in a smaller component
                if (candidate_size < best_size) {
                    best_candidate = candidate;
                    best_size = candidate_size;
                }
            }

            return best_candidate;
        };

        node_u = select_node_weighted();
        node_v = select_node_weighted();

        // Add edge
        nodes[node_u].neighbors.push_back(node_v);
        nodes[node_v].neighbors.push_back(node_u);

        // Update component information
        unite(node_u, node_v);
    }

    return new Graph(nodes);
}

// Normalize group labels: first seen label -> 0, second -> 1, etc.
std::vector<NodeIndex> normalize_groups(const std::vector<NodeIndex> &group) {
    std::vector<NodeIndex> result(group.size());
    std::unordered_map<NodeIndex, NodeIndex> label_map;
    NodeIndex next_label = 0;

    for (size_t i = 0; i < group.size(); i++) {
        auto it = label_map.find(group[i]);
        if (it == label_map.end()) {
            label_map[group[i]] = next_label++;
        }
        result[i] = label_map[group[i]];
    }
    return result;
}

bool compare_connected_components(
    const char *source_name, const ConnectedComponents &source,
    const std::vector<std::pair<const char *, ConnectedComponents>> &others) {
    auto source_normalized = normalize_groups(source.group);

    std::cout << "Number of groups: "
              << std::set<NodeIndex>(source_normalized.begin(), source_normalized.end()).size()
              << "\n";

    bool found_different = false;

    for (const auto &[other_name, other] : others) {
        if (source.group.size() != other.group.size()) {
            std::cout << "ERROR: " << source_name << " vs " << other_name << " - size mismatch ("
                      << source.group.size() << " vs " << other.group.size() << ")\n";
            continue;
        }

        auto other_normalized = normalize_groups(other.group);
        bool match = true;
        for (size_t i = 0; i < source_normalized.size(); i++) {
            if (source_normalized[i] != other_normalized[i]) {
                found_different = true;

                std::cout << "ERROR: " << source_name << " vs " << other_name
                          << " - mismatch at index " << i << " (group " << source_normalized[i]
                          << " vs " << other_normalized[i] << ")\n";
                match = false;
                break;
            }
        }
        if (match) {
            std::cout << "OK: " << source_name << " == " << other_name << "\n";
        }
    }

    return found_different;
}

template <typename Func>
std::pair<double, ConnectedComponents> benchmark(const char *name, int runs, Func func) {
    ConnectedComponents result;
    double total_time = 0;
    std::vector<double> iteration_times;

    for (int i = 0; i < runs; i++) {
        double start = omp_get_wtime();
        result = func();
        double end = omp_get_wtime();
        double iteration_time = end - start;
        total_time += iteration_time;
        iteration_times.push_back(iteration_time);
    }

    double avg_time = total_time / runs;
    std::cout << name << ": " << avg_time << " seconds (avg of " << runs << " runs)\n";
    for (int i = 0; i < runs; i++) {
        // std::cout << "  Run " << (i + 1) << ": " << iteration_times[i] << " seconds\n";
    }
    return {avg_time, result};
}

void test_efficiency_and_scalability(const char *func_name,
                                     std::function<ConnectedComponents(unsigned)> func,
                                     unsigned max_threads, int runs) {
    std::cout << "\n=== Testing Efficiency and Scalability: " << func_name << " ===\n";

    std::vector<double> times(max_threads + 1);
    std::vector<double> speedups(max_threads + 1);
    std::vector<double> efficiencies(max_threads + 1);

    // Baseline: single thread
    auto [t_1, groups_1] = benchmark((std::string(func_name) + " (001 thread)").c_str(), runs,
                                     [&]() { return func(1); });
    times[1] = t_1;
    speedups[1] = 1.0;
    efficiencies[1] = 1.0;

    std::cout << "  Speedup: 1.00x, Efficiency: 100.00%\n";

    // Test with increasing thread counts
    for (unsigned tc = 2; tc <= max_threads; tc++) {
        std::string thread_str = std::to_string(tc);
        if (tc < 10)
            thread_str = "00" + thread_str;
        else if (tc < 100)
            thread_str = "0" + thread_str;

        auto [t_tc, groups_tc] =
            benchmark((std::string(func_name) + " (" + thread_str + " threads)").c_str(), runs,
                      [&]() { return func(tc); });

        times[tc] = t_tc;
        speedups[tc] = t_1 / t_tc;
        efficiencies[tc] = speedups[tc] / tc;

        std::cout << "  Speedup: " << speedups[tc]
                  << "x, Efficiency: " << (efficiencies[tc] * 100.0) << "%\n";
    }

    // Summary statistics
    std::cout << "\nSummary:\n";
    std::cout << "  Best speedup: " << speedups[max_threads] << "x at " << max_threads
              << " threads\n";
    std::cout << "  Best efficiency: " << (efficiencies[1] * 100.0) << "% at 1 thread\n";

    // Find thread count with best efficiency/performance tradeoff (efficiency > 50%)
    unsigned best_tradeoff = 1;
    for (unsigned tc = 1; tc <= max_threads; tc++) {
        if (efficiencies[tc] >= 0.5) {
            best_tradeoff = tc;
        }
    }
    std::cout << "  Best tradeoff (eff >= 50%): " << best_tradeoff << " threads "
              << "(" << speedups[best_tradeoff] << "x speedup, "
              << (efficiencies[best_tradeoff] * 100.0) << "% efficiency)\n";
}

struct TestResult {
    double avg_time;
    bool correct;
    size_t component_count;
};

// Returns true if A and B represent the same connectivity
bool verify_equivalence(const std::vector<NodeIndex> &baseline,
                        const std::vector<NodeIndex> &target) {
    if (baseline.size() != target.size())
        return false;

    // We normalize both. If they are identical graphs, the normalized forms must be identical.
    auto norm_base = normalize_groups(baseline);
    auto norm_targ = normalize_groups(target);

    return norm_base == norm_targ;
}

TestResult run_benchmark(const char *algo_name, std::function<ConnectedComponents()> func, int runs,
                         const std::vector<NodeIndex> *baseline_groups = nullptr) {

    double total_time = 0;
    ConnectedComponents result;

    // Warmup / First run (used for correctness check)
    double start = omp_get_wtime();
    result = func();
    double end = omp_get_wtime();
    total_time += (end - start);

    bool correct = true;
    if (baseline_groups) {
        correct = verify_equivalence(*baseline_groups, result.group);
        if (!correct) {
            std::cerr << "  [ERROR] " << algo_name << " result mismatch!\n";
        }
    }

    // Remaining runs for timing
    for (int i = 1; i < runs; i++) {
        start = omp_get_wtime();
        // We ignore the result of subsequent runs to save memory/time,
        // strictly measuring execution speed.
        auto dummy = func();
        end = omp_get_wtime();
        total_time += (end - start);
    }

    // Calculate unique components for reporting
    size_t unique_count = 0;
    if (baseline_groups == nullptr) {
        // Only strictly calculate this for the serial/baseline to avoid overhead on every run
        // Using the optimized normalizer logic to count
        std::vector<NodeIndex> map(result.group.size(), ULLONG_MAX);
        for (auto g : result.group) {
            if (map[g] == ULLONG_MAX) {
                map[g] = 1;
                unique_count++;
            }
        }
    }

    return {total_time / runs, correct, unique_count};
}

#include <iomanip>
#include <string>

int main() {
    // ---------------- CONFIGURATION ---------------- //
    const std::vector<NodeIndex> NODE_COUNTS = {
        50000,
        500000,
        5000000,
    };
    // const std::vector<NodeIndex> NODE_COUNTS = {2.5M, 5m, 10m, 20m, 40m, 80, 160m};
    const std::vector<double> EDGE_RATIOS = {0.1, 0.5, 1.0, 2.0};
    // pack exclusive
    // processors: 1,2,4,8,16,...,64
    const int RUNS = 3;

    // Set threads.
    const std::vector<int> THREAD_COUNTS = {1, 2, 6, 12};
    const int MAX_THREADS = 12;

    // Graph *graph = create_graph(5000000, 2500000);

    // auto components = graph->get_connected_components_serial();

    // // Print component sizes as CSV
    // std::unordered_map<NodeIndex, size_t> component_sizes;
    // for (NodeIndex comp_id : components.group) {
    //     component_sizes[comp_id]++;
    // }

    // std::cout << "ComponentID,Size\n";
    // for (std::unordered_map<NodeIndex, size_t>::const_iterator it = component_sizes.begin();
    //      it != component_sizes.end(); ++it) {
    //     std::cout << it->first << "," << it->second << "\n";
    // }

    // exit(0);

    // CSV Header
    std::cout << "Nodes,Ratio,Edges,Comps,Threads,Algorithm,Time,Speedup,SerialSpeedup,Status\n";

    for (NodeIndex n_count : NODE_COUNTS) {
        for (double ratio : EDGE_RATIOS) {
            long long e_count = static_cast<long long>(n_count * ratio);

            // 1. Create Graph (Reuse for all algos in this batch)
            Graph *graph = create_graph(n_count, e_count);

            // 2. Run Serial Baseline
            using AlgoFunc = std::function<ConnectedComponents()>;
            AlgoFunc serial_func = [&]() { return graph->get_connected_components_serial(); };

            TestResult serial_res = run_benchmark("Serial", serial_func, RUNS, nullptr);

            // Get baseline groups for verification (requires one more run to capture output)
            std::vector<NodeIndex> baseline_groups = graph->get_connected_components_serial().group;

            // Output Serial Row (speedup always 1.0 for itself, serial_speedup also 1.0)
            std::cout << n_count << "," << ratio << "," << e_count << ","
                      << serial_res.component_count << ",1,Serial," << std::fixed
                      << std::setprecision(5) << serial_res.avg_time << "," << std::setprecision(2)
                      << 1.0 << "," << 1.0 << "," << (serial_res.correct ? "OK" : "FAIL") << "\n";

            // 3. Define all parallel algorithms
            struct AlgoDef {
                std::string name;
                std::function<ConnectedComponents(int)> func;
            };

            std::vector<AlgoDef> par_algos = {
                {"Paper1",
                 [&](int t) { return graph->get_connected_components_Shiloach_Vishkin(t); }},
                {"Parallel3",
                 [&](int t) { return graph->get_connected_components_parallel_3(t, false); }},
                {"SV_improvement",
                 [&](int t) { return graph->get_connected_components_SV_impr(t); }}};

            // 4. For each algorithm, compute its own baseline with 1 thread
            for (const auto &alg : par_algos) {
                // Compute baseline for this algorithm with 1 thread
                AlgoFunc baseline_func = [&]() { return alg.func(1); };
                TestResult baseline_res = run_benchmark((alg.name + "_1thread").c_str(),
                                                        baseline_func, RUNS, &baseline_groups);

                // Run and report for all thread counts
                for (int t_count : THREAD_COUNTS) {
                    AlgoFunc algo_func = [&]() { return alg.func(t_count); };
                    TestResult res =
                        run_benchmark(alg.name.c_str(), algo_func, RUNS, &baseline_groups);

                    // Speedup relative to this algorithm's own 1-thread baseline
                    double speedup = baseline_res.avg_time / res.avg_time;

                    // Speedup relative to serial baseline
                    double serial_speedup = serial_res.avg_time / res.avg_time;

                    std::cout << n_count << "," << ratio << "," << e_count << ","
                              << serial_res.component_count << "," << t_count << "," << alg.name
                              << "," << std::fixed << std::setprecision(5) << res.avg_time << ","
                              << std::setprecision(2) << speedup << "," << serial_speedup << ","
                              << (res.correct ? "OK" : "FAIL") << "\n";
                }
            }

            delete graph;
        }
    }

    return 0;
}
