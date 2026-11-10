// Copyright (c) 2018-present, JiaQi Yu <github.com/tiedanGH>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).
//
// 【工具】导出「漫漫长夜」全部区块模式与主题的区块池预览图
//
// ============================== 如何运行 ==============================
//
// 【方式一（推荐）】一键脚本，自动完成编译 + 导出 + 渲染 PNG（参数说明见 tool_pool_exporter.sh）：
//     bash games/long_night/tools/tool_pool_exporter.sh                    # 默认 build / classic
//     bash games/long_night/tools/tool_pool_exporter.sh build fresh        # 清新材质
//     bash games/long_night/tools/tool_pool_exporter.sh build classic 输出目录
//   输出：<输出目录>/《漫漫长夜》<模式>模式<类型>区块[（材质名）].png，默认为 build/long_night_pools/
//   前置条件：已构建出 <build_dir>/markdown2image
//
// 【方式二】手动分步（本文件只负责生成 HTML，渲染交给 markdown2image），以下命令均在仓库根目录执行：
//   1. 编译
//        g++ -std=c++23 -I. -Ithird_party -Ithird_party/json/include -O1 -DGAME_MODULE_NAME=long_night \
//            -o build/tool_pool_exporter \
//            games/long_night/tools/src/tool_pool_exporter.cc utility/html.cc
//   2. 生成各区块池的 HTML，标准输出为渲染清单（每行「<渲染宽度> <md 路径>」）
//        ./build/tool_pool_exporter \
//            --resource "$(pwd)/build/plugins/long_night/resource/" \
//            --outdir   ./build/pools_md \
//            --texture  classic > build/pools_md.list
//      可选参数：--texture classic（默认）/ retro / fresh
//   3. 按清单逐个渲染为 PNG
//        mkdir -p build/long_night_pools
//        while read -r w f; do
//            ./build/markdown2image --input "$f" \
//                --output "build/long_night_pools/$(basename "$f" .md).png" \
//                --width "$w" --with_css=false
//        done < build/pools_md.list
//
// 【导出内容】共 7 张，与游戏内 GetAllBlocksInfo 的区块池预览一致：
//   经典、狂野+疯狂（全部区块，含特殊区块列表）、幻变、按钮、陷阱、空旷、无声
// 【命名规则】文件名即预览标题，如 《漫漫长夜》幻变模式轮换区块.png；
//   使用非经典材质时追加材质名，如 《漫漫长夜》狂野+疯狂模式全部区块（复古）.png
// 【关于图片尺寸】与游戏内 SaveMarkdown 一致：每行 4 个区块，宽 (50+10)*16+40=1000；
//   「狂野+疯狂」每行 8 个区块，宽度翻倍为 2000。--with_css=false 等同游戏内渲染的 --nowith_css。
// ====================================================================

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <regex>
#include <span>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "bot_core/id.h"
#include "bot_core/msg_sender.h"   // ChildMsgSenderBase
#include "utility/html.h"

// boss.h 回合结算模板用到的框架宏（原定义于 utility/extend_option.h，无法单独包含）
#ifndef GET_OPTION_VALUE
#define GET_OPTION_VALUE(option, name) (option).template Option2Value<std::decay_t<decltype(option)>::OPTION_##name>()
#endif

using namespace std;

#include "games/long_night/common/constants.h"
#include "games/long_night/actor/player.h"
#include "games/long_night/map/grid.h"
#include "games/long_night/map/map.h"
#include "games/long_night/actor/boss.h"
#include "games/long_night/map/board.h"

namespace {

string Arg(int argc, char** argv, const string& key, const string& fallback)
{
    for (int i = 1; i + 1 < argc; i++) {
        if (argv[i] == key) return argv[i + 1];
    }
    return fallback;
}

} // namespace

int main(int argc, char** argv)
{
    const string resource_dir = Arg(argc, argv, "--resource", "./resource/");
    const string out_dir = Arg(argc, argv, "--outdir", "./pool_md/");
    const string texture_name = Arg(argc, argv, "--texture", "classic");
    // 材质 + 文件名后缀：非经典材质追加「（材质名）」
    struct TextureInfo { Texture texture; string suffix; };
    const map<string, TextureInfo> texture_map = {
        {"classic", {Texture::CLASSIC, ""}}, {"retro", {Texture::RETRO, "（复古）"}}, {"fresh", {Texture::FRESH, "（清新）"}},
    };
    const auto texture_it = texture_map.find(texture_name);
    if (texture_it == texture_map.end()) {
        cerr << "[错误] 未知材质：" << texture_name << "（可选：classic / retro / fresh）" << endl;
        return 1;
    }
    const auto& [texture, name_suffix] = texture_it->second;

    filesystem::create_directories(out_dir);

    // 与游戏内 SaveMarkdown 的宽度一致：每行 4 个区块；「狂野+疯狂」每行 8 个，宽度翻倍
    const int pool_width = (GRID_SIZE + WALL_SIZE) * 16 + 40;

    struct Pool { string name; BlockMode mode; };
    const vector<Pool> pools = {
        {"《漫漫长夜》经典模式区块",          BlockMode::CLASSIC},
        {"《漫漫长夜》狂野+疯狂模式全部区块", BlockMode::CRAZY},
        {"《漫漫长夜》幻变模式轮换区块",      BlockMode::TWIST},
        {"《漫漫长夜》按钮模式主题区块",      BlockMode::BUTTON},
        {"《漫漫长夜》陷阱模式主题区块",      BlockMode::TRAP},
        {"《漫漫长夜》空旷模式主题区块",      BlockMode::OPEN},
        {"《漫漫长夜》无声模式主题区块",      BlockMode::SILENT},
    };

    int ok = 0;
    for (const auto& pool : pools) {
        // 按对应模式构造 Board：构造时即按游戏内逻辑载入该模式的区块池
        const Board board(resource_dir, texture, pool.mode, {}, false);
        const string html = board.GetAllBlocksInfo(SpecialEvent::NONE, true, pool.mode);
        const int width = pool.mode == BlockMode::CRAZY ? pool_width * 2 : pool_width;

        const string path = out_dir + "/" + pool.name + name_suffix + ".md";
        ofstream ofs(path, ios::binary);
        if (!ofs) {
            cerr << "[错误] 无法写入：" << path << endl;
            continue;
        }
        ofs << html;
        ofs.close();
        cout << width << " " << path << endl;     // 渲染清单：<渲染宽度> <md 路径>
        ok++;
    }
    cerr << "共导出 " << ok << " / " << pools.size() << " 个区块池预览（材质：" << texture_name << "）" << endl;
    return ok == static_cast<int>(pools.size()) ? 0 : 1;
}
