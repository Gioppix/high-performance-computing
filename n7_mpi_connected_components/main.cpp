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
};

Graph *create_graph(NodeIndex count, long long edge_count, unsigned threads_count) {
    std::vector<Node> nodes(count);

    // Thread-local edge lists to avoid contention
    std::vector<std::vector<std::pair<NodeIndex, NodeIndex>>> thread_edges(threads_count);

    // Parallel edge generation - just pick random (u, v) pairs directly

    // clang-format off
    #pragma omp parallel num_threads(threads_count)
    // clang-format on
    {
        int tid = omp_get_thread_num();
        // Fast xorshift64 RNG
        uint64_t rng = 12345ULL + tid * 1000003ULL;
        auto fast_rand = [&rng](NodeIndex max) -> NodeIndex {
            rng ^= rng << 13;
            rng ^= rng >> 7;
            rng ^= rng << 17;
            return rng % max;
        };

        long long my_edges =
            (edge_count * (tid + 1)) / threads_count - (edge_count * tid) / threads_count;
        thread_edges[tid].reserve(my_edges);

        for (long long i = 0; i < my_edges; i++) {
            NodeIndex u = fast_rand(count);
            NodeIndex v = fast_rand(count - 1);
            if (v >= u)
                v++; // Ensure v != u
            thread_edges[tid].emplace_back(u, v);
        }
    }

    // Merge into graph
    for (unsigned t = 0; t < threads_count; t++) {
        for (auto [u, v] : thread_edges[t]) {
            nodes[u].neighbors.push_back(v);
            nodes[v].neighbors.push_back(u);
        }
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

int main() {
    const NodeIndex NODE_COUNT = 5000000;
    const long long EDGE_COUNT = NODE_COUNT * 6;

    std::cout << "Node count: " << NODE_COUNT << "\n";
    std::cout << "Edge count: " << EDGE_COUNT << "\n";

    const unsigned threads_count = 12;
    const int RUNS = 2;

    Graph *graph = create_graph(NODE_COUNT, EDGE_COUNT, threads_count);

    auto [t_serial, groups_serial] =
        benchmark("Serial", RUNS, [&]() { return graph->get_connected_components_serial(); });

    graph->get_connected_components_parallel_3(threads_count, true);
    // exit(0);

    auto [t_parallel_1, groups_parallel_1] = benchmark("Parallel 1", RUNS, [&]() {
        return graph->get_connected_components_parallel_1(threads_count);
    });

    auto [t_parallel_2, groups_parallel_2] = benchmark("Parallel 2", RUNS, [&]() {
        return graph->get_connected_components_parallel_2(threads_count);
    });

    auto [t_parallel_3, groups_parallel_3] = benchmark("Parallel 3", RUNS, [&]() {
        return graph->get_connected_components_parallel_3(threads_count);
    });

    // Verify that serial and parallel results are equivalent
    bool found_different = compare_connected_components("Serial", groups_serial,
                                                        {
                                                            {"Parallel 1", groups_parallel_1},
                                                            {"Parallel 2", groups_parallel_2},
                                                            {"Parallel 3", groups_parallel_3},
                                                        });

    // test_efficiency_and_scalability(
    //     "Parallel 2", [&](unsigned tc) { return graph->get_connected_components_parallel_2(tc);
    //     }, threads_count, RUNS);

    // test_efficiency_and_scalability(
    //     "Parallel 3", [&](unsigned tc) { return graph->get_connected_components_parallel_3(tc);
    //     }, threads_count, RUNS);

    return static_cast<int>(found_different);
}
