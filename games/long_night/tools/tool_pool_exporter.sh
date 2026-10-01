#!/usr/bin/env bash

# 【工具】导出「漫漫长夜」全部区块模式与主题的区块池预览图
#
# ============================== 如何运行 ==============================
# 在 WSL / Linux 下执行（脚本自行定位仓库根目录，可在任意目录运行）：
#     bash games/long_night/tools/tool_pool_exporter.sh [build_dir] [texture] [out_dir]
#
# 参数（均可省略，路径均相对仓库根目录）：
#     build_dir  构建目录，默认 build（需已构建出其中的 markdown2image）
#     texture    材质，classic（默认）/ retro / fresh
#     out_dir    输出目录，默认 <build_dir>/long_night_pools
#
# 示例：
#     bash games/long_night/tools/tool_pool_exporter.sh
#     bash games/long_night/tools/tool_pool_exporter.sh build_t fresh
#     bash games/long_night/tools/tool_pool_exporter.sh build retro build/long_night_pools_retro
#
# 输出：<out_dir>/<预览标题>.png，共 7 张：经典、狂野+疯狂、幻变、按钮、陷阱、空旷、无声
#     如 《漫漫长夜》幻变模式轮换区块.png；非经典材质追加材质名，如 《漫漫长夜》幻变模式轮换区块（复古）.png
#     宽度与游戏内 SaveMarkdown 一致（1000，「狂野+疯狂」为 2000），由导出工具的渲染清单给出
#
# 手动分步运行方式见 src/tool_pool_exporter.cc 顶部注释。
# ====================================================================
set -euo pipefail

TOOL_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"    # games/long_night/tools
REPO="$(cd "${TOOL_DIR}/../../.." && pwd)"
BUILD_DIR="${1:-build}"
TEXTURE="${2:-classic}"
OUT_DIR="${3:-${BUILD_DIR}/long_night_pools}"

BUILD_ABS="${REPO}/${BUILD_DIR}"
RESOURCE_DIR="${BUILD_ABS}/plugins/long_night/resource/"
MD2IMG="${BUILD_ABS}/markdown2image"
EXPORTER="${BUILD_ABS}/tool_pool_exporter"
MD_DIR="${BUILD_ABS}/long_night_pools_md"

[[ -x "${MD2IMG}" ]] || { echo "[错误] 未找到 markdown2image：${MD2IMG}（请先构建）" >&2; exit 1; }

# 资源目录：优先用构建产物，缺失则回退到源码资源
if [[ ! -d "${RESOURCE_DIR}" ]]; then
    RESOURCE_DIR="${REPO}/games/long_night/resource/"
fi

echo "[1/3] 编译导出工具…"
g++ -std=c++23 -I"${REPO}" -I"${REPO}/third_party" -I"${REPO}/third_party/json/include" -O1 -DGAME_MODULE_NAME=long_night \
    -o "${EXPORTER}" \
    "${TOOL_DIR}/src/tool_pool_exporter.cc" \
    "${REPO}/utility/html.cc"

echo "[2/3] 生成区块池 HTML（材质：${TEXTURE}）…"
rm -rf "${MD_DIR}"
# 标准输出为渲染清单，每行「<渲染宽度> <md 路径>」
MANIFEST="$("${EXPORTER}" --resource "${RESOURCE_DIR}" --outdir "${MD_DIR}" --texture "${TEXTURE}")"

echo "[3/3] 渲染 PNG…"
rm -rf "${REPO}/${OUT_DIR}"
mkdir -p "${REPO}/${OUT_DIR}"
count=0
while read -r width md; do
    name="$(basename "${md}" .md)"
    "${MD2IMG}" --input "${md}" --output "${REPO}/${OUT_DIR}/${name}.png" \
        --width "${width}" --with_css=false >/dev/null 2>&1
    count=$((count + 1))
done <<< "${MANIFEST}"
rm -rf "${MD_DIR}"

echo "完成：${count} 张区块池预览图 → ${OUT_DIR}/"
