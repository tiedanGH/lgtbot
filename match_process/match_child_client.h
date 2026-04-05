#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "bot_core/bot_core.h"
#include "bot_core/id.h"
#include "game_framework/game_main.h"
#include "nlohmann/json.hpp"

#include "match_process/subprocess.h"

class Match;
class MsgSender;
class Subprocess;

// Parent-side RAII client: owns game subprocess and stdin/stdout framed JSON protocol.
class MatchChildClient
{
  public:
    MatchChildClient(std::filesystem::path runner_exe, std::filesystem::path game_library);
    ~MatchChildClient();

    MatchChildClient(const MatchChildClient&) = delete;
    MatchChildClient& operator=(const MatchChildClient&) = delete;

    [[nodiscard]] bool ok() const { return static_cast<bool>(proc_); }

    [[nodiscard]] bool SendInit(const std::string& resource_dir, const std::string& saved_image_dir, const bool public_timer_alert, const uint32_t bench, const uint32_t is_formal);

    [[nodiscard]] bool SendSetOption(const std::string& text);

    [[nodiscard]] bool SendStart(Match& match, const uint64_t match_id, const uint32_t user_num, const nlohmann::json& players_json);

    [[nodiscard]] ErrCode SendExecute(Match& match, const PlayerID player_id, const bool is_public, const std::string& text, MsgSender& reply);

    [[nodiscard]] bool SendLeave(Match& match, const PlayerID player_id);

    [[nodiscard]] bool FetchHelp(Match& match, const bool text_mode, std::string& text_out);

  private:
    [[nodiscard]] bool WriteJson(const nlohmann::json& j);
    void ApplyPost(Match& match, const nlohmann::json& j) const;
    void ApplyReplyItems(MsgSender& reply, const nlohmann::json& items) const;
    void ApplyPlayerState(Match& match, const nlohmann::json& j) const;
    void ApplyGameOver(Match& match, const nlohmann::json& j) const;

    std::unique_ptr<Subprocess> proc_;
    FILE* child_in_{nullptr};
    FILE* child_out_{nullptr};
};
