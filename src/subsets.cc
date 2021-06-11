#include "counters.h"
#include "flags.h"
#include "logging.h"

#include <vector>
#include <sstream>

using std::vector;

struct Edge {
    Edge(int tip, Edge* next) : tip(tip), next(next) {}

    ~Edge() { if (next != nullptr) delete next; }

    int tip;
    Edge* next;
};

struct Graph {
    Graph(int n) : adj(n, nullptr), m(0) {}

    ~Graph() { for (Edge* head : adj) { if (head != nullptr) delete head; } }

    void add_edge(int u, int v) {
        adj[u] = new Edge(v, adj[u]);
        adj[v] = new Edge(u, adj[v]);
        m++;
    }

    Edge* edges(int u) { return adj[u]; }

    int num_vertices() { return adj.size(); }
    int num_edges() { return m; }

private:
    vector<Edge*> adj;
    int m;
};

Graph parse_dimacs(const char* filename) {
    FILE* f = fopen(filename, "r");
    CHECK(f) << "Failed to open file: " << filename;
    long long n = 0, m = 0;
    // Read comments until "p edge $n $m" header.
    int read;
    do {
        read = fscanf(f, " p edge %lld %lld \n", &n, &m);
        if (read > 0 && read != EOF) break;
        read = fscanf(f, "%*s\n");
    } while (read != 2 && read != EOF);
    CHECK(n >= 0) << "Variable count must be non-negative.";
    CHECK(m >= 0) << "Clause count must be non-negative.";

    Graph g(n);

    // Read edges until EOF.
    long long u = 0, v = 0;
    while (true) {
        read = fscanf(f, " e %lld %lld \n", &u, &v);
        if (read != 2) break;
        if (read == EOF) break;
        g.add_edge(u-1,v-1);  // DIMACS nodes are numbered [1,n]
    }
    CHECK(m == g.num_edges()) << "Mismatch in edge count. Header says "
                              << m << ", found " << g.num_edges();

    fclose(f);
    return g;
}

struct State {
    int v;
    int i;
    Edge* a;
};

void connected_subsets(Graph& g, int v, int n) {
    // R1. [Initialize.]
    vector<int> tag(g.num_vertices());
    vector<State> state(g.num_vertices());
    state[0].v = v;
    int i = state[0].i = 0;
    Edge* a = state[0].a = g.edges(v);
    tag[v] = 1;
    int l = 1;

    while (true) {
        // R4. [Done with level?]
        if (a == nullptr && i == l-1) {
            // R6. [Backtrack.]
            --l;
            if (l == 0) break;
            i = state[l].i;
            // Untag all neighbors of v_k, for i < k <= l.
            for (int k = i+1; k <= l; ++k) {
                Edge* ptr = g.edges(state[k].v);
                while (ptr != nullptr) {
                    tag[ptr->tip]--;
                    ptr = ptr->next;
                }
            }
            a = state[l].a->next;
            while (a != nullptr) {
                tag[a->tip]--;
                a = a->next;
            }
            a = state[l].a;
            // R3. [Advance a.]
            a = a->next;
            continue;  // -> R4.
        } else if (a == nullptr) {
            ++i;
            v = state[i].v;
            a = g.edges(v);
        }

        // R5. [Try a.]
        int u = a->tip;
        ++tag[u];
        if (tag[u] > 1) {
            // R3. [Advance a.]
            a = a->next;
            continue;
        }

        state[l].i = i;
        state[l].a = a;
        state[l].v = u;
        ++l;
        // R2. [Enter level l.]
        CHECK(i == state[l-1].i)
            << "Want i == i_{l-1} but " << i << " != " << state[l-1].i;
        //CHECK(v == state[i].v)
        //    << "Want v == v_i but " << v << " != " << state[i].v;
        CHECK(a == state[l-1].a)
            << "Want a == a_{l-1} but " << a << " != " << state[l-1].a;
        if (l == n) {
            INC(solutions);
            if (LOG_ENABLED(2)) {
                std::ostringstream oss;
                for (int k = 0; k < n; ++k) {
                    oss << state[k].v << " ";
                }
                LOG(2) << "Solution: " << oss.str();
            }
            l = n-1;
        }
        // R3. [Advance a.]
        a = a->next;
    }
}

int main(int argc, char** argv) {
    int oidx;
    CHECK(parse_flags(argc, argv, &oidx))
        << "Usage: " << argv[0] << " -nN [-vV] DIMACS-graph-file\n"
        << "N: number of vertices in connected subset (> 0)\n"
        << "V: verbosity (>= 2 prints solutions)\n";
    init_counters();
    LOG(1) << "Reading graph from file " << argv[argc-1];
    Graph g = parse_dimacs(argv[argc-1]);
    LOG(1) << "Finding connected subsets with n = " << FLAGS_size;
    connected_subsets(g, 0, FLAGS_size);
    return 0;
}
