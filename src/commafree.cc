#include "counters.h"
#include "flags.h"
#include "logging.h"

#include <cstdint>
#include <vector>

typedef uint16_t addr_t;
typedef uint16_t mem_t;

enum State {
    RED = 0,
    BLUE = 1,
    GREEN = 2,
};

uint16_t u16(size_t x) {
    // TODO: add assert in debug mode
    return static_cast<uint16_t>(x);
}

const char* print_hex(uint16_t h) {
    static char x[5];
    sprintf(x, "%04x", h);
    return x;
}

struct uframe_t {
    addr_t addr;
    mem_t val;
};

struct Backtrack {
    size_t m;
    size_t m4;
    size_t M;
    size_t L;
    size_t p1off;
    size_t p2off;
    size_t p3off;
    size_t s1off;
    size_t s2off;
    size_t s3off;
    size_t cloff;
    size_t poison;
    std::vector<mem_t> mem;
    std::vector<size_t> stamp; // TODO: what type here? how big are stamps?
    uint16_t sigma;
    std::vector<mem_t> free;
    std::vector<mem_t> ifree;
    std::vector<mem_t> X;
    std::vector<mem_t> C;
    std::vector<mem_t> S;
    std::vector<mem_t> U;
    std::vector<uframe_t> undo;
    std::vector<mem_t> hex2m;
    std::vector<mem_t> m2hex;
    std::vector<mem_t> cl;  // Maps base-m word to prime string class ID
    std::vector<mem_t> clrep;  // Mapss class IDs to canonical base-m words
    size_t l;
    mem_t x;
    size_t c;
    mem_t s;
    mem_t f;

    inline size_t p1head(mem_t) { return p1off + hex2m[x & 0xf000]; }
    inline size_t p2head(mem_t) { return p2off + hex2m[x & 0xff00]; }
    inline size_t p3head(mem_t) { return p3off + hex2m[x & 0xfff0]; }
    inline size_t s1head(mem_t) { return s1off + hex2m[(x & 0x000f)<<12]; }
    inline size_t s2head(mem_t) { return s2off + hex2m[(x & 0x00ff)<<8]; }
    inline size_t s3head(mem_t) { return s3off + hex2m[(x & 0x0fff)<<4]; }
    inline size_t clhead(mem_t) { return cloff + 4 * cl[hex2m[x]]; }

    inline size_t invp1(mem_t) { return p1off - m4 + hex2m[x]; }
    inline size_t invp2(mem_t) { return p2off - m4 + hex2m[x]; }
    inline size_t invp3(mem_t) { return p3off - m4 + hex2m[x]; }
    inline size_t invs1(mem_t) { return s1off - m4 + hex2m[x]; }
    inline size_t invs2(mem_t) { return s2off - m4 + hex2m[x]; }
    inline size_t invs3(mem_t) { return s3off - m4 + hex2m[x]; }
    inline size_t invcl(mem_t) { return cloff - m4 + hex2m[x]; }

    inline size_t p1tail(mem_t) { return p1off + m4 + hex2m[x & 0xf000]; }
    inline size_t p2tail(mem_t) { return p2off + m4 + hex2m[x & 0xff00]; }
    inline size_t p3tail(mem_t) { return p3off + m4 + hex2m[x & 0xfff0]; }
    inline size_t s1tail(mem_t) { return s1off + m4 + hex2m[(x & 0x000f)<<12]; }
    inline size_t s2tail(mem_t) { return s2off + m4 + hex2m[(x & 0x00ff)<<8]; }
    inline size_t s3tail(mem_t) { return s3off + m4 + hex2m[(x & 0x0fff)<<4]; }
    inline size_t cltail(mem_t) { return cloff + m4 + cl[hex2m[x]]; }

