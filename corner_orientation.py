#!/usr/bin/env python3

import numpy as np

XYZ = [
    np.array(r, dtype=int)
    for r in (
        ((1, 0, 0), (0, 0, 1), (0, -1, 0)),  # X (F)
        ((0, 0, -1), (0, 1, 0), (1, 0, 0)),  # Y (R)
        ((0, 1, 0), (-1, 0, 0), (0, 0, 1)),  # Z (U)
    )
]


def to_tuple(arr):
    if type(arr) == np.int64:
        return arr
    return tuple(to_tuple(a) for a in arr)


def from_tuple(arr):
    return np.array(arr, dtype=int)


def turns(pos):
    pos = from_tuple(pos)
    for r in XYZ:
        for _ in range(3):
            pos = np.matmul(r, pos)
            yield to_tuple(pos)
        pos = np.matmul(r, pos)


def dfs(pos):
    seen = set()

    def aux(pos):
        if pos in seen:
            return
        seen.add(pos)
        for turn in turns(pos):
            aux(turn)

    aux(pos)
    return sorted(seen)


ROTS = dfs(to_tuple(np.identity(3, dtype=int)))
FACES = dfs((1, 0, 0))
EDGES = dfs((1, 1, 0))
CORNERS = dfs((1, 1, 1))

for r in ROTS:
    print("{ %s }," % ", ".join(str(ROTS.index(pos)) for pos in turns(list(r))))
