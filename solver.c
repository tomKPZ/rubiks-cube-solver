#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef unsigned char RotationState;

typedef enum {
  ROTATE_R = 0,
  ROTATE_RP,
  ROTATE_R2,
  ROTATE_U,
  ROTATE_UP,
  ROTATE_U2,
  ROTATE_F,
  ROTATE_FP,
  ROTATE_F2,
  ROTATE_END,
} Rotation;

typedef enum {
  MOVE_R = 0,
  MOVE_RP,
  MOVE_R2,
  MOVE_L,
  MOVE_LP,
  MOVE_L2,
  MOVE_U,
  MOVE_UP,
  MOVE_U2,
  MOVE_D,
  MOVE_DP,
  MOVE_D2,
  MOVE_F,
  MOVE_FP,
  MOVE_F2,
  MOVE_B,
  MOVE_BP,
  MOVE_B2,
  MOVE_END = MOVE_B2,
} Move;

typedef struct __attribute__((packed)) __attribute__((aligned(16))) {
  RotationState corner1 : 5;
  RotationState corner2 : 5;
  RotationState corner3 : 5;
  RotationState corner4 : 5;
  RotationState corner5 : 5;
  RotationState corner6 : 5;
  RotationState corner7 : 5;
  RotationState corner8 : 5;

  unsigned int edge1 : 4;
  unsigned int edge2 : 4;
  unsigned int edge3 : 4;
  unsigned int edge4 : 4;
  unsigned int edge5 : 4;
  unsigned int edge6 : 4;
  unsigned int edge7 : 4;
  unsigned int edge8 : 4;
  unsigned int edge9 : 4;
  unsigned int edge10 : 4;
  unsigned int edge11 : 4;
  unsigned int edge12 : 4;
  bool edge1_parity : 1;
  bool edge2_parity : 1;
  bool edge3_parity : 1;
  bool edge4_parity : 1;
  bool edge5_parity : 1;
  bool edge6_parity : 1;
  bool edge7_parity : 1;
  bool edge8_parity : 1;
  bool edge9_parity : 1;
  bool edge10_parity : 1;
  bool edge11_parity : 1;
  bool edge12_parity : 1;
  int unused : 12;

  unsigned char depth;
  Move prev_move : 8;
} State;

_Static_assert(sizeof(State) == 16, "");
_Static_assert(offsetof(State, depth) == 14, "");

const RotationState rotate[][ROTATE_END] = {
    {9, 12, 5, 2, 3, 1, 22, 18, 4},     {13, 8, 4, 3, 2, 0, 19, 23, 5},
    {17, 20, 7, 1, 0, 3, 10, 14, 6},    {21, 16, 6, 0, 1, 2, 15, 11, 7},
    {8, 13, 1, 7, 6, 5, 18, 22, 0},     {12, 9, 0, 6, 7, 4, 23, 19, 1},
    {16, 21, 3, 4, 5, 7, 14, 10, 2},    {20, 17, 2, 5, 4, 6, 11, 15, 3},
    {1, 4, 13, 11, 10, 9, 16, 20, 12},  {5, 0, 12, 10, 11, 8, 21, 17, 13},
    {19, 22, 15, 8, 9, 11, 6, 2, 14},   {23, 18, 14, 9, 8, 10, 3, 7, 15},
    {0, 5, 9, 14, 15, 13, 20, 16, 8},   {4, 1, 8, 15, 14, 12, 17, 21, 9},
    {18, 23, 11, 13, 12, 15, 2, 6, 10}, {22, 19, 10, 12, 13, 14, 7, 3, 11},
    {3, 6, 21, 18, 19, 17, 12, 8, 20},  {7, 2, 20, 19, 18, 16, 9, 13, 21},
    {11, 14, 23, 17, 16, 19, 0, 4, 22}, {15, 10, 22, 16, 17, 18, 5, 1, 23},
    {2, 7, 17, 23, 22, 21, 8, 12, 16},  {6, 3, 16, 22, 23, 20, 13, 9, 17},
    {10, 15, 19, 20, 21, 23, 4, 0, 18}, {14, 11, 18, 21, 20, 22, 1, 5, 19},
};

size_t hash_state(const State *state) {
  // djb2
  size_t hash = 5381;
  for (size_t i = 0; i < 14; i++)
    hash = hash * 33 + ((const char *)state)[i];
  return hash;
}

int cmp_state(const State *state1, const State *state2) {
  return memcmp(state1, state2, 14);
}

