solver: solver.c types.h tables.h
	gcc solver.c -Wall -Wextra -Werror -std=gnu18 -ggdb3 -O3 -march=native -mtune=native -o solver -pthread -latomic -Wno-packed-bitfield-compat

tables.h: gen_tables.py
	python3 gen_tables.py

.PHONY: run
run: solver
	time ./solver
