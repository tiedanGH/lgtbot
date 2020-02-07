#include "stdafx.h"
#include "lgtbot.h"
#include "message_handlers.h"
#include "../GameFrameWork/dllmain.h"
#include "msg_checker.h"
#include "dllmain.h"
#include "log.h"

#include <list>
#include <sstream>
#include <memory>
#include <optional>

const int32_t LGT_AC = -1;
const UserID INVALID_USER_ID = 0;
const GroupID INVALID_GROUP_ID = 0;

std::mutex g_mutex;
std::map<std::string, std::unique_ptr<GameHandle>> g_game_handles;

AT_CALLBACK g_at_cb = nullptr;
PRIVATE_MSG_CALLBACK g_send_pri_msg_cb = nullptr;
PUBLIC_MSG_CALLBACK g_send_pub_msg_cb = nullptr;
UserID g_this_uid = INVALID_USER_ID;

MatchManager match_manager;

static bool IsAtMe(const std::string& msg)
{
  return msg.find(At(g_this_uid)) != std::string::npos;
}

static void BoardcastPlayers(void* match, const char* const msg)
{
  static_cast<Match*>(match)->BoardcastPlayers(msg);
}

static void TellPlayer(void* match, const uint64_t pid, const char* const msg)
{
  static_cast<Match*>(match)->TellPlayer(pid, msg);
}

static void AtPlayer(void* match, const uint64_t pid, char* buf, const uint64_t len)
{
  static_cast<Match*>(match)->AtPlayer(pid, buf, len);
}

static void MatchGameOver(void* match, const int64_t scores[])
{
  static_cast<Match*>(match)->GameOver(scores);
  MatchManager::DeleteMatch(static_cast<Match*>(match)->mid());
}

static std::unique_ptr<GameHandle> LoadGame(HINSTANCE mod)
{
  if (!mod)
  {
    LOG_ERROR("Load mod failed");
    return nullptr;
  }

  typedef int (__cdecl *Init)(const boardcast, const tell, const at, const game_over);
  typedef char* (__cdecl *GameInfo)(uint64_t* const, uint64_t* const);
  typedef GameBase* (__cdecl *NewGame)(void* const match, const uint64_t);
  typedef int (__cdecl *DeleteGame)(GameBase* const);

  Init init = (Init)GetProcAddress(mod, "Init");
  GameInfo game_info = (GameInfo)GetProcAddress(mod, "GameInfo");
  NewGame new_game = (NewGame)GetProcAddress(mod, "NewGame");
  DeleteGame delete_game = (DeleteGame)GetProcAddress(mod, "DeleteGame");

  if (!init || !game_info || !new_game || !delete_game)
  {
    LOG_ERROR("Invalid Plugin DLL: some interface not be defined.");
    return nullptr;
  }

  if (!init(&BoardcastPlayers, &TellPlayer, &AtPlayer, &MatchGameOver))
  {
    LOG_ERROR("Init failed");
    return nullptr;
  }

  uint64_t min_player = 0;
  uint64_t max_player = 0;
  char* name = game_info(&min_player, &max_player);
  if (!name)
  {
    LOG_ERROR("Cannot get game game");
    return nullptr;
  }
  if (min_player == 0 || max_player < min_player)
  {
    LOG_ERROR("Invalid" + std::to_string(min_player) + std::to_string(max_player));
    return nullptr;
  }
  return std::make_unique<GameHandle>(name, min_player, max_player, new_game, delete_game, mod);
}

