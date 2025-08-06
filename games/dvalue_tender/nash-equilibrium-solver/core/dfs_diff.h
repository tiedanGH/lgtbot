#include "game_diff.h"
#include "strategy_dfs.h"

vector<pair<DiffGame::Action, double>> get_result(DiffGame& game, DfsStrategy<DiffGame>& st, int &rnd, int &rnd2) {
  vector<pair<DiffGame::Action, double>> result;
  DiffGame::ActionSet a, b;
  game.get_actions_simultaneous(a, b);
  int start_time = clock();
  auto value = st.evaluate(game);
  st.get_mixed_strategy(game, result, rnd, rnd2);
  cout << "Turn: " << std::to_string(game.turn) << ", Evaluate result = " << value << ", used time = " << (clock() - start_time) / 1000 << "ms" << endl;
  return result;
}
