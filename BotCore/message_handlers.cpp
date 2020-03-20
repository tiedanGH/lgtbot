#include "stdafx.h"
#include "message_handlers.h"
#include "match.h"
#include "log.h"
#include "msg_checker.h"

using MetaUserFuncType = std::string(const UserID, const std::optional<GroupID>);
using MetaCommand = MsgCommand<MetaUserFuncType>;

extern const std::vector<std::shared_ptr<MetaCommand>> meta_cmds;

static const auto make_meta_command = [](std::string&& description, const auto& cb, auto&&... checkers) -> std::shared_ptr<MetaCommand>
{
  return MakeCommand<std::string(const UserID, const std::optional<GroupID>)>(std::move(description), cb, std::move(checkers)...);
};

static std::string help(const UserID uid, const std::optional<GroupID> gid)
{
  std::stringstream ss;
  ss << "[可使用的元指令]";
  int i = 0;
  for (const std::shared_ptr<MetaCommand>& cmd : meta_cmds)
  {
    ss << std::endl << "[" << (++i) << "] " << cmd->Info();
  }
  return ss.str();
}

static std::string show_gamelist(const UserID uid, const std::optional<GroupID> gid)
{
  std::stringstream ss;
  int i = 0;
  ss << "游戏列表：";
  for (const auto& [name, _] : g_game_handles)
  {
    ss << std::endl << (++i) << ". " << name;
  }
  return ss.str();
}

static std::string new_game(const UserID uid, const std::optional<GroupID> gid, const std::string& gamename)
{
  std::string err_msg;
  const auto it = g_game_handles.find(gamename);
  if (it == g_game_handles.end()) { err_msg = "创建游戏失败：未知的游戏名"; }
  else if (gid.has_value()) { err_msg = MatchManager::NewMatch(*it->second, uid, gid); }
  else { err_msg = MatchManager::NewMatch(*it->second, uid, gid); }
  return err_msg.empty() ? "创建游戏成功" : "[错误] " + err_msg;
}

static std::string start_game(const UserID uid, const std::optional<GroupID> gid)
{
  std::string err_msg = MatchManager::StartGame(uid, gid);
  return err_msg.empty() ? "" : "[错误] " + err_msg;
}

static std::string leave(const UserID uid, const std::optional<GroupID> gid)
{
  std::string err_msg = MatchManager::DeletePlayer(uid, gid);
  return err_msg.empty() ? "" : "[错误] " + err_msg;
}

static std::string join_private(const UserID uid, const std::optional<GroupID> gid, const MatchId match_id)
{
  std::string err_msg;
  if (gid.has_value()) { err_msg = "加入游戏失败：请私信裁判加入私密游戏，或去掉比赛ID以加入当前房间游戏"; }
  else { err_msg = MatchManager::AddPlayerToPrivateGame(match_id, uid); }
  return err_msg.empty() ? "" : "[错误] " + err_msg;
}

static std::string join_public(const UserID uid, const std::optional<GroupID> gid)
{
  std::string err_msg;
  if (!gid.has_value()) { err_msg = "加入私密游戏，请指明比赛ID"; }
  else { err_msg = MatchManager::AddPlayerToPublicGame(*gid, uid); }
  return err_msg.empty() ? "加入成功" : "[错误] " + err_msg;
}

static std::string show_private_matches(const UserID uid, const std::optional<GroupID> gid)
{
  std::stringstream ss;
  uint64_t count = 0;
  MatchManager::ForEachMatch([&ss, &count](const std::shared_ptr<Match> match)
  {
    if (match->IsPrivate() && !match->is_started())
    {
      ++count;
      ss << std::endl << match->game_handle().name_ << " - [房主ID] " << match->host_uid() << " - [比赛ID] " << match->mid();
    }
  });
  if (ss.rdbuf()->in_avail() == 0) { return "当前无未开始的私密比赛"; }
  else { return "共" + std::to_string(count) + "场：" + ss.str(); }
}

