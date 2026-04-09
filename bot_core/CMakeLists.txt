cmake_minimum_required(VERSION 3.0)
project(BotCore LANGUAGES CXX C)

set(CMAKE_SYSTEM_VERSION 1)

# Default build with Debug
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 20)

find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

# Generate protobuf sources for the IPC protocol
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
  ${CMAKE_CURRENT_SOURCE_DIR}/../match_process/match_ipc.proto
)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -latomic")
endif()

# generate version info
EXECUTE_PROCESS(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE MAGE_VERSION_GIT_HEAD_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE GET_GIT_VERSION_FAILED
)
if (GET_GIT_VERSION_FAILED)
    message("-- Get Git Describe failed, will sign unpublished version: ${GET_GIT_VERSION_FAILED}")
    set(MAKE_VERSiON_GIT_HEAD_VERSION "<unpublished version>")
else()
    message("-- Current Git Describe: ${MAGE_VERSION_GIT_HEAD_VERSION}")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.cc.in" "${CMAKE_CURRENT_BINARY_DIR}/version.cc")
endif()

list(APPEND SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/bot_core.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bot_ctx.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/db_manager.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/match.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/match_manager.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/message_handlers.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/msg_sender.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/score_calculation.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../utility/html.cc
  ${CMAKE_CURRENT_BINARY_DIR}/version.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/match_child_client.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/game_config_client.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/subprocess.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/../match_process/ipc_frame.cc
  ${PROTO_SRCS}
)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  list(APPEND SOURCE_FLIES ${CMAKE_CURRENT_SOURCE_DIR}/dllmain.cc)
endif()

find_package(Threads REQUIRED)
list(APPEND THIRD_PARTIES Threads::Threads protobuf::libprotobuf
  absl::log absl::log_internal_check_op absl::strings absl::base)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  list(APPEND THIRD_PARTIES
    dl      # load shared libraries dynamicly on linux
    atomic
  )
endif()

find_package(gflags REQUIRED)
list(APPEND THIRD_PARTIES gflags)

add_library(bot_core_static STATIC ${SOURCE_FILES})
target_link_libraries(bot_core_static ${THIRD_PARTIES})
target_include_directories(bot_core_static PRIVATE ${CMAKE_BINARY_DIR})
add_dependencies(bot_core_static match_game_runner config_runner)
target_compile_definitions(bot_core_static PRIVATE
  MATCH_GAME_RUNNER_PATH="$<TARGET_FILE:match_game_runner>"
  CONFIG_RUNNER_PATH="$<TARGET_FILE:config_runner>"
)

add_library(bot_core SHARED ${SOURCE_FILES})
target_link_libraries(bot_core ${THIRD_PARTIES})
target_include_directories(bot_core PRIVATE ${CMAKE_BINARY_DIR})
add_dependencies(bot_core match_game_runner config_runner)
target_compile_definitions(bot_core PRIVATE
  MATCH_GAME_RUNNER_PATH="$<TARGET_FILE:match_game_runner>"
  CONFIG_RUNNER_PATH="$<TARGET_FILE:config_runner>"
)

