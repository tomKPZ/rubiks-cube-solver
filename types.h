#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t RotationState;
typedef uint8_t Edge;
typedef uint8_t Corner;

typedef enum {
  CW,
  HALF,
  CCW,
} Turn;

typedef enum {
  SIDE_B = 0,
  SIDE_L,
  SIDE_D,
  SIDE_U,
  SIDE_R,
  SIDE_F,
  SIDE_END,
} Side;

typedef enum {
  ROTATE_F1 = 0,
  ROTATE_F2,
  ROTATE_FP,
  ROTATE_R1,
  ROTATE_R2,
  ROTATE_RP,
  ROTATE_U1,
  ROTATE_U2,
  ROTATE_UP,
  ROTATE_END,
} Rotation;

typedef enum {
  MOVE_R1 = 0,
  MOVE_RP,
  MOVE_R2,
  MOVE_L1,
  MOVE_LP,
  MOVE_L2,
  MOVE_U1,
  MOVE_UP,
  MOVE_U2,
  MOVE_D1,
  MOVE_DP,
  MOVE_D2,
  MOVE_F1,
  MOVE_FP,
  MOVE_F2,
  MOVE_B1,
  MOVE_BP,
  MOVE_B2,
  MOVE_END = MOVE_B2,
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

  int unused : 12;

  unsigned char depth;
  Move prev_move : 8;
} State;

_Static_assert(sizeof(State) == 16, "");
_Static_assert(offsetof(State, depth) == 14, "");

static inline Move reverse_move(Move move) {
  switch (move) {
  case MOVE_R1:
    return MOVE_RP;
  case MOVE_RP:
    return MOVE_R1;
  case MOVE_R2:
    return MOVE_R2;
  case MOVE_L1:
    return MOVE_LP;
  case MOVE_LP:
    return MOVE_L1;
  case MOVE_L2:
    return MOVE_L2;
  case MOVE_F1:
    return MOVE_FP;
  case MOVE_FP:
    return MOVE_F1;
  case MOVE_F2:
    return MOVE_F2;
  case MOVE_B1:
    return MOVE_BP;
  case MOVE_BP:
    return MOVE_B1;
  case MOVE_B2:
    return MOVE_B2;
  case MOVE_U1:
    return MOVE_UP;
  case MOVE_UP:
    return MOVE_U1;
  case MOVE_U2:
    return MOVE_U2;
  case MOVE_D1:
    return MOVE_DP;
  case MOVE_DP:
    return MOVE_D1;
  case MOVE_D2:
    return MOVE_D2;
  }
}

static inline const char *move_to_string(Move move) {
  switch (move) {
  case MOVE_R1:
    return "R";
  case MOVE_RP:
    return "R'";
  case MOVE_R2:
    return "R2";
  case MOVE_L1:
    return "L";
  case MOVE_LP:
    return "L'";
  case MOVE_L2:
    return "L2";
  case MOVE_F1:
    return "F";
  case MOVE_FP:
    return "F'";
  case MOVE_F2:
    return "F2";
  case MOVE_B1:
    return "B";
  case MOVE_BP:
    return "B'";
  case MOVE_B2:
    return "B2";
  case MOVE_U1:
    return "U";
  case MOVE_UP:
    return "U'";
  case MOVE_U2:
    return "U2";
  case MOVE_D1:
    return "D";
  case MOVE_DP:
    return "D'";
  case MOVE_D2:
    return "D2";
  }
}

static inline Rotation move_to_rotation(Move move) {
  switch (move) {
  case MOVE_R1:
    return ROTATE_R1;
  case MOVE_RP:
    return ROTATE_RP;
  case MOVE_R2:
    return ROTATE_R2;
  case MOVE_L1:
    return ROTATE_RP;
  case MOVE_LP:
    return ROTATE_R1;
  case MOVE_L2:
    return ROTATE_R2;
  case MOVE_F1:
    return ROTATE_F1;
  case MOVE_FP:
    return ROTATE_FP;
  case MOVE_F2:
    return ROTATE_F2;
  case MOVE_B1:
    return ROTATE_FP;
  case MOVE_BP:
    return ROTATE_F1;
  case MOVE_B2:
    return ROTATE_F2;
  case MOVE_U1:
    return ROTATE_U1;
  case MOVE_UP:
    return ROTATE_UP;
  case MOVE_U2:
    return ROTATE_U2;
  case MOVE_D1:
    return ROTATE_UP;
  case MOVE_DP:
    return ROTATE_U1;
  case MOVE_D2:
    return ROTATE_U2;
  }
}

static inline Side move_to_side(Move move) {
  switch (move) {
  case MOVE_R1:
  case MOVE_RP:
  case MOVE_R2:
    return SIDE_R;
  case MOVE_L1:
  case MOVE_LP:
  case MOVE_L2:
    return SIDE_L;
  case MOVE_F1:
  case MOVE_FP:
  case MOVE_F2:
    return SIDE_F;
  case MOVE_B1:
  case MOVE_BP:
  case MOVE_B2:
    return SIDE_B;
  case MOVE_U1:
  case MOVE_UP:
  case MOVE_U2:
    return SIDE_U;
  case MOVE_D1:
  case MOVE_DP:
  case MOVE_D2:
    return SIDE_D;
  }
}

static inline Turn move_to_turn(Move move) {
  switch (move) {
  case MOVE_R1:
  case MOVE_L1:
  case MOVE_F1:
  case MOVE_B1:
  case MOVE_U1:
  case MOVE_D1:
    return CW;
  case MOVE_R2:
  case MOVE_L2:
  case MOVE_F2:
  case MOVE_B2:
  case MOVE_U2:
  case MOVE_D2:
    return HALF;
  case MOVE_RP:
  case MOVE_LP:
  case MOVE_FP:
  case MOVE_BP:
  case MOVE_UP:
  case MOVE_DP:
    return CCW;
  }
}
