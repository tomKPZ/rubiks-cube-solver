#!/usr/bin/env python3

from collections import OrderedDict

import numpy as np


def rotate(v):
    c, s = 0, -1
    x, y, z = v
    return np.array(
        (
            (c + x * x * (1 - c), x * y * (1 - c) - z * s, x * z * (1 - c) + y * s),
            (y * x * (1 - c) + z * s, c + y * y * (1 - c), y * z * (1 - c) - x * s),
            (z * x * (1 - c) - y * s, z * y * (1 - c) + x * s, c + z * z * (1 - c)),
        ),
        dtype=int,
    )


ROT_MATS = [
    rotate((1, 0, 0)),
    rotate((0, 1, 0)),
    rotate((0, 0, 1)),
]


def to_tuple(arr):
    if type(arr) == np.int64:
        return arr
    return tuple(to_tuple(a) for a in arr)


def from_tuple(arr):
    return np.array(arr, dtype=int)


def turns(pos):
    pos = from_tuple(pos)
    for r in ROT_MATS:
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
    seen_l = sorted(seen)
    return OrderedDict(zip(seen_l, range(len(seen_l))))


ROTS = dfs(to_tuple(np.identity(3, dtype=int)))
FACES = dfs((1, 0, 0))
EDGES = dfs((1, 1, 0))
CORNERS = dfs((1, 1, 1))

print('#include "types.h"')
print("static const RotationState rotate[][ROTATE_END] = {")
for r in ROTS:
    print("{ %s }," % ", ".join(str(ROTS[pos]) for pos in turns(r)))
print("};")
