#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t RotationState;
typedef uint8_t Edge;
typedef uint8_t Corner;

typedef enum {
  SIDE_B = 0,
  SIDE_L,
  SIDE_D,
  SIDE_U,
  SIDE_R,
  SIDE_F,
  SIDE_END = SIDE_F,
} Side;

typedef enum {
  CW,
  HALF,
  CCW,
  TURN_END = CCW,
} Turn;

typedef struct __attribute__((packed)) __attribute__((aligned(2))) {
  Side side : 8;
  Turn turn : 8;
} Move;

typedef struct __attribute__((packed)) __attribute__((aligned(16))) {
  RotationState edge1 : 5;
  RotationState edge2 : 5;
  RotationState edge3 : 5;
  RotationState edge4 : 5;
  RotationState edge5 : 5;
  RotationState edge6 : 5;
  RotationState edge7 : 5;
  RotationState edge8 : 5;
  RotationState edge9 : 5;
  RotationState edge10 : 5;
  RotationState edge11 : 5;
  RotationState edge12 : 5;

  RotationState corner1 : 5;
  RotationState corner2 : 5;
  RotationState corner3 : 5;
  RotationState corner4 : 5;
  RotationState corner5 : 5;
  RotationState corner6 : 5;
  RotationState corner7 : 5;
  RotationState corner8 : 5;

  int unused : 4;

  uint8_t depth;
  Move prev_move;
} State;

#define KEY_LEN 13

_Static_assert(sizeof(State) == 16, "");
_Static_assert(offsetof(State, depth) == KEY_LEN, "");

static inline char side_to_char(Side side) {
  switch (side) {
  case SIDE_B:
    return 'B';
  case SIDE_L:
    return 'L';
  case SIDE_D:
    return 'D';
  case SIDE_U:
    return 'U';
  case SIDE_R:
    return 'R';
  case SIDE_F:
    return 'F';
  }
}

static inline char turn_to_char(Turn turn) {
  switch (turn) {
  case CW:
    return ' ';
  case HALF:
    return '2';
  case CCW:
    return '\'';
  }
}
