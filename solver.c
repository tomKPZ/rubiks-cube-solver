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

#include "tables.h"
#include "types.h"

static const int kNumThreads = 32;
static const int kScrambleDepth = 12;
static const int kSearchDepth = 5;

static const size_t kMemGB = 2;
static const size_t kMemB = kMemGB * 1024 * 1024 * 1024;
static const size_t kTableSize = kMemB / sizeof(TableEntry);

static const State kSolved = {0};

static size_t hash_state(State state) {
  // djb2
  size_t hash = 5381;
  for (size_t i = 0; i < sizeof(State); i++)
    hash = hash * 33 + ((const char *)&state)[i];
  return hash;
}

static int cmp_state(State state1, State state2) {
  return memcmp(&state1, &state2, sizeof(State));
}

static inline void unpack(State state, Rotation edges[12],
                          Rotation corners[8]) {
  edges[0] = state.edge00;
  edges[1] = state.edge01;
  edges[2] = state.edge02;
  edges[3] = state.edge03;
  edges[4] = state.edge04;
  edges[5] = state.edge05;
  edges[6] = state.edge06;
  edges[7] = state.edge07;
  edges[8] = state.edge08;
  edges[9] = state.edge09;
  edges[10] = state.edge10;
  edges[11] = state.edge11;
  corners[0] = state.corner0;
  corners[1] = state.corner1;
  corners[2] = state.corner2;
  corners[3] = state.corner3;
  corners[4] = state.corner4;
  corners[5] = state.corner5;
  corners[6] = state.corner6;
  corners[7] = state.corner7;
}

static inline State pack(const Rotation edges[12], const Rotation corners[8]) {
  State state;
  state.edge00 = edges[0];
  state.edge01 = edges[1];
  state.edge02 = edges[2];
  state.edge03 = edges[3];
  state.edge04 = edges[4];
  state.edge05 = edges[5];
  state.edge06 = edges[6];
  state.edge07 = edges[7];
  state.edge08 = edges[8];
  state.edge09 = edges[9];
  state.edge10 = edges[10];
  state.edge11 = edges[11];
  state.corner0 = corners[0];
  state.corner1 = corners[1];
  state.corner2 = corners[2];
  state.corner3 = corners[3];
  state.corner4 = corners[4];
  state.corner5 = corners[5];
  state.corner6 = corners[6];
  state.corner7 = corners[7];
  state.unused = 0;
  return state;
}

static State make_move(State state, Move move) {
  const Edge *edge_orbit = edge_orbits[move.side];
  const Corner *corner_orbit = corner_orbits[move.side];
  Rotation edges[12];
  Rotation corners[8];
  unpack(state, edges, corners);

  uint8_t offset = 3 - move.turn;

  Rotation tmp_edges[4];
  for (size_t i = 0; i < 4; i++)
    tmp_edges[i] = rotate[edges[edge_orbit[i]]][move.side][move.turn];
  for (size_t i = 0; i < 4; i++)
    edges[edge_orbit[i]] = tmp_edges[(i + offset) % 4];

  Rotation tmp_corners[4];
  for (size_t i = 0; i < 4; i++)
    tmp_corners[i] = rotate[corners[corner_orbit[i]]][move.side][move.turn];
  for (size_t i = 0; i < 4; i++)
    corners[corner_orbit[i]] = tmp_corners[(i + offset) % 4];

  return pack(edges, corners);
}

static State delta(State a, State b) {
  Rotation rea[12], reb[12], rec[12];
  Rotation rca[8], rcb[8], rcc[8];
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

  return pack(rec, rcc);
}

static TableEntry *get(TableEntry *table, State state) {
  for (size_t bucket = hash_state(state) % kTableSize; true;
       bucket = (bucket + 1) % kTableSize) {
    if (!table[bucket].depth || cmp_state(state, table[bucket].state) == 0)
      return &table[bucket];
  }
}

