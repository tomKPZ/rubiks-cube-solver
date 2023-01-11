solver: solver.c types.h tables.h
	clang solver.c -Wall -Wextra -Werror -pedantic -std=gnu18 -ggdb3 -Ofast -march=native -mtune=native -o solver -pthread

tables.h: gen_tables.py
	python3 gen_tables.py

.PHONY: run
run: solver
	time ./solver
