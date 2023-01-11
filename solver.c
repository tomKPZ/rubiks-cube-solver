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

#include "tables.c"

static const int kNumThreads = 32;
static const int kScrambleDepth = 14;

const size_t kMemGB = 2;
const size_t kMemB = kMemGB * 1024 * 1024 * 1024;
const size_t kTableSize = kMemB / sizeof(State);

static const State kSolved = {
    .edge1 = 23,
    .edge2 = 23,
    .edge3 = 23,
    .edge4 = 23,
    .edge5 = 23,
    .edge6 = 23,
    .edge7 = 23,
    .edge8 = 23,
    .edge9 = 23,
    .edge10 = 23,
    .edge11 = 23,
    .edge12 = 23,

    .corner1 = 23,
    .corner2 = 23,
    .corner3 = 23,
    .corner4 = 23,
    .corner5 = 23,
    .corner6 = 23,
    .corner7 = 23,
    .corner8 = 23,

    .unused = 0,

    .depth = 1,
    .prev_move = {.side = 0, .turn = 0},
};

static size_t hash_state(const State *state) {
  // djb2
  size_t hash = 5381;
  for (size_t i = 0; i < KEY_LEN; i++)
    hash = hash * 33 + ((const char *)state)[i];
  return hash;
}

static int cmp_state(const State *state1, const State *state2) {
  return memcmp(state1, state2, KEY_LEN);
}

static inline void unpack(State state, RotationState edges[12],
                          RotationState corners[8]) {
  edges[0] = state.edge1;
  edges[1] = state.edge2;
  edges[2] = state.edge3;
  edges[3] = state.edge4;
  edges[4] = state.edge5;
  edges[5] = state.edge6;
  edges[6] = state.edge7;
  edges[7] = state.edge8;
  edges[8] = state.edge9;
  edges[9] = state.edge10;
  edges[10] = state.edge11;
  edges[11] = state.edge12;
  corners[0] = state.corner1;
  corners[1] = state.corner2;
  corners[2] = state.corner3;
  corners[3] = state.corner4;
  corners[4] = state.corner5;
  corners[5] = state.corner6;
  corners[6] = state.corner7;
  corners[7] = state.corner8;
}

static inline State pack(State state, const RotationState edges[12],
                         const RotationState corners[8]) {
  state.edge1 = edges[0];
  state.edge2 = edges[1];
  state.edge3 = edges[2];
  state.edge4 = edges[3];
  state.edge5 = edges[4];
  state.edge6 = edges[5];
  state.edge7 = edges[6];
  state.edge8 = edges[7];
  state.edge9 = edges[8];
  state.edge10 = edges[9];
  state.edge11 = edges[10];
  state.edge12 = edges[11];
  state.corner1 = corners[0];
  state.corner2 = corners[1];
  state.corner3 = corners[2];
  state.corner4 = corners[3];
  state.corner5 = corners[4];
  state.corner6 = corners[5];
  state.corner7 = corners[6];
  state.corner8 = corners[7];
  return state;
}

static State make_move(State state, Move move) {
  Rotation rotation = move_to_rotation(move);
  const Edge *edge_orbit = edge_orbits[move.side];
  const Corner *corner_orbit = corner_orbits[move.side];
  RotationState edges[12];
  RotationState corners[8];
  unpack(state, edges, corners);

  uint8_t offset = 3 - move.turn;

  RotationState tmp_edges[4];
  for (size_t i = 0; i < 4; i++)
    tmp_edges[i] = rotate[edges[edge_orbit[i]]][rotation];
  for (size_t i = 0; i < 4; i++)
    edges[edge_orbit[i]] = tmp_edges[(i + offset) % 4];

  RotationState tmp_corners[4];
  for (size_t i = 0; i < 4; i++)
    tmp_corners[i] = rotate[corners[corner_orbit[i]]][rotation];
  for (size_t i = 0; i < 4; i++)
    corners[corner_orbit[i]] = tmp_corners[(i + offset) % 4];

  state = pack(state, edges, corners);
  state.prev_move = move;
  state.depth++;
  return state;
}

