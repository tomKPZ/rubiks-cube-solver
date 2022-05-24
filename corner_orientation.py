#!/usr/bin/env python3

#           0    1    2    3    4    5
#           U    F    L    B    R    D
INITIAL = ["W", "G", "O", "B", "R", "Y"]

R = [0, 3, 5, 1]
U = [1, 2, 3, 4]
F = [0, 4, 5, 2]


def gather(lst, js):
    return [lst[i] for i in js]


def scatter(lst, xs, js):
    for x, i in zip(xs, js):
        lst[i] = x
    return lst


def rot(pos, cycle):
    xs = gather(pos, cycle)
    xs = [xs[-1]] + xs[:-1]
    return scatter(pos[:], xs, cycle)


def rot3(pos, cycle):
    return rot(rot(rot(pos, cycle), cycle), cycle)


def turns(pos):
    for r in (R, U, F):
        yield rot(pos, r)
        yield rot3(pos, r)


orientations = set()
orientations.add("".join(INITIAL))
while len(orientations) != 24:
    to_add = []
    for orientation in orientations:
        to_add += list(turns(list(orientation)))
    for orientation in to_add:
        orientations.add("".join(orientation))

orientations = sorted(list(orientations))
for orientation in orientations:
    print(
        *[orientations.index("".join(pos))
          for pos in turns(list(orientation))],
        sep="\t"
    )
