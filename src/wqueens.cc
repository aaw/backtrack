#include "counters.h"
#include "flags.h"
#include "logging.h"

#include <cmath>
#include <vector>
#include <string>
#include <sstream>

struct NQueens {
    // The n-queens problem asks for a configuration of n queens on an n-by-n
    // chessboard with no two queens attacking each other. In other words, we
    // want a permutation x of [0,n) such that:
    //
    //     |x[k] - x[j]| != k - j for 0 <= j < k < n
    //
    // TODO

    std::vector<uint64_t> a, b, c, s;

    uint64_t mask;
    size_t l;  // Backtrack level. l == n means we have a solution.
    size_t n;  // Number of queens in the problem. Chessboard is n x n.

    NQueens(size_t n) :
        a(n), b(n), c(n), s(n), mask((1LL << n) - 1), l(0), n(n) { }

    inline void prepare() {
        s[l] = mask & ~a[l] & ~b[l] & ~c[l];
    }

    inline void backtrack() {
        if (l-- == 0) exit(0);
    }

    inline bool advance() {
        uint64_t t = s[l] & -s[l];
        if (t == 0) return false;
        a[l+1] = a[l] + t;
        b[l+1] = (b[l] + t) >> 1;
        c[l+1] = ((c[l] + t) << 1) & mask;
        s[l] = s[l] - t;
        ++l;
        return true;
    }

    inline void visit() {
        if (LOG_ENABLED(2)) {
            std::ostringstream oss;
            for (size_t ll = l; ll > 0; --ll) {
                oss << log2(a[ll] - a[ll-1]) + 1 << " ";
            }
            LOG(2) << "Solution: " << oss.str();
        }
        INC(solutions);
    }

    void solve() {
        while (true) {
            if (l > n-1) { visit(); backtrack(); } else { prepare(); }
            while (!advance()) { backtrack(); }
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
