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
static const int kScrambleDepth = 13;

const size_t kMemGB = 1;
const size_t kMemB = kMemGB * 1024 * 1024 * 1024;
const size_t kTableSize = kMemB / sizeof(State) / 2;

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

static inline void unpack(State state, Edge edges[12], bool edge_parities[12],
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
  edge_parities[0] = state.edge1_parity;
  edge_parities[1] = state.edge2_parity;
  edge_parities[2] = state.edge3_parity;
  edge_parities[3] = state.edge4_parity;
  edge_parities[4] = state.edge5_parity;
  edge_parities[5] = state.edge6_parity;
  edge_parities[6] = state.edge7_parity;
  edge_parities[7] = state.edge8_parity;
  edge_parities[8] = state.edge9_parity;
  edge_parities[9] = state.edge10_parity;
  edge_parities[10] = state.edge11_parity;
  edge_parities[11] = state.edge12_parity;
  corners[0] = state.corner1;
  corners[1] = state.corner2;
  corners[2] = state.corner3;
  corners[3] = state.corner4;
  corners[4] = state.corner5;
  corners[5] = state.corner6;
  corners[6] = state.corner7;
  corners[7] = state.corner8;
}

static inline State pack(State state, const Edge edges[12],
                         const bool edge_parities[12],
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
  state.edge1_parity = edge_parities[0];
  state.edge2_parity = edge_parities[1];
  state.edge3_parity = edge_parities[2];
  state.edge4_parity = edge_parities[3];
  state.edge5_parity = edge_parities[4];
  state.edge6_parity = edge_parities[5];
  state.edge7_parity = edge_parities[6];
  state.edge8_parity = edge_parities[7];
  state.edge9_parity = edge_parities[8];
  state.edge10_parity = edge_parities[9];
  state.edge11_parity = edge_parities[10];
  state.edge12_parity = edge_parities[11];
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
  Side side = move_to_side(move);
  Turn turn = move_to_turn(move);
  const Edge *edge_orbit = edge_orbits[side];
  const Corner *corner_orbit = corner_orbits[side];
  Edge edges[12];
  bool edge_parities[12];
  RotationState corners[8];
  unpack(state, edges, edge_parities, corners);
  Edge tmp_edge;
  bool tmp_bool;
  RotationState tmp_rot;
  switch (turn) {
  case CW:
    tmp_edge = edges[edge_orbit[1]];
    edges[edge_orbit[1]] = edges[edge_orbit[0]];
    edges[edge_orbit[0]] = edges[edge_orbit[3]];
    edges[edge_orbit[3]] = edges[edge_orbit[2]];
    edges[edge_orbit[2]] = tmp_edge;

    tmp_bool = !edge_parities[edge_orbit[1]];
    edge_parities[edge_orbit[1]] = !edge_parities[edge_orbit[0]];
    edge_parities[edge_orbit[0]] = !edge_parities[edge_orbit[3]];
    edge_parities[edge_orbit[3]] = !edge_parities[edge_orbit[2]];
    edge_parities[edge_orbit[2]] = tmp_bool;

    tmp_rot = rotate[corners[corner_orbit[1]]][rotation];
    corners[corner_orbit[1]] = rotate[corners[corner_orbit[0]]][rotation];
    corners[corner_orbit[0]] = rotate[corners[corner_orbit[3]]][rotation];
    corners[corner_orbit[3]] = rotate[corners[corner_orbit[2]]][rotation];
    corners[corner_orbit[2]] = tmp_rot;
    break;
  case HALF:
    tmp_edge = edges[edge_orbit[0]];
    edges[edge_orbit[0]] = edges[edge_orbit[2]];
    edges[edge_orbit[2]] = tmp_edge;
    tmp_edge = edges[edge_orbit[1]];
    edges[edge_orbit[1]] = edges[edge_orbit[3]];
    edges[edge_orbit[3]] = tmp_edge;

    tmp_bool = edge_parities[edge_orbit[0]];
    edge_parities[edge_orbit[0]] = edge_parities[edge_orbit[2]];
    edge_parities[edge_orbit[2]] = tmp_bool;
    tmp_bool = edge_parities[edge_orbit[1]];
    edge_parities[edge_orbit[1]] = edge_parities[edge_orbit[3]];
    edge_parities[edge_orbit[3]] = tmp_bool;

    tmp_rot = rotate[corners[corner_orbit[0]]][rotation];
    corners[corner_orbit[0]] = rotate[corners[corner_orbit[2]]][rotation];
    corners[corner_orbit[2]] = tmp_rot;
    tmp_rot = rotate[corners[corner_orbit[1]]][rotation];
    corners[corner_orbit[1]] = rotate[corners[corner_orbit[3]]][rotation];
    corners[corner_orbit[3]] = tmp_rot;
    break;
  case CCW:
    tmp_edge = edges[edge_orbit[0]];
    edges[edge_orbit[0]] = edges[edge_orbit[1]];
    edges[edge_orbit[1]] = edges[edge_orbit[2]];
    edges[edge_orbit[2]] = edges[edge_orbit[3]];
    edges[edge_orbit[3]] = tmp_edge;

    tmp_bool = !edge_parities[edge_orbit[0]];
    edge_parities[edge_orbit[0]] = !edge_parities[edge_orbit[1]];
    edge_parities[edge_orbit[1]] = !edge_parities[edge_orbit[2]];
    edge_parities[edge_orbit[2]] = !edge_parities[edge_orbit[3]];
    edge_parities[edge_orbit[3]] = tmp_bool;

    tmp_rot = rotate[corners[corner_orbit[0]]][rotation];
    corners[corner_orbit[0]] = rotate[corners[corner_orbit[1]]][rotation];
    corners[corner_orbit[1]] = rotate[corners[corner_orbit[2]]][rotation];
    corners[corner_orbit[2]] = rotate[corners[corner_orbit[3]]][rotation];
    corners[corner_orbit[3]] = tmp_rot;
    break;
  }
  state = pack(state, edges, edge_parities, corners);
  state.prev_move = move;
  state.depth++;
  return state;
}