static State delta(State a, State b) {
  RotationState rea[12], reb[12], rec[12];
  RotationState rca[8], rcb[8], rcc[8];
  unpack(a, rea, rca);
  unpack(b, reb, rcb);

  Edge ea[12], eb[12];
  for (Edge i = 0; i < 12; i++) {
    ea[i] = rotation_to_edge[rea[i]][i];
    eb[i] = rotation_to_edge[reb[i]][i];
  }
  Edge ie[12];
  for (Edge i = 0; i < 12; i++)
    ie[eb[i]] = i;
  for (Edge i = 0; i < 12; i++)
    rec[ie[ea[i]]] = rotation_delta[rea[i]][reb[ie[ea[i]]]];

  Corner ca[8], cb[8];
  for (Corner i = 0; i < 8; i++) {
    ca[i] = rotation_to_corner[rca[i]][i];
    cb[i] = rotation_to_corner[rcb[i]][i];
  }
  Corner ic[8];
  for (Corner i = 0; i < 8; i++)
    ic[cb[i]] = i;
  for (Corner i = 0; i < 8; i++)
    rcc[ic[ca[i]]] = rotation_delta[rca[i]][rcb[ic[ca[i]]]];

  State state = {
      .unused = 0,
      .prev_move = {0},
      .depth = 0,
  };
  return pack(state, rec, rcc);
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

static void output_move(Move move) {
  printf("%c%c ", side_to_char(move.side), turn_to_char(move.turn));
}

static void output_moves(State *table, const State *state) {
  while (state->depth > 1) {
    Move reversed = {state->prev_move.side, 2 - state->prev_move.turn};
    output_move(reversed);
    State moved = make_move(*state, reversed);
    state = get(table, &moved);
  }
}

typedef struct {
  State *table;
  const State *scrambled;
  size_t depth;
  size_t range_begin;
  size_t range_end;
  pthread_barrier_t *barrier;
} Task;

static pthread_mutex_t output_lock = PTHREAD_MUTEX_INITIALIZER;

static void *task(void *args) {
  Task *task = (Task *)args;
  State *table = task->table;
  size_t depth = task->depth;
  size_t range_begin = task->range_begin;
  size_t range_end = task->range_end;
  State scrambled = *task->scrambled;

  for (size_t i = range_begin; i < range_end; i++) {
    if (table[i].depth == depth) {
      Side prev_side = table[i].prev_move.side;
      for (Side side = 0; side <= SIDE_END; side++) {
        if (depth == 1 || prev_side != side) {
          for (Turn turn = 0; turn <= TURN_END; turn++) {
            Move move = {.side = side, .turn = turn};
            State next = make_move(table[i], move);
            State *pos = get(table, &next);
            if (cmp_state(pos, &next))
              insert(table, pos, &next);
          }
        }
      }
    }
  }
  pthread_barrier_wait(task->barrier);
  for (size_t i = range_begin; i < range_end; i++) {
    if (table[i].depth == depth + 1) {
      State diff = delta(table[i], scrambled);
      State *match = get(table, &diff);
      if (cmp_state(&diff, match) == 0) {
        pthread_mutex_lock(&output_lock);
        output_moves(table, get(table, match));
        output_moves(table, get(table, &table[i]));
        printf("\n");
        exit(0);
      }
    }
  }
  return NULL;
}

State scramble(const State *state) {
  State scrambled = *state;
  srand(time(NULL));
  Side prev = SIDE_END + 1;
  for (int i = 0; i < kScrambleDepth; i++) {
    Side side;
    do {
      side = rand() % (SIDE_END + 1);
    } while (side == prev);
    Turn turn = rand() % (TURN_END + 1);
    Move move = {.side = side, .turn = turn};

    prev = side;
    scrambled = make_move(scrambled, move);
    output_move(move);
  }
  printf("\n");
  scrambled.depth = 1;
  scrambled.prev_move.side = 0;
  scrambled.prev_move.turn = 0;
  return scrambled;
}

int main() {
  State *table = mmap(NULL, kMemB, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE | MAP_HUGETLB |
                          MAP_HUGE_1GB,
                      -1, 0);
  if (table == MAP_FAILED) {
    fprintf(stderr, "mmap() failed.  Falling back to calloc().\n");
    table = calloc(kTableSize, sizeof(State));
    if (table == NULL) {
      fprintf(stderr, "calloc() failed.\n");
      return 1;
    }
  }

  const State scrambled = scramble(&kSolved);

  insert(table, get(table, &kSolved), &kSolved);

  for (size_t depth = 1; true; depth++) {
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, kNumThreads);

    Task args[kNumThreads];
    pthread_t tids[kNumThreads];
    for (int i = 0; i < kNumThreads; i++) {
      args[i].table = table;
      args[i].scrambled = &scrambled;
      args[i].depth = depth;
      args[i].range_begin = i * kTableSize / kNumThreads;
      args[i].range_end = (i + 1) * kTableSize / kNumThreads;
      args[i].barrier = &barrier;
      pthread_create(&tids[i], NULL, task, &args[i]);
    }
    for (int i = 0; i < kNumThreads; i++)
      pthread_join(tids[i], NULL);
  }
}
