#!/usr/bin/python3

# Usage: python3 grid.py n
#
# Generates an n x n grid graph
#
# 0--1--2
# |  |  |
# 3--4--5
# |  |  |
# 6--7--8

import io
import itertools
import sys

def grid(n):
    buffer = io.StringIO()
    for i in range(n*n-1):
        if i % n == n-1: continue
        buffer.write("e {} {}\n".format(i+1,i+2))
    for i in range(n):
        for j in range(1,n):
            buffer.write("e {} {}\n".format(i+1+n*(j-1),i+1+n*j))
    return 'p edge {0} {1}\n'.format(n*n, 2*n*(n-1)) + buffer.getvalue()

if __name__ == '__main__':
    try:
        assert(len(sys.argv) == 2)
        n = int(sys.argv[1])
    except:
        print('Usage: "grid.py n" for integer n > 0')
        sys.exit(-1)
    print(grid(n))