Move reverse_move(Move move) {
  switch (move) {
  case MOVE_R:
    return MOVE_RP;
  case MOVE_RP:
    return MOVE_R;
  case MOVE_R2:
    return MOVE_R2;
  case MOVE_L:
    return MOVE_LP;
  case MOVE_LP:
    return MOVE_L;
  case MOVE_L2:
    return MOVE_L2;
  case MOVE_F:
    return MOVE_FP;
  case MOVE_FP:
    return MOVE_F;
  case MOVE_F2:
    return MOVE_F2;
  case MOVE_B:
    return MOVE_BP;
  case MOVE_BP:
    return MOVE_B;
  case MOVE_B2:
    return MOVE_B2;
  case MOVE_U:
    return MOVE_UP;
  case MOVE_UP:
    return MOVE_U;
  case MOVE_U2:
    return MOVE_U2;
  case MOVE_D:
    return MOVE_DP;
  case MOVE_DP:
    return MOVE_D;
  case MOVE_D2:
    return MOVE_D2;
  }
}

const char *move_to_string(Move move) {
  switch (move) {
  case MOVE_R:
    return "R";
  case MOVE_RP:
    return "R'";
  case MOVE_R2:
    return "R2";
  case MOVE_L:
    return "L";
  case MOVE_LP:
    return "L'";
  case MOVE_L2:
    return "L2";
  case MOVE_F:
    return "F";
  case MOVE_FP:
    return "F'";
  case MOVE_F2:
    return "F2";
  case MOVE_B:
    return "B";
  case MOVE_BP:
    return "B'";
  case MOVE_B2:
    return "B2";
  case MOVE_U:
    return "U";
  case MOVE_UP:
    return "U'";
  case MOVE_U2:
    return "U2";
  case MOVE_D:
    return "D";
  case MOVE_DP:
    return "D'";
  case MOVE_D2:
    return "D2";
  }
}

Rotation move_to_rotation(Move move) {
  switch (move) {
  case MOVE_R:
    return ROTATE_R;
  case MOVE_RP:
    return ROTATE_RP;
  case MOVE_R2:
    return ROTATE_R2;
  case MOVE_L:
    return ROTATE_RP;
  case MOVE_LP:
    return ROTATE_R;
  case MOVE_L2:
    return ROTATE_R2;
  case MOVE_F:
    return ROTATE_F;
  case MOVE_FP:
    return ROTATE_FP;
  case MOVE_F2:
    return ROTATE_F2;
  case MOVE_B:
    return ROTATE_FP;
  case MOVE_BP:
    return ROTATE_F;
  case MOVE_B2:
    return ROTATE_F2;
  case MOVE_U:
    return ROTATE_U;
  case MOVE_UP:
    return ROTATE_UP;
  case MOVE_U2:
    return ROTATE_U2;
  case MOVE_D:
    return ROTATE_UP;
  case MOVE_DP:
    return ROTATE_U;
  case MOVE_D2:
    return ROTATE_U2;
  }
}

#define ROTATE_EDGES(a, b, c, d)                                               \
  {                                                                            \
    unsigned int tmp = state.edge##d;                                          \
    state.edge##d = state.edge##c;                                             \
    state.edge##c = state.edge##b;                                             \
    state.edge##b = state.edge##a;                                             \
    state.edge##a = tmp;                                                       \
  }                                                                            \
  {                                                                            \
    bool tmp = state.edge##d##_parity;                                         \
    state.edge##d##_parity = state.edge##c##_parity;                           \
    state.edge##c##_parity = state.edge##b##_parity;                           \
    state.edge##b##_parity = state.edge##a##_parity;                           \
    state.edge##a##_parity = tmp;                                              \
    state.edge##a##_parity = !state.edge##a##_parity;                          \
    state.edge##b##_parity = !state.edge##b##_parity;                          \
    state.edge##c##_parity = !state.edge##c##_parity;                          \
    state.edge##d##_parity = !state.edge##d##_parity;                          \
  }
#define ROTATE_CORNERS(a, b, c, d)                                             \
  {                                                                            \
    RotationState tmp = state.corner##d;                                       \
    state.corner##d = state.corner##c;                                         \
    state.corner##c = state.corner##b;                                         \
    state.corner##b = state.corner##a;                                         \
    state.corner##a = tmp;                                                     \
    state.corner##a = rotate[state.corner##a][rotation];                       \
    state.corner##b = rotate[state.corner##b][rotation];                       \
    state.corner##c = rotate[state.corner##c][rotation];                       \
    state.corner##d = rotate[state.corner##d][rotation];                       \
  }
