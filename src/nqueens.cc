#include "counters.h"
#include "flags.h"
#include "logging.h"

#include <vector>
#include <string>
#include <sstream>

struct NQueens {
    NQueens(size_t n) :
        x(n), a(n), b(2*n-1), c(2*n-1), l(0), t(0), n(n) { }

    inline void track() {
        a[t] = b[t+l] = c[t-l+n] = true;
        x[l] = t;
        ++l;
    }

    inline void backtrack() {
        while (t >= n-1) {
            if (l == 0) exit(0);
            --l;
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

    std::vector<size_t> x;
    std::vector<bool> a, b, c;
    size_t l, t, n;
};

int main(int argc, char** argv) {
    int oidx;
    CHECK(parse_flags(argc, argv, &oidx))
        << "Usage: " << argv[0] << " <filename>";
    init_counters();
    LOG(1) << "Solving n queens with n = " << FLAGS_size;
    CHECK(FLAGS_size > 0) << "Please pass n > 0 (-n)";
    NQueens n(FLAGS_size);
    n.solve();
    return 0;
}
