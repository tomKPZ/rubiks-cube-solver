#include <linux/mman.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>

#include "types.h"

static const int kNumThreads = 32;
static const int kScrambleDepth = 13;

const size_t kMemGB = 1;
const size_t kTableSize = kMemGB * 1024 * 1024 * 1024 / 2 / 16;

static const State kSolved = {
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

static const RotationState rotate[][ROTATE_END] = {
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

static size_t hash_state(const State *state) {
  // djb2
  size_t hash = 5381;
  for (size_t i = 0; i < 14; i++)
    hash = hash * 33 + ((const char *)state)[i];
  return hash;
}

static int cmp_state(const State *state1, const State *state2) {
  return memcmp(state1, state2, 14);
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

static State make_move(State state, Move move) {
  Rotation rotation = move_to_rotation(move);
  switch (move) {
  case MOVE_R1:
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
  case MOVE_L1:
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
  case MOVE_F1:
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
  case MOVE_B1:
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
  case MOVE_U1:
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
  case MOVE_D1:
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

static State *get(State *table, const State *state) {
  for (size_t bucket = hash_state(state) % kTableSize; true;
       bucket = (bucket + 1) % kTableSize) {
    if (!table[bucket].depth || cmp_state(state, &table[bucket]) == 0)
      return &table[bucket];
  }
}

static void insert(State *table_state, State *pos, const State *state) {
  __int128_t *table = (__int128_t *)table_state;
  __int128_t expected = 0;
  __int128_t desired = *((__int128 *)state);

  for (size_t bucket = pos - table_state;
       !__atomic_compare_exchange_n(&table[bucket], &expected, desired, false,
                                    __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
       bucket = (bucket + 1) % kTableSize) {
  }
}

static void output_moves(State *table, const State *state, bool reverse) {
  if (state->depth == 1)
    return;
  Move reversed = reverse_move(state->prev_move);
  State moved = make_move(*state, reversed);
  if (reverse) {
    printf("%s ", move_to_string(reversed));
    output_moves(table, get(table, &moved), reverse);
  } else {
    output_moves(table, get(table, &moved), reverse);
    printf("%s ", move_to_string(state->prev_move));
  }
}

typedef struct {
  State **tables;
  size_t depth;
  int table_i;
  size_t range_begin;
  size_t range_end;
} TaskArgs;

static pthread_mutex_t output_lock = PTHREAD_MUTEX_INITIALIZER;

static void *task(void *args) {
  State **tables = ((TaskArgs *)args)->tables;
  size_t depth = ((TaskArgs *)args)->depth;
  int table_i = ((TaskArgs *)args)->table_i;
  size_t range_begin = ((TaskArgs *)args)->range_begin;
  size_t range_end = ((TaskArgs *)args)->range_end;

  State *table = tables[table_i];
  for (size_t i = range_begin; i < range_end; i++) {
    if (table[i].depth == depth) {
      Side prev_side = move_to_side(table[i].prev_move);
      for (Move move = 0; move <= MOVE_END; move++) {
        if (depth > 1 && prev_side == move_to_side(move))
          continue;
        State next = make_move(table[i], move);
        State *pos = get(table, &next);
        if (pos->depth)
          continue;
        insert(table, pos, &next);
        State *match = get(tables[!table_i], &next);
        if (match->depth) {
          pthread_mutex_lock(&output_lock);
          output_moves(tables[1], get(tables[1], match), false);
          output_moves(tables[0], get(tables[0], match), true);
          printf("\n");
          exit(0);
        }
      }
    }
  }
  return NULL;
}

State scramble(const State *state) {
  State scrambled = *state;
  srand(time(NULL));
  for (int i = 0; i < kScrambleDepth; i++) {
    Move move = rand() % (MOVE_END + 1);
    scrambled = make_move(scrambled, move);
    printf("%s ", move_to_string(move));
  }
  printf("\n");
  scrambled.depth = 1;
  scrambled.prev_move = 0;
  return scrambled;
}

int main() {
  State *mem = mmap(
      NULL, kTableSize * sizeof(State) * 2, PROT_READ | PROT_WRITE,
      MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE | MAP_HUGETLB | MAP_HUGE_1GB,
      -1, 0);
  if (mem == MAP_FAILED)
    return 1;

  State scrambled = scramble(&kSolved);

  State *tables[2] = {mem, mem + kTableSize};
  insert(tables[0], get(tables[0], &kSolved), &kSolved);
  insert(tables[1], get(tables[1], &scrambled), &scrambled);

  for (size_t depth = 1; true; depth++) {
    for (int table_i = 0; table_i < 2; table_i++) {
      TaskArgs args[kNumThreads];
      pthread_t tids[kNumThreads];
      for (int i = 0; i < kNumThreads; i++) {
        args[i].tables = tables;
        args[i].depth = depth;
        args[i].table_i = table_i;
        args[i].range_begin = i * kTableSize / kNumThreads;
        args[i].range_end = (i + 1) * kTableSize / kNumThreads;
        pthread_create(&tids[i], NULL, task, &args[i]);
      }
      for (int i = 0; i < kNumThreads; i++) {
        pthread_join(tids[i], NULL);
      }
    }
  }
}
