#include "counters.h"
#include "flags.h"
#include "logging.h"

#include <vector>
#include <string>
#include <sstream>

struct NQueens {
    std::vector<size_t> x;  // Current solution, permutation of n.

    // The n-queens problem asks for a configuration of n queens on an n-by-n
    // chessboard with no two queens attacking each other. In other words, we
    // want a permutation x of [0,n) such that:
    //
    //     |x[k] - x[j]| != k - j for 0 <= j < k < n
    //
    // We don't need to check all pairs of k, j each time we make a branch
    // decision during backtracking. Instead, we keep three bitvectors a, b, c
    // where:
    //
    // a[t]   means 'x[k] = t', for some k in [0,n).
    // b[t]   means 'x[k] + k = t' for some k in [0,n).
    // c[t+n] means 'x[k] - k = t' for some k in [0,n).
    //
    // Then x[l] can be set to t if all of a[t], b[t+l], and c[t-l+n] are false.
    // Otherwise, we continue searching.

    std::vector<bool> a, b, c;

    size_t l;  // Backtrack level. l == n means we have a solution.
    size_t t;  // Current choice we're considering for level l.
    size_t n;  // Number of queens in the problem. Chessboard is n x n.

    NQueens(size_t n) :
        x(n), a(n), b(2*n-1), c(2*n-1), l(0), t(0), n(n) { }

    inline void track() {
        a[t] = b[t+l] = c[t-l+n] = true;
        x[l++] = t;
    }

    inline void backtrack() {
        while (t >= n-1) {
            if (l-- == 0) exit(0);
            t = x[l];
            c[t-l+n] = b[t+l] = a[t] = false;
        }
    }

    inline bool blocked() {
        return a[t] || b[t+l] || c[t-l+n];
    }

    inline void visit() {
        INC(solutions);
        if (LOG_ENABLED(2)) {
            std::ostringstream oss;
            for (const size_t xx : x) { oss << xx+1 << " "; }
            LOG(2) << "Solution: " << oss.str();
        }
    }

    void solve() {
        while (true) {
            if (l > n-1) { visit(); backtrack(); ++t; }
            else { t = 0; }
            while (blocked()) { backtrack(); ++t; }
            track();
        }
    }
};

int main(int argc, char** argv) {
    int oidx;
    CHECK(parse_flags(argc, argv, &oidx))
        << "Usage: " << argv[0] << " -nN [-vV]\n"
        << "N: number of queens (> 0)\n"
        << "V: verbosity (>= 2 prints solutions)\n";
    init_counters();
    LOG(1) << "Solving n queens with n = " << FLAGS_size;
    NQueens(FLAGS_size).solve();
    return 0;
}
