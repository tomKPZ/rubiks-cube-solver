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


I3 = np.identity(3, dtype=int)
ROTS = dfs(to_tuple(I3))
FACES = dfs((1, 0, 0))
EDGES = dfs((1, 1, 0))
CORNERS = dfs((1, 1, 1))
FACE_NAMES = dict(zip("BLDURF", FACES))
INV = dict(
    (rot, [r for r in ROTS if np.array_equal(np.matmul(r, rot), I3)][0]) for rot in ROTS
)


def pieces_on_face(pieces, face):
    return [edge for edge in pieces if all(f in (0, e) for (f, e) in zip(face, edge))]


def orbit(pieces, face):
    piece = pieces_on_face(pieces, face)[0]
    M = rotCW(*face)
    return [
        pieces[to_tuple(np.matmul(np.linalg.matrix_power(M, p), piece))]
        for p in range(4)
    ]


def rotation_to_id(pieces, rot, piece):
    return pieces[to_tuple(np.matmul(INV[rot], piece))]


def rotation_delta(r1, r2):
    return ROTS[to_tuple(np.matmul(r2, INV[r1]))]


def moves(rot, face):
    return [
        ROTS[to_tuple(np.matmul(np.linalg.matrix_power(rotCW(*face), p), rot))]
        for p in range(1, 4)
    ]


def output_table(sig, arr, f):
    def aux(arr) -> str:
        if type(arr) == list:
            return "{ %s }" % ", ".join(aux(a) for a in arr)
        return str(arr)

    print(("static const %s = %s;") % (sig, aux(arr)), file=f)


def output(f):
    print("#pragma once", file=f)
    print('#include "types.h"', file=f)
    rotate = [[moves(r, f) for f in FACES] for r in ROTS]
    edge_orbits = [orbit(EDGES, face) for face in FACES]
    corner_orbits = [orbit(CORNERS, face) for face in FACES]
    r2c = [[rotation_to_id(CORNERS, r, c) for c in CORNERS] for r in ROTS]
    r2e = [[rotation_to_id(EDGES, r, e) for e in EDGES] for r in ROTS]
    rd = [[rotation_delta(r1, r2) for r2 in ROTS] for r1 in ROTS]
    output_table("RotationState rotate[24][6][3]", rotate, f)
    output_table("Edge edge_orbits[6][4]", edge_orbits, f)
    output_table("Corner corner_orbits[6][4]", corner_orbits, f)
    output_table("Corner rotation_to_corner[24][8]", r2c, f)
    output_table("Corner rotation_to_edge[24][12]", r2e, f)
    output_table("RotationState rotation_delta[24][24]", rd, f)


def main():
    with open("tables.h", "w") as f:
        output(f)


if __name__ == "__main__":
    main()
