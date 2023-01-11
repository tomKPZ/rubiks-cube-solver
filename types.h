#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t RotationState;
typedef uint8_t Edge;
typedef uint8_t Corner;
typedef uint8_t Side;
typedef uint8_t Turn;

typedef struct {
  Side side;
  Turn turn;
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

const char side_to_char[] = "BLDURF";
const char turn_to_char[] = " 2'";