#define ROTATEP_EDGES(a, b, c, d) ROTATE_EDGES(d, c, b, a)
#define ROTATEP_CORNERS(a, b, c, d) ROTATE_CORNERS(d, c, b, a)
#define ROTATE2_EDGES(a, b, c, d)                                              \
  {                                                                            \
    unsigned int tmp = state.edge##a;                                          \
    state.edge##a = state.edge##c;                                             \
    state.edge##c = tmp;                                                       \
    tmp = state.edge##b;                                                       \
    state.edge##b = state.edge##d;                                             \
    state.edge##d = tmp;                                                       \
  }                                                                            \
  {                                                                            \
    bool tmp = state.edge##a##_parity;                                         \
    state.edge##a##_parity = state.edge##c##_parity;                           \
    state.edge##c##_parity = tmp;                                              \
    tmp = state.edge##b##_parity;                                              \
    state.edge##b##_parity = state.edge##d##_parity;                           \
    state.edge##d##_parity = tmp;                                              \
  }
#define ROTATE2_CORNERS(a, b, c, d)                                            \
  {                                                                            \
    RotationState tmp = state.corner##a;                                       \
    state.corner##a = state.corner##c;                                         \
    state.corner##c = tmp;                                                     \
    tmp = state.corner##b;                                                     \
    state.corner##b = state.corner##d;                                         \
    state.corner##d = tmp;                                                     \
    state.corner##a = rotate[state.corner##a][rotation];                       \
    state.corner##b = rotate[state.corner##b][rotation];                       \
    state.corner##c = rotate[state.corner##c][rotation];                       \
    state.corner##d = rotate[state.corner##d][rotation];                       \
  }

State make_move(State state, Move move) {
  Rotation rotation = move_to_rotation(move);
  switch (move) {
  case MOVE_R:
    ROTATE_EDGES(3, 6, 11, 8);
    ROTATE_CORNERS(2, 6, 8, 4);
    break;
  case MOVE_RP:
    ROTATEP_EDGES(3, 6, 11, 8);
    ROTATEP_CORNERS(2, 6, 8, 4);
    break;
  case MOVE_R2:
    ROTATE2_EDGES(3, 6, 11, 8);
    ROTATE2_CORNERS(2, 6, 8, 4);
    break;
  case MOVE_L:
    ROTATE_EDGES(2, 7, 10, 5);
    ROTATE_CORNERS(1, 3, 7, 5);
    break;
  case MOVE_LP:
    ROTATEP_EDGES(2, 7, 10, 5);
    ROTATEP_CORNERS(1, 3, 7, 5);
    break;
  case MOVE_L2:
    ROTATE2_EDGES(2, 7, 10, 5);
    ROTATE2_CORNERS(1, 3, 7, 5);
    break;
  case MOVE_F:
    ROTATE_EDGES(4, 8, 12, 7);
    ROTATE_CORNERS(3, 4, 8, 7);
    break;
  case MOVE_FP:
    ROTATEP_EDGES(4, 8, 12, 7);
    ROTATEP_CORNERS(3, 4, 8, 7);
    break;
  case MOVE_F2:
    ROTATE2_EDGES(4, 8, 12, 7);
    ROTATE2_CORNERS(3, 4, 8, 7);
    break;
  case MOVE_B:
    ROTATE_EDGES(1, 5, 9, 6);
    ROTATE_CORNERS(1, 5, 6, 2);
    break;
  case MOVE_BP:
    ROTATEP_EDGES(1, 5, 9, 6);
    ROTATEP_CORNERS(1, 5, 6, 2);
    break;
  case MOVE_B2:
    ROTATE2_EDGES(1, 5, 9, 6);
    ROTATE2_CORNERS(1, 5, 6, 2);
    break;
  case MOVE_U:
    ROTATE_EDGES(1, 3, 4, 2);
    ROTATE_CORNERS(1, 2, 4, 3);
    break;
  case MOVE_UP:
    ROTATEP_EDGES(1, 3, 4, 2);
    ROTATEP_CORNERS(1, 2, 4, 3);
    break;
  case MOVE_U2:
    ROTATE2_EDGES(1, 3, 4, 2);
    ROTATE2_CORNERS(1, 2, 4, 3);
    break;
  case MOVE_D:
    ROTATE_EDGES(11, 9, 10, 12);
    ROTATE_CORNERS(6, 5, 7, 8);
    break;
  case MOVE_DP:
    ROTATEP_EDGES(11, 9, 10, 12);
    ROTATEP_CORNERS(6, 5, 7, 8);
    break;
  case MOVE_D2:
    ROTATE2_EDGES(11, 9, 10, 12);
    ROTATE2_CORNERS(6, 5, 7, 8);
    break;
  }
  state.prev_move = move;
  state.depth++;
  return state;
}

