#!/usr/bin/env bash

# 【工具】校验「漫漫长夜」区块数据（map/block_data.h）：修改区块后、实装前运行，无错误后再构建
#
# ============================== 如何运行 ==============================
# 在 WSL / Linux 下执行（脚本自行定位仓库根目录，可在任意目录运行）：
#     bash games/long_night/tools/tool_block_validator.sh [build_dir] [json_file]
#
# 参数（均可省略）：
#     build_dir  存放校验工具的构建目录（相对仓库根目录），默认 build
#     json_file  要校验的外部 JSON 文件（如草稿本导出的区块数据），省略则校验内置的 block_data.h
#
# 示例：
#     bash games/long_night/tools/tool_block_validator.sh
#     bash games/long_night/tools/tool_block_validator.sh build_t ~/blocks.json
#
# 退出码：无错误为 0，有错误为 1。校验内容与手动运行方式见 src/tool_block_validator.cc 顶部注释。
# ====================================================================
set -euo pipefail

TOOL_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"    # games/long_night/tools
REPO="$(cd "${TOOL_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-build}"
JSON_FILE="${2:-}"

VALIDATOR="${REPO}/${BUILD_DIR}/tool_block_validator"
mkdir -p "${REPO}/${BUILD_DIR}"

echo "[1/2] 编译校验工具…"
g++ -std=c++23 -I"${REPO}" -I"${REPO}/third_party" -I"${REPO}/third_party/json/include" -O1 -DGAME_MODULE_NAME=long_night \
    -o "${VALIDATOR}" \
    "${TOOL_DIR}/src/tool_block_validator.cc" \
    "${REPO}/utility/html.cc"

echo "[2/2] 校验区块数据…"
if [[ -n "${JSON_FILE}" ]]; then
    "${VALIDATOR}" --json "${JSON_FILE}"
else
    "${VALIDATOR}"
fi