static void LoadGameModules()
{
  WIN32_FIND_DATA file_data;
  HANDLE file_handle = FindFirstFile(L".\\plugins\\*.dll", &file_data);
  if (file_handle == INVALID_HANDLE_VALUE) { return; }
  do {
    std::wstring dll_path = L".\\plugins\\" + std::wstring(file_data.cFileName);
    std::unique_ptr<GameHandle> game_handle = LoadGame(LoadLibrary(dll_path.c_str()));
    if (!game_handle)
    {
      LOG_ERROR(std::string(dll_path.begin(), dll_path.end()) + " loaded failed!\n");
    }
    else
    {
      g_game_handles.emplace(game_handle->name_, std::move(game_handle));
      LOG_INFO(std::string(dll_path.begin(), dll_path.end()) + " loaded success!\n");
    }    
  } while (FindNextFile(file_handle, &file_data));
  FindClose(file_handle);
  LOG_INFO("共加载游戏个数：" + std::to_string(g_game_handles.size()));
}

static std::string HandleMetaRequest(const UserID uid, const std::optional<GroupID> gid, MsgReader& reader)
{
  using MetaUserFuncType = std::string(const UserID, const std::optional<GroupID>);
  using MetaCommand = MsgCommand<MetaUserFuncType>;
  static const auto make_meta_command = [](const auto& cb, auto&&... checkers) -> std::shared_ptr<MetaCommand>
  {
    return MakeCommand<std::string(const UserID, const std::optional<GroupID>)>(cb, std::move(checkers)...);
  };
  static const std::vector<std::shared_ptr<MetaCommand>> meta_cmds =
  {
    make_meta_command(show_gamelist, std::make_unique<VoidChecker>("游戏列表")),
    make_meta_command(new_game, std::make_unique<VoidChecker>("新游戏"), std::make_unique<AnyArg>("游戏名称", "某游戏名")),
    make_meta_command(start_game, std::make_unique<VoidChecker>("开始游戏")),
    make_meta_command(leave, std::make_unique<VoidChecker>("退出游戏")),
    make_meta_command(join_public, std::make_unique<VoidChecker>("加入游戏")),
    make_meta_command(join_private, std::make_unique<VoidChecker>("加入游戏"), std::make_unique<BasicChecker<MatchId>>("赛事编号")),
  };
  reader.Reset();
  for (const std::shared_ptr<MetaCommand>& cmd : meta_cmds)
  {
    if (std::optional<std::string> reply_msg = cmd->CallIfValid(reader, std::tuple{ uid, gid })) { return *reply_msg; }
  }
  return "[错误] 未预料的元请求";
}

static std::string HandleRequest(const UserID uid, const std::optional<GroupID> gid, const std::string& msg)
{
  if (msg.empty()) { return "[错误] 空白消息"; }
  if (msg[0] == '#') { return HandleMetaRequest(uid, gid, MsgReader(msg.substr(1, msg.size()))); }
  else
  {
    std::shared_ptr<Match> match = MatchManager::GetMatch(uid, gid);
    if (!match) { return "[错误] 您未参与游戏"; }
    if (match->gid() != gid && gid.has_value()) { return "[错误] 您未在本群参与游戏"; }
    return match->Request(uid, gid, msg);
  }
}

bool __cdecl BOT_API::Init(const UserID this_uid, const PRIVATE_MSG_CALLBACK pri_msg_cb, const PUBLIC_MSG_CALLBACK pub_msg_cb, const AT_CALLBACK at_cb)
{
  if (this_uid == INVALID_USER_ID || !pri_msg_cb || !pub_msg_cb || !at_cb) { return false; }
  g_this_uid = this_uid;
  g_send_pri_msg_cb = pri_msg_cb;
  g_send_pub_msg_cb = pub_msg_cb;
  g_at_cb = at_cb;
  LoadGameModules();
  return true;
}

void __cdecl BOT_API::HandlePrivateRequest(const UserID uid, const char* const msg)
{
  if (const std::string reply_msg = HandleRequest(uid, {}, msg); !reply_msg.empty()) { SendPrivateMsg(uid, reply_msg); }
}

void __cdecl BOT_API::HandlePublicRequest(const UserID uid, const GroupID gid, const char* const msg)
{
  if (const std::string reply_msg = HandleRequest(uid, gid, msg); !reply_msg.empty()) { SendPublicMsg(gid, At(uid) + reply_msg); };
}