static State delta(State a, State b) {
  Edge ea[12], eb[12], ec[12];
  bool pa[12], pb[12], pc[12];
  RotationState ra[8], rb[8], rc[8];
  unpack(a, ea, pa, ra);
  unpack(b, eb, pb, rb);

  Edge ie[12];
  for (Edge i = 0; i < 12; i++)
    ie[ea[i]] = i;
  for (Edge i = 0; i < 12; i++) {
    ec[i] = ie[eb[i]];
    pc[i] = eb[i] ^ ea[ie[eb[i]]];
  }

  Corner ca[8], cb[8];
  for (Corner i = 0; i < 8; i++) {
    ca[i] = rotation_to_corner[ra[i]][i];
    cb[i] = rotation_to_corner[rb[i]][i];
  }
  Corner ic[8];
  for (Corner i = 0; i < 8; i++)
    ic[ca[i]] = i;
  for (Corner i = 0; i < 8; i++)
    rc[i] = rotation_delta[ra[ic[i]]][rb[i]];

  State state = {
      .unused = 0,
      .prev_move = 0,
      .depth = 0,
  };
  return pack(state, ec, pc, rc);
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
  Side prev = SIDE_END;
  for (int i = 0; i < kScrambleDepth; i++) {
    Move move;
    do {
      move = rand() % (MOVE_END + 1);
    } while (move_to_side(move) == prev);
    prev = move_to_side(move);
    scrambled = make_move(scrambled, move);
    printf("%s ", move_to_string(move));
  }
  printf("\n");
  scrambled.depth = 1;
  scrambled.prev_move = 0;
  return scrambled;
}

int main() {
  State *mem = mmap(NULL, kMemB, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE | MAP_HUGETLB |
                        MAP_HUGE_1GB,
                    -1, 0);
  if (mem == MAP_FAILED) {
    fprintf(stderr, "mmap() failed.  Falling back to malloc().\n");
    mem = malloc(kMemB);
    if (mem == NULL) {
      fprintf(stderr, "malloc() failed.\n");
      return 1;
    }
  }

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
