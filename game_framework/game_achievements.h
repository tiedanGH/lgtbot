// Copyright (c) 2023-present, Chang Liu <github.com/slontia>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).

#ifndef GAME_MODULE_NAME
#error GAME_MODULE_NAME is not defined
#endif

#ifndef GAME_ACHIEVEMENT_FILENAME
#error GAME_ACHIEVEMENT_FILENAME is not defined
#endif

#ifdef ENUM_BEGIN
#ifdef ENUM_MEMBER
#ifdef ENUM_END

ENUM_BEGIN(Achievement)
#define EXTEND_ACHIEVEMENT(name, description) ENUM_MEMBER(Achievement, name)
#include GAME_ACHIEVEMENT_FILENAME
#undef EXTEND_ACHIEVEMENT
ENUM_END(Achievement)

#endif
#endif
#endif

#ifndef GAME_ACHIEVEMENTS_H
#define GAME_ACHIEVEMENTS_H

#include "game_framework/game_main.h"

#include <array>
#include <optional>
#include <map>
#include <bitset>

namespace lgtbot {

namespace game {

namespace GAME_MODULE_NAME {

// Achievement is a developer-defined enum class, so it should be put into GAME_MODULE_NAME namespace.

#define ENUM_FILE "../game_framework/game_achievements.h"
#include "../utility/extend_enum.h"

static const std::array<const GameAchievement, Achievement::Count()> k_achievements = {
#define EXTEND_ACHIEVEMENT(name, description) GameAchievement{.name_ = #name, .description_ = description},
#include GAME_ACHIEVEMENT_FILENAME
#undef EXTEND_ACHIEVEMENT
};

} // namespace GAME_MODULE_NAME

} // namespace game

} // namespace lgtbot

#endif
