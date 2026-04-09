cmake_minimum_required(VERSION 3.10)
project(MatchProcess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Protobuf REQUIRED)

# Generate .pb.h / .pb.cc from the IPC proto
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
  ${CMAKE_CURRENT_SOURCE_DIR}/match_ipc.proto
)

set(CHILD_TRANSPORT
  ${CMAKE_CURRENT_SOURCE_DIR}/ipc_frame.cc
  ${PROTO_SRCS}
)

find_package(Threads REQUIRED)
find_package(absl REQUIRED)

add_executable(match_game_runner
  ${CMAKE_CURRENT_SOURCE_DIR}/game_runner_main.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/child_session.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ipc_match_env.cc
  ${CHILD_TRANSPORT}
)
target_include_directories(match_game_runner PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_BINARY_DIR}
)
target_link_libraries(match_game_runner PRIVATE Threads::Threads protobuf::libprotobuf
  absl::log absl::log_internal_check_op absl::strings absl::base)
if (WITH_GLOG)
  target_link_libraries(match_game_runner PRIVATE glog::glog)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(match_game_runner PRIVATE ${CMAKE_DL_LIBS})
endif()

add_executable(config_runner
  ${CMAKE_CURRENT_SOURCE_DIR}/config_runner_main.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/child_config_session.cc
  ${CHILD_TRANSPORT}
)
target_include_directories(config_runner PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_BINARY_DIR}
)
target_link_libraries(config_runner PRIVATE Threads::Threads protobuf::libprotobuf
  absl::log absl::log_internal_check_op absl::strings absl::base)
if (WITH_GLOG)
  target_link_libraries(config_runner PRIVATE glog::glog)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(config_runner PRIVATE ${CMAKE_DL_LIBS})
endif()
