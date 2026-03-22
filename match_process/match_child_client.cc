#include "match_process/match_child_client.h"

#include "bot_core/bot_core.h"
#include "bot_core/match.h"
#include "bot_core/msg_sender.h"
#include "match_process/json_frame.h"
#include "match_process/subprocess.h"
#include "nlohmann/json.hpp"
#include "utility/log.h"

namespace {

ErrCode StageStrToErr(const std::string_view s)
{
    if (s == "ok") {
        return EC_GAME_REQUEST_OK;
    }
    if (s == "checkout") {
        return EC_GAME_REQUEST_CHECKOUT;
    }
    if (s == "failed") {
        return EC_GAME_REQUEST_FAILED;
    }
    if (s == "continue") {
        return EC_GAME_REQUEST_CONTINUE;
    }
    if (s == "not_found") {
        return EC_GAME_REQUEST_NOT_FOUND;
    }
    return EC_GAME_REQUEST_UNKNOWN;
}

} // namespace

MatchChildClient::MatchChildClient(const std::filesystem::path runner_exe, const std::filesystem::path game_library)
{
    std::string spawn_err;
    std::vector<std::string> argv;
    argv.push_back(runner_exe.string());
    argv.push_back(game_library.string());
    proc_ = std::make_unique<Subprocess>(std::move(argv), spawn_err);
    if (!proc_->ok()) {
        ErrorLog() << "MatchChildClient spawn failed: " << spawn_err;
        proc_.reset();
        return;
    }
    child_in_ = proc_->child_stdin();
    child_out_ = proc_->child_stdout();
}

MatchChildClient::~MatchChildClient() = default;

bool MatchChildClient::WriteJson(const nlohmann::json& j)
{
    if (!child_in_) {
        return false;
    }
    return WriteJsonFrame(child_in_, j.dump());
}

bool MatchChildClient::SendInit(const std::string& resource_dir, const std::string& saved_image_dir, const bool public_timer_alert, const uint32_t bench, const uint32_t is_formal)
{
    nlohmann::json j;
    j["op"] = "init";
    j["resource_dir"] = resource_dir;
    j["saved_image_dir"] = saved_image_dir;
    j["public_timer_alert"] = public_timer_alert;
    j["bench"] = bench;
    j["is_formal"] = is_formal;
    if (!WriteJson(j)) {
        return false;
    }
    std::string raw;
    if (!ReadJsonFrame(child_out_, raw)) {
        return false;
    }
    const auto resp = nlohmann::json::parse(raw, nullptr, false);
    return resp.is_object() && resp.value("ok", false);
}

bool MatchChildClient::SendSetOption(const std::string& text)
{
    if (!WriteJson(nlohmann::json{{"op", "set_option"}, {"text", text}})) {
        return false;
    }
    std::string raw;
    if (!ReadJsonFrame(child_out_, raw)) {
        return false;
    }
    const auto resp = nlohmann::json::parse(raw, nullptr, false);
    return resp.is_object() && resp.value("ok", false);
}

bool MatchChildClient::SendStart(Match& /*match*/, const uint64_t match_id, const uint32_t user_num, const nlohmann::json& players_json)
{
    nlohmann::json j;
    j["op"] = "start";
    j["match_id"] = match_id;
    j["user_num"] = user_num;
    j["players"] = players_json;
    if (!WriteJson(j)) {
        return false;
    }
    std::string raw;
    if (!ReadJsonFrame(child_out_, raw)) {
        return false;
    }
    const auto resp = nlohmann::json::parse(raw, nullptr, false);
    return resp.is_object() && resp.value("ok", false);
}

