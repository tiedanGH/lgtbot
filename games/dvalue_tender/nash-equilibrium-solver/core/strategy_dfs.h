#pragma once
#include "game.h"
#include "../util/util.h"

#include <thread>
#include <atomic>
#include <fstream>

// const double magic_eps = 1e-8;

template <class Game, class HashType=double>
class DfsStrategy {
 public:
  using Action = typename Game::Action;
  using ActionSet = typename Game::ActionSet;
  DfsStrategy(const std::function<double(const Game&)>& greedy = 0, const std::string& hash_file = ""): greedy(greedy) {
    hash = nullptr;
    if constexpr (HasHash<Game>::value) {
      if constexpr (const int c = Game::hash_state_cnt(); c > 0) {
        hash = new HashType[c];
        std::fill(hash, hash + c, NaN);
        if (!hash_file.empty()) {
          load_hash_table(hash_file, c);
        }
      }
    }
  }
  ~DfsStrategy() {
    if (hash) {
      delete[] hash;
    }
  }

  void save_hash_table(const std::string& hash_file) const {
    if constexpr (HasHash<Game>::value) {
      if (!hash) return;

      std::ofstream out(hash_file, std::ios::binary);
      if (!out) {
        throw std::runtime_error("Failed to open file for writing: " + hash_file);
      }
      for (int i = 0; i < Game::hash_state_cnt(); ++i) {
        out.write(reinterpret_cast<const char*>(&hash[i]), sizeof(HashType));
      }
      out.close();
    }
  }

  void load_hash_table(const std::string& hash_file, int size) {
    std::ifstream in(hash_file, std::ios::binary);
    if (!in) {
      throw std::runtime_error("Failed to open file for reading: " + hash_file);
    }
    for (int i = 0; i < size; ++i) {
      in.read(reinterpret_cast<char*>(&hash[i]), sizeof(HashType));
      if (in.eof()) {
        throw std::runtime_error("Unexpected end of file while reading hash table.");
      }
    }
    in.close();
  }

  void evaluate_actions(const Game &g, vector<pair<Action, double>> &result, int max_depth = 0) {
    if (g.is_ended()) return;
    assert(!g.is_simultaneous());
    // assert(g.is_fully_observable());
    bool side = g.get_actor();

    ActionSet actions;
    g.get_actions(actions);
    result.clear();
    for (auto &action : actions) {
      if constexpr (HasRedo<Game>::value) {
      } else {
        Game fork = g;
        fork.step(action);
        double value = evaluate(fork, side, max_depth?max_depth:-1);
        result.emplace_back(action, value);
      }
    }
  }

  void get_mixed_strategy(const Game &g, vector<pair<Action, double>> &result) {
    if (g.is_ended()) return;
    assert(g.is_simultaneous());
    // assert(g.is_fully_observable());
    bool side = g.get_actor();

    ActionSet ours, theirs;
    g.get_actions_simultaneous(ours, theirs);
    result.clear();
    int dim1 = ours.size(), dim2 = theirs.size(), i = 0;
    auto bimatrix = create_bimatrix(dim1, dim2);
    double mi = 0;
    for (auto &action : ours) {
      int j = 0;
      for (auto &action2 : theirs) {
        if constexpr (HasRedo<Game>::value) {
        } else {
          Game fork = g;
          fork.step_simultaneous(action, action2);
          mi = min(mi, bimatrix[i][j] = evaluate(fork, side));
          mi = min(mi, bimatrix[i + dim1][j] = -bimatrix[i][j]);
        }
        j++;
      }
      i++;
    }
    // for (int i = 0; i < dim1; i++) {
    //   for (int j = 0; j < dim2; j++) {
    //     printf("%lf%c", bimatrix[i][j], j == dim2 - 1 ? '\n' : ' ');
    //   }
    // }
    positivize_bimatrix(bimatrix, dim1, dim2, mi);
    auto eq = solve_equilibrium(bimatrix, dim1, dim2);
    free_bimatrix(bimatrix, dim1, dim2);
    int index = 0;
    for (auto &action : ours) {
      result.emplace_back(action, eq[index++]);
    }
    for (auto &action : theirs) {
      result.emplace_back(action, eq[index++]);
    }
    free(eq);
  }

  void get_mixed_strategy(const Game &g, vector<pair<Action, double>> &result, int &rnd,
                          int &rnd2) {
    if (g.is_ended()) return;
    assert(g.is_simultaneous());
    // assert(g.is_fully_observable());
    bool side = g.get_actor();
    ActionSet ours, theirs;
    g.get_actions_simultaneous(ours, theirs);
    result.clear();
    int dim1 = ours.size(), dim2 = theirs.size(), i = 0, j;
    auto bimatrix = create_bimatrix(dim1, dim2);
    double mi = 0;
    for (auto &action : ours) {
      j = 0;
      for (auto &action2 : theirs) {
        if constexpr (HasRedo<Game>::value) {
        } else {
          Game fork = g;
          fork.step_simultaneous(action, action2);
          mi = min(mi, bimatrix[i][j] = evaluate(fork, side));
          mi = min(mi, bimatrix[i + dim1][j] = -bimatrix[i][j]);
        }
        j++;
      }
      i++;
    }
    positivize_bimatrix(bimatrix, dim1, dim2, mi);
    auto eq = solve_equilibrium(bimatrix, dim1, dim2);
    free_bimatrix(bimatrix, dim1, dim2);
    int index = 0;
    std::discrete_distribution<int> dist(eq, eq + dim1), dist2(eq + dim1, eq + dim1 + dim2);
    static std::mt19937 gen(time(0) ^ clock());
    rnd = dist(gen), rnd2 = dist2(gen) + dim1;
    for (auto &action : ours) {
      result.emplace_back(action, eq[index++]);
    }
    for (auto &action : theirs) {
      result.emplace_back(action, eq[index++]);
    }
    free(eq);
  }

