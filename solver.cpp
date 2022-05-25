struct State {
  unsigned int corner1 : 5;
  unsigned int corner2 : 5;
  unsigned int corner3 : 5;
  unsigned int corner4 : 5;
  unsigned int corner5 : 5;
  unsigned int corner6 : 5;
  unsigned int corner7 : 5;
  unsigned int corner8 : 5;

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
  unsigned char prev_move;
};

static_assert(sizeof(State) == 16);

int main() { return 0; }