if (WITH_TEST)
  enable_testing()
  find_package(GTest REQUIRED)
  list(APPEND THIRD_PARTIES GTest::GTest GTest::Main)

  # Build test_game plugin (dylib) for test_bot
  set(TEST_GAME_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_game/mygame.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../game_framework/game_main.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../game_framework/resource_loader.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../game_framework/stage.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../game_framework/stage_utility.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../game_framework/player_ready_masker.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../utility/html.cc
  )
  # test_game_plugin is placed in a dedicated directory (test_game_plugins/test_game/)
  # so that test_bot loads ONLY this plugin, avoiding symbol conflicts with other game plugins.
  set(TEST_GAME_PLUGIN_PARENT_DIR ${CMAKE_BINARY_DIR}/test_game_plugins)
  set(TEST_GAME_PLUGIN_DIR ${TEST_GAME_PLUGIN_PARENT_DIR}/test_game)
  add_custom_target(make_test_game_plugin_dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TEST_GAME_PLUGIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/test_game/rule.md ${TEST_GAME_PLUGIN_DIR}/rule.md)
  add_library(test_game_plugin SHARED ${TEST_GAME_SOURCE_FILES})
  add_dependencies(test_game_plugin make_test_game_plugin_dir)
  target_compile_definitions(test_game_plugin PUBLIC
    GAME_ACHIEVEMENT_FILENAME="${CMAKE_CURRENT_SOURCE_DIR}/test_game/achievements.h"
    GAME_OPTION_FILENAME="${CMAKE_CURRENT_SOURCE_DIR}/test_game/options.h"
    GAME_MODULE_NAME=test_game
    TEST_BOT)
  target_include_directories(test_game_plugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/test_game
    ${CMAKE_CURRENT_SOURCE_DIR}/..)
  if (WITH_GLOG)
    target_link_libraries(test_game_plugin glog::glog)
  endif()
  set_target_properties(test_game_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${TEST_GAME_PLUGIN_DIR}
    OUTPUT_NAME "libgame"
    PREFIX "")

  add_executable(test_bot
    test_bot.cc
    ${SOURCE_FILES})
  target_link_libraries(test_bot ${THIRD_PARTIES})
  target_include_directories(test_bot PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR})
  add_dependencies(test_bot match_game_runner config_runner test_game_plugin)
  target_compile_definitions(test_bot PRIVATE
    MATCH_GAME_RUNNER_PATH="$<TARGET_FILE:match_game_runner>"
    CONFIG_RUNNER_PATH="$<TARGET_FILE:config_runner>"
    TEST_GAME_PLUGIN_DIR="${TEST_GAME_PLUGIN_PARENT_DIR}"
    TEST_BOT)
  add_test(NAME test_bot COMMAND test_bot)

  # Build test_game unit test (game-level, no IPC, tests timer/checkout/score logic directly)
  add_library(test_game_for_test_lib OBJECT ${TEST_GAME_SOURCE_FILES})
  target_compile_definitions(test_game_for_test_lib PUBLIC
    GAME_ACHIEVEMENT_FILENAME="${CMAKE_CURRENT_SOURCE_DIR}/test_game/achievements.h"
    GAME_OPTION_FILENAME="${CMAKE_CURRENT_SOURCE_DIR}/test_game/options.h"
    GAME_MODULE_NAME=test_game
    TEST_BOT)
  target_include_directories(test_game_for_test_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/test_game
    ${CMAKE_CURRENT_SOURCE_DIR}/..)
  if (WITH_GLOG)
    target_link_libraries(test_game_for_test_lib glog::glog)
  endif()

  add_executable(test_test_game
    ${CMAKE_CURRENT_SOURCE_DIR}/test_game/unittest.cc
    $<TARGET_OBJECTS:test_game_for_test_lib>)
  target_compile_definitions(test_test_game PUBLIC
    GAME_ACHIEVEMENT_FILENAME="${CMAKE_CURRENT_SOURCE_DIR}/test_game/achievements.h"
    GAME_OPTION_FILENAME="${CMAKE_CURRENT_SOURCE_DIR}/test_game/options.h"
    GAME_MODULE_NAME=test_game
    TEST_BOT)
  target_include_directories(test_test_game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/test_game
    ${CMAKE_CURRENT_SOURCE_DIR}/..)
  target_link_libraries(test_test_game GTest::GTest GTest::Main gflags)
  if (WITH_GLOG)
    target_link_libraries(test_test_game glog::glog)
  endif()
  add_test(NAME test_test_game COMMAND test_test_game)

  add_executable(test_db test_db.cc db_manager.cc score_calculation.cc)
  target_link_libraries(test_db ${THIRD_PARTIES})
  add_test(NAME test_db COMMAND test_db)

  add_executable(test_match_score test_match_score.cc score_calculation.cc)
  target_link_libraries(test_match_score ${THIRD_PARTIES})
  add_test(NAME test_match_score COMMAND test_match_score)
endif()

