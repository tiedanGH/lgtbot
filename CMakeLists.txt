cmake_minimum_required(VERSION 3.0)
project(BotCore LANGUAGES CXX C)

set(CMAKE_SYSTEM_VERSION 1)

# Default build with Debug
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

option(WITH_GCOV "bulid with gcov" FALSE)
option(WITH_ASAN "build with address sanitizier" FALSE)
option(WITH_TEST "build with unittest" TRUE)
option(WITH_CORE "build with bot core" TRUE)
option(WITH_TOOLS "build with tools" TRUE)
option(WITH_GAMES "build with games" TRUE)
option(WITH_IMAGE "allow bot print image" TRUE)
option(WITH_GLOG "build with glog" TRUE)
option(WITH_SQLITE "build with sqlite" TRUE)

# 'char' type in arm machines is 'unsigned char', we should define it as 'signed char'
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsigned-char -g -Wno-invalid-utf8 -Wno-c99-designator")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsigned-char -g -Wno-invalid-utf8")

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json/include)

if (WITH_IMAGE)
  find_package(gflags REQUIRED)
  add_subdirectory(third_party/markdown2image)
endif()

if (WITH_GCOV)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fprofile-arcs -ftest-coverage -lgcov --coverage")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -fprofile-arcs -ftest-coverage -lgcov --coverage")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
endif()

if (WITH_ASAN)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
endif()

if (WITH_SQLITE)
  add_definitions(-DWITH_SQLITE)
  find_package(SQLite3 REQUIRED)
  list(APPEND THIRD_PARTIES SQLite::SQLite3)
  include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite_modern_cpp/hdr)
endif()

if (WITH_GLOG)
  find_package(glog REQUIRED)
  list(APPEND THIRD_PARTIES glog::glog)
  add_definitions(-DWITH_GLOG)
  get_target_property(GLOG_INCLUDE_DIRS glog::glog INTERFACE_INCLUDE_DIRECTORIES)
  include_directories(${GLOG_INCLUDE_DIRS})
  get_target_property(GLOG_COMPILE_DEFS glog::glog INTERFACE_COMPILE_DEFINITIONS)
  add_definitions(-DGLOG_USE_GLOG_EXPORT)
endif()

if (WITH_TEST)
  enable_testing()
endif()

if (WITH_CORE)
  add_subdirectory(match_process)
  add_subdirectory(bot_core)
endif()

if (WITH_TOOLS)
  add_subdirectory(tools)
endif()

if (WITH_GAMES)
  add_subdirectory(third_party/necessary-and-unnecessary-tiles)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/necessary-and-unnecessary-tiles)
  find_package(pybind11 QUIET)
  if(pybind11_FOUND)
    include_directories(${pybind11_INCLUDE_DIRS})
    find_package(Python3 COMPONENTS Development QUIET)
    if(NOT Python3_FOUND)
      set(pybind11_FOUND FALSE)
      message(STATUS "Python3 development libraries not found, skipping MahjongPy")
    else()
      set(PYTHON_LIBRARIES ${Python3_LIBRARIES})
      set(PYTHON_INCLUDE_DIRS ${Python3_INCLUDE_DIRS})
      if(APPLE)
        # Python framework on macOS does not export all symbols; resolve at runtime
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
      endif()
    endif()
  endif()
  add_subdirectory(third_party/mahjong)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/mahjong)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/tiny_expr)
  add_library(tinyexpr SHARED ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tiny_expr/tinyexpr.c)
  add_subdirectory(games)
endif()

if (WITH_TEST)
  add_subdirectory(game_util) # only build for unittest
  add_subdirectory(utility) # only build for unittest
endif()
