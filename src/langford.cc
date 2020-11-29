#include "counters.h"
#include "flags.h"
#include "logging.h"

#include <vector>
#include <sstream>

struct Langford {
    std::vector<int32_t> x;
    std::vector<size_t> p, y;

    size_t n, k, l, j;

    Langford(size_t n) : x(2*n+1), p(n+1), y(2*n+1), n(n), l(1) {
        for (size_t i = 0; i < n; ++i) {
            p[i] = i+1;
        }
    }

    inline void visit() {
        INC(solutions);
        if (LOG_ENABLED(2)) {
            std::ostringstream oss;
            for (size_t i = 1; i < x.size(); ++i) {
                oss << x[i] << " ";
            }
            LOG(2) << "Solution: " << oss.str();
        }
    }

    inline void backtrack() {
        while (true) {
            --l;
            if (l <= 0) exit(0);
            while (x[l] < 0) --l;
            k = x[l];
            x[l] = x[l+k+1] = 0;
            p[y[l]] = j = k;
            k = p[j];
            if (k != 0) break;
        }
    }

    inline void prepare() {
        j = 0;
        while (x[l] < 0) ++l;
    }

    inline void advance() {
        x[l] = k;
        x[l+k+1] = -k;
        y[l] = j;
        p[j] = p[k];
        ++l;
    }

    void solve() {
        while (true) {
            k = p[0];
            if (k == 0) { visit(); backtrack(); } else { prepare(); }
            while (true) {
                while (l+k+1 > 2*n) { backtrack(); }
                if (x[l+k+1] == 0) { advance(); break; }
                j = k;
                k = p[j];
                if (k != 0) continue;
                backtrack();
            }
        }
    }
};

int main(int argc, char** argv) {
    int oidx;
    CHECK(parse_flags(argc, argv, &oidx))
        << "Usage: " << argv[0] << " -nN [-vV]\n"
        << "N: size of problem (number of pairs)\n"
        << "V: verbosity (>= 2 prints solutions)\n";
    init_counters();
    LOG(1) << "Finding a Langford pairing for 2n, n = " << FLAGS_size;
    Langford(FLAGS_size).solve();
    return 0;
}
