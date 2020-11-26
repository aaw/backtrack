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
    // In contrast to nqueens.cc, this program uses R.J. Walker's method of
    // maintaining a set S[l] at each level l of the possible candidates for
    // x[l]. If we define:
    //
    //     S[l] = [0,n) - A[l] - B[l] - C[l], where
    //
    //     A[l] = { x_j         | 0 <= j < l }
    //     B[l] = { x_j + j - l | 0 <= j < l }
    //     C[l] = { x_j - j + l | 0 <= j < l }
    //
    // Then each x[l] we select just needs to be added to A,B, and C before
    // continuing to the next level. And if we maintain A,B,C as bit fields
    // then S[l+1] is easy to compute from S[l] since generating A[l+1] from
    // A[l] is trivial and B[l+1] and C[l+1] can be generated from B[l] and
    // C[l] using bit-shifts. Backtracking is trivial because there's nothing
    // to undo!

    std::vector<uint64_t> a, b, c, s;

    uint64_t mask;  // 2^n - 1
    size_t l;       // Backtrack level. l == n means we have a solution.
    size_t n;       // Number of queens in the problem. Chessboard is n x n.

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
        s[l] -= t;
        ++l;
        return true;
    }

    inline void visit() {
        INC(solutions);
        if (LOG_ENABLED(2)) {
            std::ostringstream oss;
            for (size_t ll = l; ll > 0; --ll) {
                oss << log2(a[ll] - a[ll-1]) + 1 << " ";
            }
            LOG(2) << "Solution: " << oss.str();
        }
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
    CHECK(FLAGS_size < 65) << "-n must be less than 65.";
    LOG(1) << "Solving n queens with n = " << FLAGS_size;
    NQueens(FLAGS_size).solve();
    return 0;
}
