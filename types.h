#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t Rotation;
typedef uint8_t Piece;
typedef uint8_t Face;
typedef uint8_t Edge;
typedef uint8_t Corner;
typedef uint8_t Side;
typedef uint8_t Turn;

typedef struct {
  Side side;
  Turn turn;
} Move;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Color;

typedef struct __attribute__((packed)) {
  Rotation edge00 : 5;
  Rotation edge01 : 5;
  Rotation edge02 : 5;
  Rotation edge03 : 5;
  Rotation edge04 : 5;
  Rotation edge05 : 5;
  Rotation edge06 : 5;
  Rotation edge07 : 5;
  Rotation edge08 : 5;
  Rotation edge09 : 5;
  Rotation edge10 : 5;
  Rotation edge11 : 5;

  Rotation corner0 : 5;
  Rotation corner1 : 5;
  Rotation corner2 : 5;
  Rotation corner3 : 5;
  Rotation corner4 : 5;
  Rotation corner5 : 5;
  Rotation corner6 : 5;
  Rotation corner7 : 5;

  int unused : 4;
} State;

typedef struct __attribute__((aligned(16))) {
  State state;
  uint8_t depth;
  Move prev_move;
} TableEntry;

_Static_assert(sizeof(TableEntry) == 16, "");

static const char side_to_char[] = "FRUDLB";
static const char turn_to_char[] = " 2'";

static const Color colors[6] = {
    {0, 255, 0},   {255, 0, 0},   {255, 255, 255},
    {255, 255, 0}, {255, 128, 0}, {0, 0, 255},
};