static void insert(TableEntry *table_state, TableEntry *pos, TableEntry state) {
  __int128_t *table = (__int128_t *)table_state;
  __int128_t expected = 0;
  __int128_t desired = *((__int128 *)&state);

  for (size_t bucket = pos - table_state;
       !__atomic_compare_exchange_n(&table[bucket], &expected, desired, false,
                                    __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
       bucket = (bucket + 1) % kTableSize) {
  }
}

static Move reverse_move(Move move) {
  Move reversed = {move.side, 2 - move.turn};
  return reversed;
}

static void output_move(Move move) {
  printf("%c%c ", side_to_char[move.side], turn_to_char[move.turn]);
}

static void output_moves(TableEntry *table, TableEntry state) {
  while (state.depth > 1) {
    Move reversed = reverse_move(state.prev_move);
    output_move(reversed);
    State moved = make_move(state.state, reversed);
    state = *get(table, moved);
  }
}

typedef struct {
  TableEntry *table;
  State scrambled;
  size_t depth;
  size_t range_begin;
  size_t range_end;
  pthread_barrier_t *barrier;
  pthread_mutex_t *output_lock;
} Task;

static bool dfs(TableEntry *table, State state, size_t depth, Move prev_move,
                State scrambled, pthread_mutex_t *output_lock) {
  if (depth == 0) {
    State diff = delta(state, scrambled);
    TableEntry *match = get(table, diff);
    if (cmp_state(diff, match->state) == 0) {
      pthread_mutex_lock(output_lock);
      output_moves(table, *get(table, match->state));
      return true;
    }
    return false;
  }
  Side prev_side = prev_move.side;
  for (Side side = 0; side < 6; side++) {
    if (depth == 1 || prev_side != side) {
      for (Turn turn = 0; turn < 3; turn++) {
        Move move = {.side = side, .turn = turn};
        State next = make_move(state, move);
        if (dfs(table, next, depth - 1, move, scrambled, output_lock)) {
          output_move(reverse_move(move));
          return true;
        }
      }
    }
  }
  return false;
}

static void *task(void *args) {
  Task *task = (Task *)args;
  TableEntry *table = task->table;
  size_t depth = task->depth;

  if (depth > kSearchDepth) {
    for (size_t i = task->range_begin; i < task->range_end; i++) {
      if (table[i].depth == kSearchDepth) {
        if (dfs(table, table[i].state, depth - kSearchDepth, table[i].prev_move,
                task->scrambled, task->output_lock)) {
          output_moves(table, *get(table, table[i].state));
          printf("\n");
          exit(0);
        }
      }
    }
    return NULL;
  }

  for (size_t i = task->range_begin; i < task->range_end; i++) {
    if (table[i].depth == depth) {
      Side prev_side = table[i].prev_move.side;
      for (Side side = 0; side < 6; side++) {
        if (depth == 1 || prev_side != side) {
          for (Turn turn = 0; turn < 3; turn++) {
            Move move = {.side = side, .turn = turn};
            State next = make_move(table[i].state, move);
            TableEntry *pos = get(table, next);
            if (cmp_state(pos->state, next)) {
              TableEntry entry = {next, table[i].depth + 1, move};
              insert(table, pos, entry);
            }
          }
        }
      }
    }
  }
  pthread_barrier_wait(task->barrier);
  for (size_t i = task->range_begin; i < task->range_end; i++) {
    if (table[i].depth == depth + 1) {
      State diff = delta(table[i].state, task->scrambled);
      TableEntry *match = get(table, diff);
      if (cmp_state(diff, match->state) == 0) {
        pthread_mutex_lock(task->output_lock);
        output_moves(table, *get(table, match->state));
        output_moves(table, *get(table, table[i].state));
        printf("\n");
        exit(0);
      }
    }
  }
  return NULL;
}

static State scramble(State state) {
  srand(time(NULL));
  Side prev = 6;
  for (int i = 0; i < kScrambleDepth; i++) {
    Side side;
    do {
      side = rand() % 6;
    } while (side == prev);
    Turn turn = rand() % 3;
    Move move = {.side = side, .turn = turn};

    prev = side;
    state = make_move(state, move);
    output_move(move);
  }
  printf("\n");
  return state;
}

int main() {
  TableEntry *table = mmap(NULL, kMemB, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE |
                               MAP_HUGETLB | MAP_HUGE_1GB,
                           -1, 0);
  if (table == MAP_FAILED) {
    fprintf(stderr, "mmap() failed.  Falling back to calloc().\n");
    table = calloc(kTableSize, sizeof(TableEntry));
    if (table == NULL) {
      fprintf(stderr, "calloc() failed.\n");
      return 1;
    }
  }

  const State scrambled = scramble(kSolved);

  TableEntry solved = {kSolved, 1, {0, 0}};
  insert(table, get(table, kSolved), solved);

  for (size_t depth = 1; true; depth++) {
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, kNumThreads);
    pthread_mutex_t output_lock = PTHREAD_MUTEX_INITIALIZER;

    Task args[kNumThreads];
    pthread_t tids[kNumThreads];
    for (int i = 0; i < kNumThreads; i++) {
      args[i].table = table;
      args[i].scrambled = scrambled;
      args[i].depth = depth;
      args[i].range_begin = i * kTableSize / kNumThreads;
      args[i].range_end = (i + 1) * kTableSize / kNumThreads;
      args[i].barrier = &barrier;
      args[i].output_lock = &output_lock;
      pthread_create(&tids[i], NULL, task, &args[i]);
    }
    for (int i = 0; i < kNumThreads; i++)
      pthread_join(tids[i], NULL);
  }
}
