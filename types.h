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

typedef struct __attribute__((packed)) {
  RotationState edge00 : 5;
  RotationState edge01 : 5;
  RotationState edge02 : 5;
  RotationState edge03 : 5;
  RotationState edge04 : 5;
  RotationState edge05 : 5;
  RotationState edge06 : 5;
  RotationState edge07 : 5;
  RotationState edge08 : 5;
  RotationState edge09 : 5;
  RotationState edge10 : 5;
  RotationState edge11 : 5;

  RotationState corner0 : 5;
  RotationState corner1 : 5;
  RotationState corner2 : 5;
  RotationState corner3 : 5;
  RotationState corner4 : 5;
  RotationState corner5 : 5;
  RotationState corner6 : 5;
  RotationState corner7 : 5;

  int unused : 4;
} State;

typedef struct __attribute__((aligned(16))) {
  State key;
  uint8_t depth;
  Move prev_move;
} TableEntry;

_Static_assert(sizeof(TableEntry) == 16, "");

const char side_to_char[] = "FRUDLB";
const char turn_to_char[] = " 2'";
