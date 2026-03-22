#include "match_process/child_session.h"

#include <stdexcept>

#include "match_process/json_frame.h"
#include "utility/log.h"
#include "nlohmann/json.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace {

class ErrCollector final : public MsgSenderBase
{
  public:
    void SetMatch(const Match* const) override {}

  protected:
    void SaveText(const char* const data, const uint64_t len) override { text_.append(data, data + len); }
    void SaveUser(const UserID&, const bool) override {}
    void SavePlayer(const PlayerID&, const bool) override {}
    void SaveImage(const char* const) override {}
    void SaveMarkdown(const char* const, const uint32_t) override {}
    void Flush() override {}

  public:
    std::string text_;
};

class ReplySender final : public MsgSenderBase
{
  public:
    explicit ReplySender(ChildGameSession& session)
        : session_(session)
    {}

    void SetMatch(const Match* const) override {}

  protected:
    void SaveText(const char* const data, const uint64_t len) override
    {
        nlohmann::json it;
        it["text"] = std::string(data, data + len);
        items_.push_back(std::move(it));
    }

    void SaveUser(const UserID& id, const bool /*is_at*/) override
    {
        nlohmann::json it;
        it["user_id"] = id.GetStr();
        items_.push_back(std::move(it));
    }

    void SavePlayer(const PlayerID& id, const bool /*is_at*/) override
    {
        nlohmann::json it;
        it["at_player_id"] = id.Get();
        items_.push_back(std::move(it));
    }

    void SaveImage(const char* const path) override
    {
        nlohmann::json it;
        it["image_path"] = path;
        items_.push_back(std::move(it));
    }

    void SaveMarkdown(const char* const markdown, const uint32_t width) override
    {
        nlohmann::json it;
        it["markdown"] = markdown;
        it["markdown_width"] = width;
        items_.push_back(std::move(it));
    }

    void Flush() override
    {
        nlohmann::json j;
        j["op"] = "reply";
        j["items"] = items_;
        session_.SendJson(j);
        items_ = nlohmann::json::array();
    }

  public:
    void flush_out() { Flush(); }

  private:
    ChildGameSession& session_;
    nlohmann::json items_{nlohmann::json::array()};
};

static const char* StageErrToStr(const StageErrCode rc)
{
    switch (rc) {
    case StageErrCode::OK: return "ok";
    case StageErrCode::CHECKOUT: return "checkout";
    case StageErrCode::FAILED: return "failed";
    case StageErrCode::CONTINUE: return "continue";
    case StageErrCode::NOT_FOUND: return "not_found";
    default: return "unknown";
    }
}

} // namespace

ChildGameSession::ChildGameSession(FILE* const in, FILE* const out)
    : in_(in)
    , out_(out)
{}

ChildGameSession::~ChildGameSession()
{
#ifdef _WIN32
    if (module_.mod_) {
        FreeLibrary(module_.mod_);
    }
#else
    if (module_.mod_) {
        dlclose(module_.mod_);
    }
#endif
}

bool ChildGameSession::LoadModule(const std::string& lib_path, std::string& error_out)
{
#ifdef _WIN32
    const HMODULE mod = LoadLibraryA(lib_path.c_str());
#else
    void* const mod = dlopen(lib_path.c_str(), RTLD_NOW);
#endif
    if (!mod) {
#ifdef __linux__
        error_out = dlerror() ? dlerror() : "dlopen failed";
#elif defined(__APPLE__)
        error_out = dlerror() ? dlerror() : "dlopen failed";
#else
        error_out = "LoadLibrary failed";
#endif
        return false;
    }
    module_.mod_ = mod;
    const auto sym = [&mod, &error_out](const char* const name) -> void*
        {
#ifdef _WIN32
            void* const p = reinterpret_cast<void*>(GetProcAddress(mod, name));
#else
            void* const p = dlsym(mod, name);
#endif
            if (!p) {
                error_out = std::string("missing symbol ") + name;
            }
            return p;
        };
    error_out.clear();
    module_.alloc_opt_ = reinterpret_cast<GameHandle::game_options_allocator>(sym("NewGameOptions"));
    if (!error_out.empty()) {
        return false;
    }
    module_.del_opt_ = reinterpret_cast<GameHandle::game_options_deleter>(sym("DeleteGameOptions"));
    if (!error_out.empty()) {
        return false;
    }
    module_.alloc_stage_ = reinterpret_cast<GameHandle::main_stage_allocator>(sym("NewMainStage"));
    if (!error_out.empty()) {
        return false;
    }
    module_.del_stage_ = reinterpret_cast<GameHandle::main_stage_deleter>(sym("DeleteMainStage"));
    if (!error_out.empty()) {
        return false;
    }
    const auto get_info = reinterpret_cast<lgtbot::game::GameInfo (*)()>(sym("GetGameInfo"));
    if (!error_out.empty() || !get_info) {
        return false;
    }
    const lgtbot::game::GameInfo info = get_info();
    game_title_ = info.properties_ ? info.properties_->name_ : "game";
    return true;
}

