#!/usr/bin/env python3


import numpy as np


def rotCW(x, y, z):
    return np.array(
        (
            (x * x, x * y + z, x * z - y),
            (x * y - z, y * y, y * z + x),
            (x * z + y, y * z - x, z * z),
        ),
        dtype=int,
    )


ROT_MATS = [
    rotCW(1, 0, 0),
    rotCW(0, 1, 0),
    rotCW(0, 0, 1),
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
    return dict(zip(seen_l, range(len(seen_l))))


ROTS = dfs(to_tuple(np.identity(3, dtype=int)))
FACES = dfs((1, 0, 0))
EDGES = dfs((1, 1, 0))
CORNERS = dfs((1, 1, 1))
FACE_NAMES = dict(zip("BLDURF", FACES))


def pieces_on_face(pieces, face):
    return [edge for edge in pieces if all(f in (0, e) for (f, e) in zip(face, edge))]


def orbit(pieces, face):
    piece = pieces_on_face(pieces, face)[0]
    M = rotCW(*face)
    return [
        pieces[to_tuple(np.matmul(np.linalg.matrix_power(M, p), piece))]
        for p in range(4)
    ]


with open("tables.c", "w") as f:
    print('#include "types.h"', file=f)
    print("static const RotationState rotate[][ROTATE_END] = {", file=f)
    for r in ROTS:
        print("{ %s }," % ", ".join(str(ROTS[pos]) for pos in turns(r)), file=f)
    print("};", file=f)
    print("static const Edge edge_orbits[SIDE_END][4] = {", file=f)
    for face in FACES:
        edges = orbit(EDGES, face)
        print("{ %s }," % ", ".join(str(edge) for edge in edges), file=f)
    print("};", file=f)
    print("static const Corner corner_orbits[SIDE_END][4] = {", file=f)
    for face in FACES:
        corners = orbit(CORNERS, face)
        print("{ %s }," % ", ".join(str(corner) for corner in corners), file=f)
    print("};", file=f)
