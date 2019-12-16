// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <memory>
#include <map>
#include <functional>
#include <vector>
#include <iostream>
#include "game.h"

typedef void (*request_f)(char const* msg);

class Game;
class Stage;

std::unique_ptr<GameEnv> MakeGameEnv(const uint64_t player_num);
std::unique_ptr<Stage> MakeMainStage();

static uint64_t g_current_game_id = 0;
static std::map<uint64_t, std::unique_ptr<Game>> g_games;

extern "C" bool NewGame(const unsigned long player_num,
                            void (*private_msg)(const uint64_t player_id, char const* msg),
                            void (*boardcast)(char const* msg),
                            void (*at)(const uint64_t player_id, char* at_str, const uint64_t buffer_size),
                            void (*game_over)(const int64_t score[]),
                            uint64_t* game_id)
{
  if (!private_msg || !boardcast || !game_over || !game_id) { return false; }
  
  const auto private_msg_f = [private_msg](const uint64_t player_id, const std::string& msg) { private_msg(player_id, msg.c_str()); };
  const auto boardcast_f = [boardcast](const std::string& msg) { boardcast(msg.c_str()); };
  const auto game_over_f = [game_over](const std::vector<int64_t>& scores) { game_over(scores.data()); };
  const auto at_f = [at](const uint64_t player_id)
  {
    constexpr uint64_t buffer_size = 100;
    char buffer[buffer_size];
    at(player_id, buffer, buffer_size);
    return std::string(buffer);
  };

  std::unique_ptr<Game> game = std::make_unique<Game>(private_msg_f, boardcast_f, game_over_f, at_f, MakeGameEnv(player_num), MakeMainStage());
  while (!g_games.emplace(g_current_game_id ++, std::move(game)).second);
  *game_id = g_current_game_id - 1;

  return true;
}

extern "C" bool HandleRequest(const uint64_t game_id, const uint64_t player_id, char const* msg, void (*reply)(char const* msg))
{
  const auto it = g_games.find(game_id);
  if (it == g_games.end()) { return false; }
  std::string reply_msg = it->second->HandleRequest(player_id, msg);
  if (!reply_msg.empty()) { reply(reply_msg.c_str()); }
  if (it->second->IsOver()) { g_games.erase(it); }
  return true;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

