#include "counters.h"
#include "flags.h"
#include "logging.h"

#include <vector>

#define mem_t uint16_t

struct State {
    size_t M;
    size_t L;
    std::vector<mem_t> mem;
    std::vector<size_t> stamp; // TODO: what type here? how big are stamps?
    size_t sigma;
    mem_t poison;
    std::vector<mem_t> free;
    std::vector<mem_t> ifree;
    std::vector<mem_t> X;
    std::vector<mem_t> C;
    std::vector<mem_t> S;
    std::vector<mem_t> U;
    std::vector<mem_t> undo;
    size_t l;
    mem_t x;
    size_t c;
    mem_t s;
    mem_t f;
    mem_t u;

    State(mem_t m, mem_t g) :
        M(23.5*m*m*m*m),
        L((m*m*m*m - m*m)/4),
        mem(M),
        stamp(M),
        sigma(0),
        poison(22*m*m*m*m),
        free(L),
        ifree(L),
        X(L+1),
        C(L+1),
        S(L+1),
        U(L+1),
        l(1),
        x(0x0001),
        c(0),
        s(L - g),
        f(L),
        u(0) {
        // C1. [Initialize.]
        CHECK_NO_OVERFLOW(mem_t, M) << "m = " << m << " creates data "
            "structures too big. Increase width of mem_t and recompile.";
        mem[poison-1] = poison;
        for (size_t i = 0; i < L; ++i) {
            free[i] = ifree[i] = i;
        }
    }

    void try_again() {
        // C5. [Try again.]
        while (u > U[l]) {
            u--;
            mem[undo[u] >> 16] = undo[u] & 0xffff;
            sigma++;
            // TODO: redden x.
        }
    }

    void backtrack() {
        // C6. [Backtrack.]
        do {
            l--;
            if (l == 0) exit(0);
            x = X[l];
            c = C[l];
            f++;
        } while (x < 0);
        s = S[l];
        try_again();
    }

    void make_the_move() {
        // C4. [Make the move.]
        X[l] = x;
        C[l] = c;
        S[l] = s;
        mem_t p = ifree[c];
        f--;
        if (p != f) {
            mem_t y = free[f];
            free[p] = y;
            ifree[y] = p;
            free[f] = c;
            ifree[c] = f;
        }
        l++;
    }

    void visit() {
        // TODO
    }

    void solve() {
        while (true) {
            // C3. [Try the candidate.]
            U[l] = u;
            sigma++;
            if (x < 0) {
                if (s == 0 || l == 1) {
                    backtrack();
                }
                else {
                    s--;
                    make_the_move();
                }
            } else { // x >= 0
                // TODO: update the data structures to make x green
                // -> C5 if trouble -> C2
                make_the_move();
            }

            // C2. [Enter level l.]
            while (l > L) {
                visit();
                backtrack();
            }
            // TODO: choose candidate word x and class c, -> C3
        }
    }
};

int main(int argc, char** argv) {
    int oidx;
    CHECK(parse_flags(argc, argv, &oidx))
        << "Usage: " << argv[0] << " -nN [-dD] [-vV]\n"
        << "N: alphabet size\n"
        << "D: diff size (search for code of size OPT - D)\n"
        << "V: verbosity (>= 2 prints solutions)\n";
    init_counters();
    LOG(1) << "Solving with n = " << FLAGS_size << ", d = " << FLAGS_diff;
    State(FLAGS_size, FLAGS_diff).solve();
    return 0;
}
