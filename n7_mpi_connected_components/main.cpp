#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <omp.h>
#include <unordered_map>
#include <utility>
#include <vector>

struct Node {
    std::vector<unsigned> neighbors;
};

struct ConnectedComponents {
    std::vector<std::vector<unsigned>> components;
};

struct Graph {
    std::vector<Node> nodes;

    Graph(std::vector<Node> nodes) : nodes(std::move(nodes)) {
    }

    ConnectedComponents get_connected_components_serial() const {
        ConnectedComponents result;
        std::vector<bool> visited(nodes.size(), false);

        for (unsigned i = 0; i < nodes.size(); i++) {
            if (!visited[i]) {
                std::vector<unsigned> component;
                std::vector<unsigned> stack;

                stack.push_back(i);
                visited[i] = true;

                while (!stack.empty()) {
                    unsigned current = stack.back();
                    stack.pop_back();
                    component.push_back(current);

                    for (unsigned neighbor : nodes[current].neighbors) {
                        if (!visited[neighbor]) {
                            visited[neighbor] = true;
                            stack.push_back(neighbor);
                        }
                    }
                }

                result.components.push_back(component);
            }
        }

        return result;
    }

    ConnectedComponents get_connected_components_parallel(unsigned threads_count) const {
        std::vector<omp_lock_t> group_locks(nodes.size());
        for (unsigned i = 0; i < nodes.size(); i++) {
            omp_init_lock(&group_locks[i]);
        }

        struct GroupNode {
            GroupNode *parent;
            unsigned index;
        };

        std::vector<GroupNode *> groups(nodes.size());
        for (unsigned i = 0; i < nodes.size(); i++) {
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

#pragma omp parallel for num_threads(threads_count)
        for (unsigned node_index = 0; node_index < nodes.size(); node_index++) {
            for (unsigned adj_index : nodes[node_index].neighbors) {
                // Lock in consistent order to avoid deadlock
                unsigned lock_first = std::min(node_index, adj_index);
                unsigned lock_second = std::max(node_index, adj_index);

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

        // Find root for each node and group nodes by their root
        std::vector<unsigned> roots(nodes.size());
        for (unsigned i = 0; i < nodes.size(); i++) {
            GroupNode *current = groups[i];
            while (current->parent != nullptr) {
                current = current->parent;
            }
            roots[i] = current->index;
        }

        // Map roots to component indices using hash map for O(1) lookup
        std::unordered_map<unsigned, unsigned> root_to_component;
        for (unsigned i = 0; i < nodes.size(); i++) {
            if (root_to_component.find(roots[i]) == root_to_component.end()) {
                root_to_component[roots[i]] = result.components.size();
                result.components.push_back(std::vector<unsigned>());
            }
        }

        // Assign nodes to their components
        for (unsigned i = 0; i < nodes.size(); i++) {
            result.components[root_to_component[roots[i]]].push_back(i);
        }

        // Clean up
        for (unsigned i = 0; i < nodes.size(); i++) {
            delete groups[i];
        }
        for (unsigned i = 0; i < nodes.size(); i++) {
            omp_destroy_lock(&group_locks[i]);
        }
        return result;
    }
};

Graph *create_graph(unsigned count, int edge_count) {
    std::vector<Node> nodes(count);
    std::srand(0);

    // Total possible edges in undirected graph without self-loops
    long long total_edges = static_cast<long long>(count) * (count - 1) / 2;

    if (edge_count > total_edges) {
        std::cerr << "ERROR: edge_count (" << edge_count << ") exceeds total possible edges ("
                  << total_edges << ")\n";
        std::exit(1);
    }

    // Use hash map for O(1) swap lookups - virtual Fisher-Yates without full array
    // Only stores elements that have been swapped, rest are implicitly identity
    std::unordered_map<long long, long long> swapped;

    auto get_value = [&swapped](long long idx) -> long long {
        auto it = swapped.find(idx);
        return it != swapped.end() ? it->second : idx;
    };

    auto index_to_edge = [count](long long idx) -> std::pair<unsigned, unsigned> {
        unsigned u = 0;
        long long edges_before_u = 0;
        while (edges_before_u + (count - 1 - u) <= idx) {
            edges_before_u += (count - 1 - u);
            u++;
        }
        unsigned v = u + 1 + (idx - edges_before_u);
        return {u, v};
    };

    // Virtual Fisher-Yates: only track swapped positions
    for (int i = 0; i < edge_count; i++) {
        long long j = i + rand() % (total_edges - i);

        long long val_i = get_value(i);
        long long val_j = get_value(j);

        // Swap
        swapped[i] = val_j;
        if (j != i) {
            swapped[j] = val_i;
        }

        // Add edge
        auto [u, v] = index_to_edge(val_j);
        nodes[u].neighbors.push_back(v);
        nodes[v].neighbors.push_back(u);
    }

    return new Graph(nodes);
}

int main() {
    const int NODE_COUNT = 10000;
    const double PERCENTAGE_CONNECTED = 0.75;
    const int EDGE_COUNT =
        static_cast<int>(NODE_COUNT * (NODE_COUNT - 1) / 2 * PERCENTAGE_CONNECTED);

    const unsigned threads_count = 12;

    Graph *graph = create_graph(NODE_COUNT, EDGE_COUNT);

    double start_time = omp_get_wtime();

    auto groups_serial = graph->get_connected_components_serial();

    double end_time = omp_get_wtime();

    std::cout << "Serial time: " << (end_time - start_time) << " seconds\n";

    start_time = omp_get_wtime();

    auto groups_parallel = graph->get_connected_components_parallel(threads_count);

    end_time = omp_get_wtime();

    std::cout << "Parallel time: " << (end_time - start_time) << " seconds\n";

    // Verify that serial and parallel results are equivalent
    if (groups_serial.components.size() != groups_parallel.components.size()) {
        std::cout << "ERROR: Different number of components! Serial: "
                  << groups_serial.components.size()
                  << ", Parallel: " << groups_parallel.components.size() << "\n";
    } else {
        // Sort components for comparison
        auto sort_components = [](ConnectedComponents &cc) {
            for (auto &comp : cc.components) {
                std::sort(comp.begin(), comp.end());
            }
            std::sort(cc.components.begin(), cc.components.end());
        };

        sort_components(groups_serial);
        sort_components(groups_parallel);

        bool equal = true;
        for (size_t i = 0; i < groups_serial.components.size(); i++) {
            if (groups_serial.components[i] != groups_parallel.components[i]) {
                equal = false;
                break;
            }
        }

        if (equal) {
            std::cout << "SUCCESS: Serial and parallel results are equivalent!\n";
        } else {
            std::cout << "ERROR: Serial and parallel results differ!\n";
        }
    }

    // #pragma omp parallel for num_threads(thread_count) reduction(&& : all_reach_one)
    // #pragma omp critical

    return 0;
}