void ChildGameSession::SendJson(const nlohmann::json& j)
{
    const std::lock_guard lock(write_mutex_);
    if (!WriteJsonFrame(out_, j.dump())) {
        ErrorLog() << "WriteJsonFrame failed";
    }
}

void ChildGameSession::SendGameOver()
{
    if (!main_stage_ || !env_) {
        return;
    }
    nlohmann::json j;
    j["op"] = "game_over";
    nlohmann::json scores = nlohmann::json::array();
    for (uint32_t i = 0; i < env_->PlayerCount(); ++i) {
        const PlayerID pid{i};
        nlohmann::json row;
        row["pid"] = i;
        row["score"] = main_stage_->PlayerScore(pid);
        nlohmann::json ach = nlohmann::json::array();
        const char* const* ap = main_stage_->VerdictateAchievements(pid);
        if (ap) {
            for (; *ap; ++ap) {
                ach.push_back(*ap);
            }
        }
        row["achievements"] = std::move(ach);
        scores.push_back(std::move(row));
    }
    j["scores"] = std::move(scores);
    SendJson(j);
}

void ChildGameSession::DrainAfterStageWork()
{
    SendGameOver();
    main_stage_.reset();
}

void ChildGameSession::Routine()
{
    if (!main_stage_ || !env_) {
        return;
    }
    if (main_stage_->IsOver()) {
        DrainAfterStageWork();
        return;
    }
    const uint64_t computer_num = env_->ComputerNum();
    uint64_t ok_count = 0;
    const uint32_t n = env_->PlayerCount();
    if (n == 0) {
        return;
    }
    for (uint64_t p = 0; !main_stage_->IsOver() && ok_count < computer_num; p = (p + 1) % n) {
        const uint32_t pi = static_cast<uint32_t>(p);
        if (!env_->IsComputerAt(pi)) {
            continue;
        }
        if (env_->IsEliminatedAt(pi) || StageErrCode::OK == main_stage_->HandleComputerAct(pi, false)) {
            ++ok_count;
        } else {
            ok_count = 0;
        }
    }
    if (main_stage_->IsOver()) {
        DrainAfterStageWork();
    }
}

bool ChildGameSession::HandleInit(const nlohmann::json& j, std::string& err)
{
    resource_dir_ = j.at("resource_dir").get<std::string>();
    saved_image_dir_ = j.at("saved_image_dir").get<std::string>();
    game_options_ = GameHandle::game_options_ptr(module_.alloc_opt_(), module_.del_opt_);
    if (!game_options_) {
        err = "NewGameOptions returned null";
        SendJson(nlohmann::json{{"op", "ack"}, {"ok", false}, {"err", err}});
        return false;
    }
    lgtbot::game::ImmutableGenericOptions imm{};
    imm.public_timer_alert_ = j.at("public_timer_alert").get<bool>();
    imm.resource_dir_ = resource_dir_.c_str();
    imm.saved_image_dir_ = saved_image_dir_.c_str();
    imm.user_num_ = 0;
    lgtbot::game::MutableGenericOptions mut{};
    mut.bench_computers_to_player_num_ = j.at("bench").get<uint32_t>();
    mut.is_formal_ = static_cast<uint8_t>(j.at("is_formal").get<uint32_t>());
    generic_options_ = lgtbot::game::GenericOptions(imm, mut);
    SendJson(nlohmann::json{{"op", "ack"}, {"ok", true}});
    return true;
}

bool ChildGameSession::HandleSetOption(const nlohmann::json& j, std::string& /*err*/)
{
    const std::string text = j.at("text").get<std::string>();
    const bool ok = game_options_ && game_options_->SetOption(text.c_str());
    SendJson(nlohmann::json{{"op", "ack"}, {"ok", ok}});
    return true;
}