  double evaluate(Game &g) { return evaluate(g, g.get_actor()); }
  double evaluate(Game &g, bool side, char max_depth = -1) {
    if (g.is_ended()) return g.get_score(side);
    if (g.get_actor() != side) return -evaluate(g, !side, max_depth);
    int ghash;
    if constexpr (HasHash<Game>::value) {
      ghash = g.hash();
      assert(ghash > -1 && ghash < Game::hash_state_cnt());
      if (ghash > 0 && !isnan(hash[ghash])) {
        return hash[ghash];
      }
    }
    if (double result; greedy && !isnan(result = greedy(g))) {
        if constexpr (HasHash<Game>::value) {
          if (ghash >= 0) return hash[ghash] = result;
        }
        return result;
    }
    if (max_depth == 0) return 0;
    if (g.is_chance()) {
      ActionSet chances;
      auto p = g.get_chances(chances);
      double sumv = 0;
      for (int i = 0; i < chances.size(); i++) {
        if constexpr (HasRedo<Game>::value) {
          g.step(chances[i]);
          double value = evaluate(g, side, max_depth - 1);
          sumv += value * p[i];
          g.redo(chances[i]);
        } else {
          Game fork = g;
          fork.step(chances[i]);
          double value = evaluate(fork, side, max_depth - 1);
          sumv += value * p[i];
        }
      }
      if constexpr (HasHash<Game>::value) {
        if (ghash >= 0) hash[ghash] = sumv;
      }
      return sumv;
    }
    else if (g.is_simultaneous()) {
      ActionSet ours, theirs;
      g.get_actions_simultaneous(ours, theirs);
      int dim1 = ours.size(), dim2 = theirs.size(), i = 0;
      auto bimatrix = create_bimatrix(dim1, dim2);
      double mi = 0;
      for (auto &action : ours) {
        int j = 0;
        for (auto &action2 : theirs) {
          if constexpr (HasRedo<Game>::value) {
          } else {
            Game fork = g;
            fork.step_simultaneous(action, action2);
            mi = min(mi, bimatrix[i][j] = evaluate(fork, side, max_depth - 1));
            mi = min(mi, bimatrix[i + dim1][j] = -bimatrix[i][j]);
          }
          j++;
        }
        i++;
      }

      positivize_bimatrix(bimatrix, dim1, dim2, mi);

      double result = calculate_ev(bimatrix, dim1, dim2) + mi - 1;
      free_bimatrix(bimatrix, dim1, dim2);
      if constexpr (HasHash<Game>::value) {
        if (ghash >= 0) hash[ghash] = result;
      }
      return result;
    } else {
      ActionSet actions;
      g.get_actions(actions);
      if (used_threads < max_threads) {
          std::atomic<double> maxv = -1e9;
          used_threads += actions.size();
          std::vector<std::thread> threads;
          for (auto& action : actions) {
              threads.emplace_back([&](){
                  Game fork = g;
                  fork.step(action);
                  double value = evaluate(fork, side, max_depth - 1);
                  maxv = max(maxv.load(), value);
              });
          }
          for (auto& t : threads) t.join();
          used_threads -= actions.size();

          if constexpr (HasHash<Game>::value) {
              // g.print();
              // printf("updated hash = %lf.\n", maxv);
              if (ghash >= 0) hash[ghash] = maxv;
          }
          return maxv;
      }
      else {
          double maxv = -1e9;
          for (auto& action : actions) {
              if constexpr (HasRedo<Game>::value) {
                  g.step(action);
                  double value = evaluate(g, side, max_depth - 1);
                  maxv = max(maxv, value);
                  g.redo(action);
                  if (maxv >= 1) break;
              }
              else {
                  Game fork = g;
                  fork.step(action);
                  double value = evaluate(fork, side, max_depth - 1);
                  maxv = max(maxv, value);
                  if (maxv >= 1) break;
              }
          }
          if constexpr (HasHash<Game>::value) {
            // g.print();
            // printf("updated hash = %lf.\n", maxv);
            if (ghash >= 0) hash[ghash] = maxv;
          }
          return maxv;
      }
    }
  }

 private:
  double *hash;
  std::function<double(const Game&)> greedy;

  std::atomic<int> used_threads = 1;
  int max_threads = 16;
  // std::unordered_map<int, double> hash;
};