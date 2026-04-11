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

  # markdown2image: default to Qt5 WebKit (e.g. RHEL 8); auto-fall back to Qt6 WebEngine when Qt5 WebKit is missing.
  option(MARKDOWN2IMAGE_USE_QT5WEBKIT
    "Prefer Qt5 WebKit for markdown2image; if unavailable, use Qt6 WebEngine when installed"
    ON)

  find_package(Qt5 QUIET COMPONENTS Gui Widgets WebKit WebKitWidgets)
  find_package(Qt6 QUIET COMPONENTS Gui Widgets WebEngineWidgets)

  set(_md2img_qt5_ok FALSE)
  if (TARGET Qt5::WebKitWidgets AND TARGET Qt5::WebKit AND TARGET Qt5::Widgets AND TARGET Qt5::Gui)
    set(_md2img_qt5_ok TRUE)
  endif()
  set(_md2img_qt6_ok FALSE)
  if (TARGET Qt6::WebEngineWidgets AND TARGET Qt6::Widgets AND TARGET Qt6::Gui)
    set(_md2img_qt6_ok TRUE)
  endif()

  if (MARKDOWN2IMAGE_USE_QT5WEBKIT)
    if (_md2img_qt5_ok)
      set(_md2img_use_qt5 TRUE)
    elseif (_md2img_qt6_ok)
      set(_md2img_use_qt5 FALSE)
      message(STATUS "markdown2image: Qt5 WebKit not found; using Qt6 WebEngine instead")
    else()
      message(FATAL_ERROR "WITH_IMAGE=ON requires Qt5 WebKit (qt5-qtwebkit + devel) and/or Qt6 WebEngine. Install one of them, or configure with -DWITH_IMAGE=OFF.")
    endif()
  else()
    if (_md2img_qt6_ok)
      set(_md2img_use_qt5 FALSE)
    elseif (_md2img_qt5_ok)
      set(_md2img_use_qt5 TRUE)
      message(STATUS "markdown2image: Qt6 WebEngine not found; using Qt5 WebKit")
    else()
      message(FATAL_ERROR "WITH_IMAGE=ON requires Qt6 WebEngine and/or Qt5 WebKit. Install one of them, or configure with -DWITH_IMAGE=OFF.")
    endif()
  endif()

  set(USE_QT5WEBKIT "${_md2img_use_qt5}" CACHE BOOL "Forwarded to markdown2image: use Qt5 WebKit backend" FORCE)
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
