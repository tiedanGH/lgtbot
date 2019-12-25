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

typedef void (*boardcast)(const uint64_t game_id, char* msg);
typedef void (*tell)(const uint64_t game_id, const uint64_t player_id, char* msg);
typedef char* (*at)(const uint64_t game_id, const uint64_t player_id);
typedef void (*game_over)(const uint64_t game_id, const int64_t scores[]);

static std::unique_ptr<char[]> tmp_msg_cache;

enum ERR_CODE
{
  LGT_SUCCESS = 0,
  LGT_INVALID_ARGUMENT,
  LGT_GAME_NOT_FOUND,
};

extern "C" int Init(const boardcast boardcast, const tell tell, const at at, const game_over game_over)
{
  return LGT_SUCCESS;
}

extern "C" int NewGame(const unsigned long player_num, uint64_t* game_id)
{
  while (g_games.find(++g_current_game_id) == g_games.end());
  std::unique_ptr<Game> game = std::make_unique<Game>(g_current_game_id, MakeGameEnv(player_num), MakeMainStage());
  assert(g_games.emplace(g_current_game_id, std::move(game)).second);
  return LGT_SUCCESS;
}

extern "C" int HandleRequest(const uint64_t game_id, const uint64_t player_id, const bool is_public,
  const char* msg, void(*const reply)(const uint64_t, const uint64_t, const bool, const char*))
{
  const auto it = g_games.find(game_id);
  if (it == g_games.end()) { return LGT_GAME_NOT_FOUND; }
  if (!msg || !reply) { return LGT_INVALID_ARGUMENT; }
  const auto reply_f = [reply, game_id, player_id, is_public](const std::string& reply_msg) { reply(game_id, player_id, is_public, reply_msg.c_str()); };
  const bool handled = it->second->HandleRequest(player_id, is_public, msg, reply_f);
  if (!handled) { reply_f("[错误] 预料之外的请求"); }
  if (it->second->IsOver()) { g_games.erase(it); }
  return LGT_SUCCESS;
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

