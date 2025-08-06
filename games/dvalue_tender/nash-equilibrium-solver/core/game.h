#pragma once

#include "../util/util.h"

template <typename T, typename = int>
struct HasRedo : std::false_type {};

template <typename T>
struct HasRedo<T, decltype(&T::redo, 0)> : std::true_type {};

template <typename T, typename = int>
struct HasHash : std::false_type {};

template <typename T>
struct HasHash<T, decltype(&T::hash, 0)> : std::true_type {};

template <typename T, typename = int>
struct HasRandomRoll : std::false_type {};

template <typename T>
struct HasRandomRoll<T, decltype(&T::random_roll, 0)> : std::true_type {};

struct AlwaysSimultaneousGame {
  constexpr bool is_simultaneous() const { return true; }
};

struct NonSimultaneousGame {
  constexpr bool is_simultaneous() const { return false; }
};

struct SometimesSimultaneousGame {
  virtual bool is_simultaneous() const = 0;
};

struct FullyObservableGame {
  constexpr bool is_fully_observable() const { return true; }
};

struct PartiallyObservableGame {
  constexpr bool is_fully_observable() const { return false; }
};

template <class _Action, class _ActionSet = std::vector<_Action>>
class GameBase {
 public:
  using Action = _Action;
  using ActionSet = _ActionSet;

  constexpr static bool is_single_player() { return true; }

  virtual bool get_actor() const = 0;

  virtual bool is_ended() const = 0;
  virtual double get_score(bool) const = 0;
  virtual double get_score() const { return get_score(get_actor()); }

  virtual bool is_chance() const { return false; }
  virtual unique_ptr<float[]> get_chances(ActionSet&) const { return NULL; }

  virtual bool is_legal(const Action&) const { return false; }

  virtual void get_actions(ActionSet&) const {}

  virtual void get_actions_simultaneous(ActionSet&, ActionSet&) const {}

  virtual void step(const Action&) {}

  virtual void step_simultaneous(Action, Action) {}
};