void output_state(const State *state) {
  printf("%d %d %d %d %d %d %d %d\n", state->corner1, state->corner2,
         state->corner3, state->corner4, state->corner5, state->corner6,
         state->corner7, state->corner8);
  printf("%d %d %d %d %d %d %d %d %d %d %d %d\n", state->edge1, state->edge2,
         state->edge3, state->edge4, state->edge5, state->edge6, state->edge7,
         state->edge8, state->edge9, state->edge10, state->edge11,
         state->edge12);
  printf("%d %d %d %d %d %d %d %d %d %d %d %d\n", state->edge1_parity,
         state->edge2_parity, state->edge3_parity, state->edge4_parity,
         state->edge5_parity, state->edge6_parity, state->edge7_parity,
         state->edge8_parity, state->edge9_parity, state->edge10_parity,
         state->edge11_parity, state->edge12_parity);
  printf("%d %d\n", state->depth, state->prev_move);
}

const size_t N = 128 * 1024 * 1024;

State *get(State *table, const State *state) {
  for (size_t bucket = hash_state(state) % N; table[bucket].depth;
       bucket = (bucket + 1) % N) {
    if (cmp_state(state, &table[bucket]) == 0)
      return &table[bucket];
  }
  return NULL;
}

void insert(State *table, const State *state) {
  size_t hash = hash_state(state);
  size_t bucket = hash % N;
  while (table[bucket].depth)
    bucket = (bucket + 1) % N;
  table[bucket] = *state;
}

void output_moves_to(State *table, const State *state) {
  if (state->depth == 1)
    return;
  Move reversed = reverse_move(state->prev_move);
  State moved = make_move(*state, reversed);
  printf("%s ", move_to_string(reversed));
  output_moves_to(table, get(table, &moved));
}

void output_moves_from(State *table, const State *state) {
  if (state->depth == 1)
    return;
  Move reversed = reverse_move(state->prev_move);
  State moved = make_move(*state, reversed);
  output_moves_from(table, get(table, &moved));
  printf("%s ", move_to_string(state->prev_move));
}

int main() {
  const State solved = {
      .corner1 = 0,
      .corner2 = 0,
      .corner3 = 0,
      .corner4 = 0,
      .corner5 = 0,
      .corner6 = 0,
      .corner7 = 0,
      .corner8 = 0,

      .edge1 = 0,
      .edge2 = 1,
      .edge3 = 2,
      .edge4 = 3,
      .edge5 = 4,
      .edge6 = 5,
      .edge7 = 6,
      .edge8 = 7,
      .edge9 = 8,
      .edge10 = 9,
      .edge11 = 10,
      .edge12 = 11,
      .edge1_parity = false,
      .edge2_parity = false,
      .edge3_parity = false,
      .edge4_parity = false,
      .edge5_parity = false,
      .edge6_parity = false,
      .edge7_parity = false,
      .edge8_parity = false,
      .edge9_parity = false,
      .edge10_parity = false,
      .edge11_parity = false,
      .edge12_parity = false,
      .unused = 0,

      .depth = 1,
      .prev_move = 0,
  };

  State scrambled = solved;
  srand(time(NULL));
  for (int i = 0; i < 12; i++) {
    Move move = rand() % (MOVE_END + 1);
    scrambled = make_move(scrambled, move);
    printf("%s ", move_to_string(move));
  }
  printf("\n");
  scrambled.depth = 1;
  scrambled.prev_move = 0;

  State *tables[2] = {calloc(N, sizeof(State)), calloc(N, sizeof(State))};
  insert(tables[0], &solved);
  insert(tables[1], &scrambled);

  for (size_t depth = 1; true; depth++) {
    for (int table_i = 0; table_i < 2; table_i++) {
      State *table = tables[table_i];
      for (size_t i = 0; i < N; i++) {
        if (table[i].depth == depth) {
          for (Move move = 0; move <= MOVE_END; move++) {
            State next = make_move(table[i], move);
            if (get(table, &next))
              continue;
            insert(table, &next);
            State *match = get(tables[!table_i], &next);
            if (match) {
              output_moves_from(tables[1], get(tables[1], match));
              output_moves_to(tables[0], get(tables[0], match));
              printf("\n");
              return 0;
            }
          }
        }
      }
    }
  }
}
