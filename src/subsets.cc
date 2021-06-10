#include "counters.h"
#include "flags.h"
#include "logging.h"

struct Graph {
    Graph(int n) : n(n), m(0) {}

    void add_edge(int u, int v) {
        m++;
    }

    int num_edges() { return m; }
private:
    int n;
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
        g.add_edge(u,v);
    }
    CHECK(m == g.num_edges()) << "Mismatch in edge count. Header says "
                              << m << ", found " << g.num_edges();

    fclose(f);
    return g;
}

struct Subsets {
    Subsets(Graph& g, size_t n) {}

    void solve() {}
};

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
    Subsets(g, FLAGS_size).solve();
    return 0;
}