static std::string show_match_status(const UserID uid, const std::optional<GroupID> gid)
{
  std::stringstream ss;
  std::shared_ptr<Match> match = MatchManager::GetMatch(uid, gid);
  if (!match && gid.has_value()) { match = MatchManager::GetMatchWithGroupID(*gid); }
  if (!match) { return "[错误] 您未加入游戏，或该房间未进行游戏"; }
  ss << "游戏名称：" << match->game_handle().name_ << std::endl;
  ss << "游戏状态：" << (match->is_started() ? "正在进行" : "未进行") << std::endl;
  ss << "房间号：";
  if (match->gid().has_value()) { ss << *gid << std::endl; }
  else { ss << "私密游戏" << std::endl; }
  ss << "可参加人数：" << match->game_handle().min_player_;
  if (const uint64_t max_player = match->game_handle().max_player_; max_player > match->game_handle().min_player_) { ss << "~" << max_player; }
  else if (max_player == 0) { ss << "+"; }
  ss << "人" << std::endl;
  ss << "房主：" << match->host_uid() << std::endl;
  const std::set<uint64_t>& ready_uid_set = match->ready_uid_set();
  ss << "已参加玩家：" << ready_uid_set.size() << "人";
  for (const uint64_t uid : ready_uid_set) { ss << std::endl << uid; }
  return ss.str();
}

static std::string show_rule(const UserID uid, const std::optional<GroupID> gid, const std::string& gamename)
{
  std::stringstream ss;
  const auto it = g_game_handles.find(gamename);
  if (it == g_game_handles.end()) { return "[错误] 未知的游戏名"; }
  ss << "可参加人数：" << it->second->min_player_;
  if (const uint64_t max_player = it->second->max_player_; max_player > it->second->min_player_) { ss << "~" << max_player; }
  else if (max_player == 0) { ss << "+"; }
  ss << "人" << std::endl;
  ss << "详细规则：" << std::endl;
  ss << it->second->rule_;
  return ss.str();
}

static const std::vector<std::shared_ptr<MetaCommand>> meta_cmds =
{
  make_meta_command("查看帮助", help, std::make_unique<VoidChecker>("#帮助")),
  make_meta_command("显示游戏列表", show_gamelist, std::make_unique<VoidChecker>("#游戏列表")),
  make_meta_command("在当前房间建立公开游戏，或私信bot以建立私密游戏", new_game, std::make_unique<VoidChecker>("#新游戏"), std::make_unique<AnyArg>("游戏名称", "某游戏名")),
  make_meta_command("房主开始游戏", start_game, std::make_unique<VoidChecker>("#开始游戏")),
  make_meta_command("在游戏开始前退出游戏", leave, std::make_unique<VoidChecker>("#退出游戏")),
  make_meta_command("加入当前房间的公开游戏", join_public, std::make_unique<VoidChecker>("#加入游戏")),
  make_meta_command("私信bot以加入私密游戏", join_private, std::make_unique<VoidChecker>("#加入游戏"), std::make_unique<BasicChecker<MatchId>>("私密比赛编号")),
  make_meta_command("查看当前所有未开始的私密比赛", show_private_matches, std::make_unique<VoidChecker>("#私密游戏列表")),
  make_meta_command("查看已加入，或该房间正在进行的比赛信息", show_match_status, std::make_unique<VoidChecker>("#游戏信息")),
  make_meta_command("查看游戏规则", show_rule, std::make_unique<VoidChecker>("#规则"), std::make_unique<AnyArg>("游戏名称", "某游戏名")),
};

std::string HandleMetaRequest(const UserID uid, const std::optional<GroupID> gid, MsgReader& reader)
{

  reader.Reset();
  for (const std::shared_ptr<MetaCommand>& cmd : meta_cmds)
  {
    if (std::optional<std::string> reply_msg = cmd->CallIfValid(reader, std::tuple{ uid, gid })) { return *reply_msg; }
  }
  return "[错误] 未预料的元请求";
}