    #define INSERT(h, LIST) { \
        mem_t tail = mem[LIST ## tail((h))]; \
        mem[tail] = (h); \
        mem[inv ## LIST((h))] = tail; \
        mem[LIST ## tail((h))] = tail+1; \
    }

    Backtrack(mem_t m, mem_t g) :
        m(m),
        m4(m*m*m*m),
        M(23.5*m4),
        L((m4 - m*m)/4),
        p1off(2*m4),
        p2off(5*m4),
        p3off(8*m4),
        s1off(11*m4),
        s2off(14*m4),
        s3off(17*m4),
        cloff(20*m4),
        poison(22*m4),
        mem(M),
        stamp(M),
        sigma(0),
        free(L),
        ifree(L),
        X(L+1),
        C(L+1),
        S(L+1),
        U(L+1),
        hex2m(make_hex(m-1,m-1,m-1,m-1)+1),
        m2hex(m4),
        cl(m4),
        clrep((m4 - m*m) / 4),
        l(1),
        x(0x0001),
        c(0),
        s(L - g),
        f(L) {
        // C1. [Initialize.]
        CHECK_NO_OVERFLOW(mem_t, M) << "m = " << m << " creates data "
            "structures too big. Increase width of mem_t and recompile.";
        mem[poison-1] = poison;
        for (size_t i = 0; i < L; ++i) {
            free[i] = ifree[i] = i;
        }

        // Populate conversion tables for tranlating between  hex-encoded words
        // (0x1234 means '1234') and base-m encoded words (1234_m means '1234').
        for (mem_t a = 0; a < m; ++a) {
            for (mem_t b = 0; b < m; ++b) {
                for (mem_t c = 0; c < m; ++c) {
                    for (mem_t d = 0; d < m; ++d) {
                        hex2m[make_hex(a,b,c,d)] = make_m(a,b,c,d);
                        m2hex[make_m(a,b,c,d)] = make_hex(a,b,c,d);
                    }
                }
            }
        }

        // Initialize prefixes and suffixes
        // TODO

        // Populate classes. This is essentially Algorithm 7.2.1.1.F, iterating
        // over all prime strings of base m, length 4.
        mem_t a[5] = {0, 0, 0, 0, 0};

        size_t class_id = 0;
        for (int i = 1; i != 0;) {
            if (i == 4) {
                uint16_t h = make_hex(a[1],a[2],a[3],a[4]);
                cl[hex2m[h]] = class_id;
                clrep[class_id] = hex2m[h];
                INSERT(h, p1);
                INSERT(h, p2);
                INSERT(h, p3);
                INSERT(h, s1);
                INSERT(h, s2);
                INSERT(h, s3);

                // TODO: Add all shifts of a_m that are blue to clhead/cltail
                // TODO: Add all shifts to clinv

                class_id++;
            }
            i = 4;
            while (a[i] == m-1) { --i; }
            ++a[i];
            for (int j = i+1; j <= 4; ++j) { a[j] = a[j-i]; }
        }
    }

    inline uint16_t make_hex(mem_t a, mem_t b, mem_t c, mem_t d) {
        return a<<12 | b<<8 | c<<4 | d;
    }

    inline uint16_t make_m(mem_t a, mem_t b, mem_t c, mem_t d) {
        return ((a*m+b)*m+c)*m+d;
    }

    void bump_stamp() {
        sigma++;
        if (sigma == 0) {  // Overflow.
            for (size_t i = 0; i < stamp.size(); ++i) { stamp[i] = 0; }
            sigma = 1;
        }
    }

    void try_again() {
        // C5. [Try again.]
        while (undo.size() > U[l]) {
            mem[undo.back().addr] = undo.back().val;
            undo.pop_back();
        }
        bump_stamp();
        // TODO: redden x.
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

    void store(size_t addr, mem_t val) {
        if (stamp[addr] != sigma) {
            stamp[addr] = sigma;
            undo.push_back({addr: u16(addr), val: mem[addr]});
        }
        mem[addr] = val;
    }

    // Choose a candidate word x and class c.
    void choose() {

    }

    void solve() {
        while (true) {
            // C3. [Try the candidate.]
            U[l] = undo.size();
            bump_stamp();
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
            choose();
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
    Backtrack(FLAGS_size, FLAGS_diff).solve();
    return 0;
}