bool ChildGameSession::HandleStart(const nlohmann::json& j, std::string& err)
{
    const auto plist = j.at("players");
    std::vector<std::string> names;
    std::vector<std::string> avatars;
    std::vector<bool> computer;
    names.reserve(plist.size());
    avatars.reserve(plist.size());
    computer.reserve(plist.size());
    for (const auto& p : plist) {
        const bool is_computer = p.at("computer").get<bool>();
        computer.push_back(is_computer);
        if (is_computer) {
            const auto cid = p.at("computer_id").get<uint32_t>();
            names.push_back("机器人" + std::to_string(cid) + "号");
            avatars.push_back("");
        } else {
            names.push_back(p.at("display_name").get<std::string>());
            avatars.push_back(p.value("avatar", std::string{}));
        }
    }
    env_ = std::make_unique<IpcMatchEnv>(*this);
    env_->SetMeta(j.at("match_id").get<uint64_t>(), game_title_, std::move(names), std::move(avatars), std::move(computer));

    lgtbot::game::ImmutableGenericOptions imm{};
    imm.public_timer_alert_ = generic_options_.public_timer_alert_;
    imm.user_num_ = j.at("user_num").get<uint32_t>();
    imm.resource_dir_ = resource_dir_.c_str();
    imm.saved_image_dir_ = saved_image_dir_.c_str();
    lgtbot::game::MutableGenericOptions mut{};
    mut.bench_computers_to_player_num_ = generic_options_.bench_computers_to_player_num_;
    mut.is_formal_ = generic_options_.is_formal_;
    generic_options_ = lgtbot::game::GenericOptions(imm, mut);

    ErrCollector err_reply;
    main_stage_ = GameHandle::main_stage_ptr(
            module_.alloc_stage_(&err_reply, game_options_.get(), &generic_options_, env_.get()),
            module_.del_stage_);
    if (!main_stage_) {
        err = err_reply.text_.empty() ? "NewMainStage failed" : err_reply.text_;
        SendJson(nlohmann::json{{"op", "ack"}, {"ok", false}, {"err", err}});
        env_.reset();
        return false;
    }
    main_stage_->HandleStageBegin();
    Routine();
    SendJson(nlohmann::json{{"op", "ack"}, {"ok", true}});
    return true;
}

bool ChildGameSession::HandleExecute(const nlohmann::json& j, std::string& /*err*/)
{
    if (!main_stage_) {
        SendJson(nlohmann::json{{"op", "result"}, {"stage", "failed"}});
        return true;
    }
    ReplySender rep(*this);
    const auto stage_rc = main_stage_->HandleRequest(
            j.at("text").get<std::string>().c_str(),
            j.at("player_id").get<uint32_t>(),
            j.at("is_public").get<bool>(),
            rep);
    rep.flush_out();
    Routine();
    SendJson(nlohmann::json{{"op", "result"}, {"stage", StageErrToStr(stage_rc)}});
    return true;
}

bool ChildGameSession::HandleLeave(const nlohmann::json& j, std::string& /*err*/)
{
    if (!main_stage_) {
        SendJson(nlohmann::json{{"op", "ack"}, {"ok", false}});
        return true;
    }
    main_stage_->HandleLeave(PlayerID{j.at("player_id").get<uint32_t>()});
    Routine();
    SendJson(nlohmann::json{{"op", "ack"}, {"ok", true}});
    return true;
}

bool ChildGameSession::HandleHelp(const nlohmann::json& j, std::string& /*err*/)
{
    const bool text_mode = j.value("text_mode", false);
    std::string out;
    if (main_stage_) {
        out += "## 阶段信息\n\n";
        out += main_stage_->StageInfoC();
        out += "\n\n";
    }
    out += "## 当前可使用的游戏命令\n\n### 查看信息\n1. 帮助";
    if (main_stage_) {
        out += main_stage_->CommandInfoC(text_mode);
    } else if (game_options_) {
        out += "\n\n ### 配置选项";
        out += game_options_->Info(true, !text_mode);
    }
    SendJson(nlohmann::json{{"op", "help_text"}, {"text", std::move(out)}, {"text_mode", text_mode}});
    return true;
}

int ChildGameSession::RunLoop()
{
    for (;;) {
        std::string raw;
        if (!ReadJsonFrame(in_, raw)) {
            return 0;
        }
        nlohmann::json j;
        try {
            j = nlohmann::json::parse(raw);
        } catch (...) {
            SendJson(nlohmann::json{{"op", "error"}, {"msg", "bad json"}});
            continue;
        }
        const std::string op = j.value("op", "");
        std::string err;
        if (op == "shutdown") {
            return 0;
        }
        if (op == "init") {
            HandleInit(j, err);
        } else if (op == "set_option") {
            HandleSetOption(j, err);
        } else if (op == "start") {
            HandleStart(j, err);
        } else if (op == "execute") {
            HandleExecute(j, err);
        } else if (op == "leave") {
            HandleLeave(j, err);
        } else if (op == "help") {
            HandleHelp(j, err);
        } else {
            SendJson(nlohmann::json{{"op", "error"}, {"msg", "unknown op"}});
        }
    }
}