void MatchChildClient::ApplyPost(Match& match, const nlohmann::json& j) const
{
    const std::string channel = j.at("channel").get<std::string>();
    const auto items = j.at("items");
    MsgSenderBase::MsgSenderGuard sender = [&]() -> MsgSenderBase::MsgSenderGuard
        {
            if (channel == "broadcast") {
                return match.Boardcast();
            }
            if (channel == "group") {
                return match.GroupMsgSender()();
            }
            if (channel == "ai") {
                return match.BoardcastAiInfo();
            }
            if (channel == "tell") {
                return match.Tell(PlayerID{j.at("target_pid").get<uint32_t>()});
            }
            return match.Boardcast();
        }();
    for (const auto& it : items) {
        if (it.contains("text")) {
            sender << it.at("text").get<std::string>();
        }
        if (it.contains("at_player_id")) {
            sender << At(PlayerID{it.at("at_player_id").get<uint32_t>()});
        }
        if (it.contains("user_id")) {
            sender << Name(UserID{it.at("user_id").get<std::string>()});
        }
        if (it.contains("image_path")) {
            sender << Image{it.at("image_path").get<std::string>()};
        }
        if (it.contains("markdown")) {
            const auto w = it.value("markdown_width", 600u);
            sender << Markdown{it.at("markdown").get<std::string>(), w};
        }
    }
}

void MatchChildClient::ApplyReplyItems(MsgSender& reply, const nlohmann::json& items) const
{
    auto g = reply();
    for (const auto& it : items) {
        if (it.contains("text")) {
            g << it.at("text").get<std::string>();
        }
        if (it.contains("at_player_id")) {
            g << At(PlayerID{it.at("at_player_id").get<uint32_t>()});
        }
        if (it.contains("user_id")) {
            g << Name(UserID{it.at("user_id").get<std::string>()});
        }
        if (it.contains("image_path")) {
            g << Image{it.at("image_path").get<std::string>()};
        }
        if (it.contains("markdown")) {
            const auto w = it.value("markdown_width", 600u);
            g << Markdown{it.at("markdown").get<std::string>(), w};
        }
    }
}

void MatchChildClient::ApplyPlayerState(Match& match, const nlohmann::json& j) const
{
    match.ApplyChildPlayerState(PlayerID{j.at("pid").get<uint32_t>()}, j.at("state").get<std::string>());
}

void MatchChildClient::ApplyGameOver(Match& match, const nlohmann::json& j) const
{
    match.ApplyChildGameOverFromScores(j.at("scores").dump());
}

ErrCode MatchChildClient::SendExecute(Match& match, const PlayerID player_id, const bool is_public, const std::string& text, MsgSender& reply)
{
    if (!WriteJson(nlohmann::json{
                {"op", "execute"},
                {"text", text},
                {"player_id", player_id.Get()},
                {"is_public", is_public},
        })) {
        return EC_MATCH_UNEXPECTED_CONFIG;
    }
    for (;;) {
        std::string raw;
        if (!ReadJsonFrame(child_out_, raw)) {
            return EC_MATCH_UNEXPECTED_CONFIG;
        }
        const auto frame = nlohmann::json::parse(raw, nullptr, false);
        if (!frame.is_object()) {
            continue;
        }
        const std::string op = frame.value("op", "");
        if (op == "post") {
            ApplyPost(match, frame);
        } else if (op == "reply") {
            ApplyReplyItems(reply, frame.at("items"));
        } else if (op == "player_state") {
            ApplyPlayerState(match, frame);
        } else if (op == "game_over") {
            ApplyGameOver(match, frame);
        } else if (op == "result") {
            const ErrCode er = StageStrToErr(frame.at("stage").get<std::string>());
            match.ReleaseGameChildIfOver();
            return er;
        }
    }
}

bool MatchChildClient::SendLeave(const PlayerID player_id)
{
    if (!WriteJson(nlohmann::json{{"op", "leave"}, {"player_id", player_id.Get()}})) {
        return false;
    }
    std::string raw;
    if (!ReadJsonFrame(child_out_, raw)) {
        return false;
    }
    const auto resp = nlohmann::json::parse(raw, nullptr, false);
    return resp.is_object() && resp.value("ok", false);
}

bool MatchChildClient::FetchHelp(Match& match, const bool text_mode, std::string& text_out)
{
    if (!WriteJson(nlohmann::json{{"op", "help"}, {"text_mode", text_mode}})) {
        return false;
    }
    for (;;) {
        std::string raw;
        if (!ReadJsonFrame(child_out_, raw)) {
            return false;
        }
        const auto frame = nlohmann::json::parse(raw, nullptr, false);
        if (!frame.is_object()) {
            continue;
        }
        const std::string op = frame.value("op", "");
        if (op == "post") {
            ApplyPost(match, frame);
        } else if (op == "help_text") {
            text_out = frame.at("text").get<std::string>();
            return true;
        }
    }
}
