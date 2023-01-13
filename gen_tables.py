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


def pow(m, p):
    return to_tuple(np.linalg.matrix_power(m, p))


def mul(a, b):
    return to_tuple(np.matmul(a, b))


def from_tuple(arr):
    return np.array(arr, dtype=int)


def turns(pos):
    for r in ROT_MATS:
        for _ in range(3):
            pos = mul(r, pos)
            yield pos
        pos = mul(r, pos)


def dfs(pos):
    seen = set()

    def aux(pos):
        if pos in seen:
            return
        seen.add(pos)
        for turn in turns(pos):
            aux(turn)

    aux(pos)
    seen_l = sorted(seen, reverse=True)
    return dict(zip(seen_l, range(len(seen_l))))


I3 = np.identity(3, dtype=int)
ROTS = dfs(to_tuple(I3))
FACES = dfs((1, 0, 0))
EDGES = dfs((1, 1, 0))
CORNERS = dfs((1, 1, 1))
PIECES = dict(zip(list(FACES) + list(EDGES) + list(CORNERS), range(26)))
FACE_NAMES = dict(zip("FRUDLB", FACES))
INV = dict(
    (rot, [r for r in ROTS if np.array_equal(np.matmul(r, rot), I3)][0]) for rot in ROTS
)


def pieces_on_face(pieces, face):
    return [edge for edge in pieces if all(f in (0, e) for (f, e) in zip(face, edge))]


def orbit(pieces, face):
    piece = pieces_on_face(pieces, face)[0]
    M = rotCW(*face)
    return [pieces[mul(pow(M, p), piece)] for p in range(4)]


def rotation_to_id(pieces, rot, piece):
    return pieces[mul(INV[rot], piece)]


def rotation_delta(r1, r2):
    return ROTS[mul(r2, INV[r1])]


def moves(rot, face):
    return [ROTS[mul(pow(rotCW(*face), p), rot)] for p in range(1, 4)]


def output_table(sig, arr, f):
    def aux(arr) -> str:
        if type(arr) == list:
            return "{ %s }" % ", ".join(aux(a) for a in arr)
        return str(arr)

    print(("static const %s = %s;") % (sig, aux(arr)), file=f)


NET = (
    " U  ",
    "LFRB",
    " D  ",
)

COLORS = {}
FACE_PIECES = {}


def output_face(r, c):
    name = NET[r][c]
    if name == " ":
        return len(FACES)
    F, L, D = (FACE_NAMES[f] for f in "FLD")
    M = mul(
        pow(rotCW(*L), (r - 1) % 4),
        pow(rotCW(*D), (c - 1) % 4),
    )
    face = FACE_NAMES[name]
    id = FACES[face]
    COLORS[id] = [FACES[mul(mul(INV[rot], M), F)] for rot in ROTS]
    pieces = sorted(
        pieces_on_face(PIECES, face),
        key=lambda p: mul(INV[M], p),
    )
    pieces = [PIECES[piece] for piece in pieces]
    pieces = np.flip(np.reshape(pieces, (3, 3)).transpose(), axis=0)
    FACE_PIECES[id] = np.ndarray.tolist(pieces)
    return FACES[face]


def output(f):
    print("#pragma once", file=f)
    print('#include "types.h"', file=f)
    rotate = [[moves(r, f) for f in FACES] for r in ROTS]
    edge_orbits = [orbit(EDGES, face) for face in FACES]
    corner_orbits = [orbit(CORNERS, face) for face in FACES]
    r2e = [[rotation_to_id(EDGES, r, e) for e in EDGES] for r in ROTS]
    r2c = [[rotation_to_id(CORNERS, r, c) for c in CORNERS] for r in ROTS]
    rd = [[rotation_delta(r1, r2) for r2 in ROTS] for r1 in ROTS]
    net = [[output_face(r, c) for c in range(4)] for r in range(3)]
    r2f = [COLORS[i] for i in range(6)]
    face_pieces = [FACE_PIECES[i] for i in range(6)]
    output_table("Rotation rotate[24][6][3]", rotate, f)
    output_table("Edge edge_orbits[6][4]", edge_orbits, f)
    output_table("Corner corner_orbits[6][4]", corner_orbits, f)
    output_table("Edge rotation_to_edge[24][12]", r2e, f)
    output_table("Corner rotation_to_corner[24][8]", r2c, f)
    output_table("Rotation rotation_delta[24][24]", rd, f)
    output_table("Face net[3][4]", net, f)
    output_table("Face rotation_to_face[6][26]", r2f, f)
    output_table("Rotation face_pieces[6][3][3]", face_pieces, f)


def main():
    with open("tables.h", "w") as f:
        output(f)


if __name__ == "__main__":
    main()
