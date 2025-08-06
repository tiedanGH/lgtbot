#include "dfs_diff.h"

int main() {
  DiffGame game{};
  game.init();
  DfsStrategy<DiffGame> st([](const DiffGame& g){
    if(g.coin[0]==g.coin[1] && g.score[0]==g.score[1]) return 0.;
    if(g.coin[0]==0 && g.score[0]<=g.score[1]) return -1.;
    if(g.coin[1]==0 && g.score[0]>=g.score[1]) return 1.;
    return NaN;
  }, "hash.bin");

  puts("Game started.\n");
  int round = 0;
  while (!game.is_ended()) {
    game.print();
    int rnd, rnd2;
    vector<pair<DiffGame::Action, double>> result;
    result = get_result(game, st, rnd, rnd2);
    cout << "Mixed strategy:" << endl;
    for (auto [action, value] : result) {
      if (action > 0 && value < 1e-5) continue;
      cout << (int)action << " : " << value << endl;
    }
    cout << "Randomized result: " << (int)result[rnd].first << "  " << (int)result[rnd2].first << endl;

    puts("\n==============\n");
    printf("Player 0, Input number: ");
    DiffGame::Action aa = readUnsigned();
    while (!CAN_OVERLOAD && aa > game.coin[0]) {
      puts("error. Input number: ");
      aa = readUnsigned();
    }
    printf("Player 1, Input number: ");
    DiffGame::Action bb = readUnsigned();
    while (!CAN_OVERLOAD && bb > game.coin[1]) {
      puts("error. Input number: ");
      bb = readUnsigned();
    }
    game.step_simultaneous(aa, bb);
  }
  printf("Player %d Win.", game.get_score(0) > 0 ? 0 : 1);

  return 0;
}