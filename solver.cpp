using RotationState = unsigned char;

enum Rotation : unsigned char {
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
};

enum Move : unsigned char {
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
  MOVE_END
};

struct State {
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
  unsigned int edge5 : 5;
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

  unsigned char depth;
  Move prev_move;
};

constexpr RotationState rotate[][ROTATE_END] = {
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
  case MOVE_END:
    throw;
  }
}

#define ROTATE_EDGES(a, b, c, d)                                               \
  {                                                                            \
    auto tmp = state.edge##d;                                                  \
    state.edge##d = state.edge##c;                                             \
    state.edge##c = state.edge##b;                                             \
    state.edge##b = state.edge##a;                                             \
    state.edge##a = tmp;                                                       \
    state.edge##a##_parity = !state.edge##a##_parity;                          \
    state.edge##b##_parity = !state.edge##b##_parity;                          \
    state.edge##c##_parity = !state.edge##c##_parity;                          \
    state.edge##d##_parity = !state.edge##d##_parity;                          \
  }

#define ROTATE_CORNERS(a, b, c, d)                                             \
  {                                                                            \
    auto tmp = state.corner##d;                                                \
    state.corner##d = state.corner##c;                                         \
    state.corner##c = state.corner##b;                                         \
    state.corner##b = state.corner##a;                                         \
    state.corner##a = tmp;                                                     \
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
  case MOVE_RP:
  case MOVE_R2:
  case MOVE_L:
  case MOVE_LP:
  case MOVE_L2:
  case MOVE_F:
  case MOVE_FP:
  case MOVE_F2:
  case MOVE_B:
  case MOVE_BP:
  case MOVE_B2:
  case MOVE_U:
  case MOVE_UP:
  case MOVE_U2:
  case MOVE_D:
  case MOVE_DP:
  case MOVE_D2:
  case MOVE_END:
    throw;
  }
}

static_assert(sizeof(State) == 16);

int main() { return 0; }
