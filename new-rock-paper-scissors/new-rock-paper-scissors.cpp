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

bool __cdecl Init(const boardcast boardcast, const tell tell, const at at, const game_over game_over)
{

  return true;
}

char* __cdecl GameInfo(uint64_t* min_player, uint64_t* max_player)
{

  return NULL;
}

GameBase* __cdecl NewGame(const uint64_t match_id, const uint64_t player_num)
{
  return new Game(match_id, MakeGameEnv(player_num), MakeMainStage());
}

void __cdecl DeleteGame(GameBase* const game)
{
  delete(game);
}
