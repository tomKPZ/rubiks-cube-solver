solver: solver.c
	clang solver.c -Wall -Wextra -Werror -pedantic -std=gnu18 -ggdb3 -Ofast -march=native -mtune=native -o solver

.PHONY: run
run: solver
	time ./solver
