#ifndef __FLAGS_H__
#define __FLAGS_H__

#include <iostream>
#include <limits>
#include <getopt.h>
#include <string>

#include "logging.h"

// To add and use a new flag:
// (1) Declare it and its default below globally as FLAGS_xxx = default.
// (2) Declare an extern reference to it in the module where you want to use it.
// (3) Add an entry to long_options[] and optstring[] below defining its parse.
// (4) Add a case in the switch statement below to handle setting the flag.
// (5) Add a sentence to the help text displayed with -h.

int FLAGS_verbosity = 1;
bool FLAGS_counters = true;
size_t FLAGS_size = 0;
size_t FLAGS_diff = 0;

bool parse_flags(int argc, char* argv[], int* option_index) {
    *option_index = 0;
    int c;

    struct option long_options[] = {
        { "verbosity",      required_argument,  NULL, 'v' },
        { "size",           required_argument,  NULL, 'n' },
        { "counters",       no_argument,        NULL, 'c' },
        { "diff",           required_argument,  NULL, 'd' },
        { 0, 0, 0, 0}
    };

    char optstring[] = "v:n:d:c";

    while (1) {
        c = getopt_long(argc, argv, optstring, long_options, nullptr);
        if (c == -1)
            break;

        switch (c) {
        case 'v':
            FLAGS_verbosity = atoi(optarg);
            break;
        case 'c':
            FLAGS_counters = true;
            break;
        case 'd':
            FLAGS_diff = atoi(optarg);
            break;
        case 'n':
            FLAGS_size = atoi(optarg);
            break;
        default:
            return false;
        }
    }

    if (FLAGS_size <= 0) return false;  // -n required.

    *option_index = optind;
    return true;
}


#endif // __FLAGS_H__
