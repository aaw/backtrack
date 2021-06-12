#!/usr/bin/python3

# Usage: python3 complete.py n
#
# Generates K_n, the complete graph on n vertices.

import io
import itertools
import sys

def complete(n):
    buffer = io.StringIO()
    for i in range(n-1):
        for j in range (i+1,n):
            buffer.write("e {} {}\n".format(i+1,j+1))
    return 'p edge {0} {1}\n'.format(n, n*(n-1)//2) + buffer.getvalue()

if __name__ == '__main__':
    try:
        assert(len(sys.argv) == 2)
        n = int(sys.argv[1])
    except:
        print('Usage: "complete.py n" for integer n > 0')
        sys.exit(-1)
    print(complete(n))
