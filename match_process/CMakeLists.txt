cmake_minimum_required(VERSION 3.10)
project(MatchProcess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(MATCH_TRANSPORT
  ${CMAKE_CURRENT_SOURCE_DIR}/subprocess.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/json_frame.cc
)

add_library(lgtbot_match_parent STATIC
  ${MATCH_TRANSPORT}
  ${CMAKE_CURRENT_SOURCE_DIR}/match_child_client.cc
)
target_include_directories(lgtbot_match_parent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package(Threads REQUIRED)
target_link_libraries(lgtbot_match_parent PUBLIC Threads::Threads)

# Headers under bot_core/ pull utility/log.h which needs glog when WITH_GLOG is defined (parent adds -DWITH_GLOG).
if (WITH_GLOG)
  find_package(glog REQUIRED)
  target_link_libraries(lgtbot_match_parent PUBLIC glog::glog)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(lgtbot_match_parent PUBLIC ${CMAKE_DL_LIBS})
endif()

add_executable(match_game_runner
  ${CMAKE_CURRENT_SOURCE_DIR}/game_runner_main.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/child_session.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ipc_match_env.cc
  ${MATCH_TRANSPORT}
)
target_include_directories(match_game_runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(match_game_runner PRIVATE Threads::Threads)
if (WITH_GLOG)
  target_link_libraries(match_game_runner PRIVATE glog::glog)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(match_game_runner PRIVATE ${CMAKE_DL_LIBS})
endif()
