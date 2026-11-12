#!/bin/bash
cd /shared/agent-13
gcc -O3 -march=native -fopenmp -fno-math-errno -fno-trapping-math -fno-signed-zeros \
    -ffp-contract=fast -fstrict-aliasing -fPIC -Wall -Wextra -std=c23 \
    -D_POSIX_C_SOURCE=199309L -fPIC -c rb_sor.c -o /tmp/kernel.c.o || exit 1
gcc -O3 -march=native -fopenmp -fno-math-errno -fno-trapping-math -fno-signed-zeros \
    -ffp-contract=fast -fstrict-aliasing -fPIC -Wall -Wextra -std=c23 \
    -D_POSIX_C_SOURCE=199309L -fPIC -c test.c -o /tmp/test.o || exit 1
gcc -O3 -march=native -fopenmp /tmp/kernel.c.o /tmp/test.o -o /tmp/test -lm || exit 1
echo BUILD OK
