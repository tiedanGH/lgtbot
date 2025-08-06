#pragma once

#include "game.h"
#include "../util/util.h"

const int INITIAL = 30;
const int MID_SCORE = 5;
const int MID_TURN = 9;
const int MAX_SCORE = 7;
const int MAX_TURN = 12;
const bool SCORE_TIE_COMPARE_COIN = false;
const bool CAN_OVERLOAD = false;

class DiffGame : public GameBase<char>, AlwaysSimultaneousGame, FullyObservableGame {
 public:
  using Action = char;
  using ActionSet = vector<Action>;

  char coin[2], score[2], turn;

  void init() { coin[0] = coin[1] = INITIAL; }
  bool is_ended() const {
    if (turn < MID_TURN) return score[0] >= MID_SCORE || score[1] >= MID_SCORE;
    if (turn == MID_TURN && score[0] != score[1]) return true;
    return turn == MAX_TURN || score[0] >= MAX_SCORE || score[1] >= MAX_SCORE;
  }
  double get_score(bool side) const {
    return side?-get_score():get_score();
  }
  double get_score() const {
    assert(is_ended());
    if (turn < MID_TURN) {
      return score[0] >= MID_SCORE ? 1 : -1;
    }
    if (turn == MID_TURN) {
      return score[0] > score[1] ? 1 : -1;
    }
    if (turn < MAX_TURN) {
      return score[0] >= MAX_SCORE ? 1 : -1;
    }
    if (score[0] > score[1]) {
      return 1;
    }
    if (score[0] < score[1]) {
      return -1;
    }
    if (!SCORE_TIE_COMPARE_COIN) return 0;
    return coin[0]>coin[1] ? 1 : coin[0]<coin[1] ? -1 :0;
  }
  bool is_simultaneous() const { return true; }
  void step_simultaneous(Action our, Action their) {
    turn++;
    if (our < their) {
      coin[1] -= (their - our), score[1]++;
      if constexpr(CAN_OVERLOAD) {
        if(coin[1] < 0) score[1] = 0, score[0] = MAX_SCORE;
      }
    } else if (our > their){
      coin[0] -= (our - their), score[0]++;
      if constexpr(CAN_OVERLOAD) {
        if(coin[0] < 0) score[0] = 0, score[1] = MAX_SCORE;
      }
    }
  }
  void get_actions_simultaneous(ActionSet& a, ActionSet& b) const {
    assert(a.empty() && b.empty());
    if constexpr (CAN_OVERLOAD) {
      int maxx = INITIAL;
      a.resize(maxx+1);
      b.resize(maxx+1);
      for (int i = 0; i <= maxx; i++) a[i] = b[i] = i;
    } else {
      a.resize(coin[0] + 1);
      for (int i = 0; i <= coin[0]; i++) a[i] = i;
      b.resize(coin[1] + 1);
      for (int i = 0; i <= coin[1]; i++) b[i] = i;
    }
  }
  bool get_actor() const { return 0; }
  int hash() const {
    int result = 0, base = INITIAL + 1;
    assert(coin[0] >= 0 && coin[1] >= 0 && 
           coin[0] <= INITIAL && coin[1] <= INITIAL && score[0] < MAX_SCORE && 
           score[1] < MAX_SCORE && turn < MAX_TURN);
    result += coin[0];
    result += coin[1] * base, base *= INITIAL + 1;
    result += score[0] * base, base *= MAX_SCORE;
    result += score[1] * base, base *= MAX_SCORE;
    result += turn * base;
    return result;
  }
  constexpr static int hash_state_cnt() {
    return (INITIAL + 1) * (INITIAL + 1) * MAX_SCORE * MAX_SCORE * MAX_TURN;
  }
  void print() {
    printf("Turn %d. \n", turn);
    printf("coin : %d, %d\n", coin[0], coin[1]);
    printf("score: %d, %d\n", score[0], score[1]);
  }
};