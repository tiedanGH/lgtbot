// new-rock-paper-scissors.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include <memory>
#include <map>
#include <functional>
#include <vector>
#include <iostream>
#include "game.h"
#include "dllmain.h"

typedef void (*request_f)(char const* msg);

class Game;
class Stage;

std::unique_ptr<GameEnv> MakeGameEnv(const uint64_t player_num);
std::unique_ptr<Stage> MakeMainStage();

static uint64_t g_current_game_id = 0;
static std::map<uint64_t, std::unique_ptr<Game>> g_games;

static std::unique_ptr<char[]> tmp_msg_cache;

ERR_CODE __cdecl Init(const boardcast boardcast, const tell tell, const at at, const game_over game_over)
{

  return LGT_SUCCESS;
}

ERR_CODE __cdecl NewGame(const unsigned long player_num, uint64_t* game_id)
{
  if (!game_id) { return LGT_INVALID_ARGUMENT; }
  while (g_games.find(++g_current_game_id) == g_games.end());
  std::unique_ptr<Game> game = std::make_unique<Game>(g_current_game_id, MakeGameEnv(player_num), MakeMainStage());
  assert(g_games.emplace(g_current_game_id, std::move(game)).second);
  return LGT_SUCCESS;
}

ERR_CODE __cdecl HandleRequest(const uint64_t game_id, const uint64_t player_id, const bool is_public, const char* msg)
{
  const auto it = g_games.find(game_id);
  if (it == g_games.end()) { return LGT_GAME_NOT_FOUND; }
  if (!msg) { return LGT_INVALID_ARGUMENT; }
  const auto reply_f = [game_id, player_id, is_public](const std::string& reply_msg)
  {
    if (is_public) { boardcast_f(game_id, at_f(game_id, player_id) + reply_msg); }
    else { tell_f(game_id, player_id, reply_msg); }
  };
  Game& game = *it->second;
  const bool handled = game.HandleRequest(player_id, is_public, msg, reply_f);
  if (!handled) { reply_f("[错误] 预料之外的请求"); }
  if (game.IsOver()) { g_games.erase(it); }
  return LGT_SUCCESS;
}