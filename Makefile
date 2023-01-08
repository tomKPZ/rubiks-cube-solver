solver: solver.c types.h tables.c
	clang solver.c -Wall -Wextra -Werror -pedantic -std=gnu18 -ggdb3 -Ofast -march=native -mtune=native -o solver -pthread

tables.c: gen_tables.py
	python3 gen_tables.py > tables.c

.PHONY: run
run: solver
	time ./solver
