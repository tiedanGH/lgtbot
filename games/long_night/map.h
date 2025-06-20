// 感谢 BC 提供的区块初始化思路
class UnitMaps {
  public:
    // 撞墙提示
    static constexpr const array<string_view, 30> wall_hints = {
        "砰！你狠狠地撞在了一堵墙上！",     // 铁蛋
        "一阵剧痛传来，你撞上了一堵墙，看来这里走不通。",
        "黑暗中，你的身体撞击了一面粗糙的墙壁。",
        "你听到一声沉闷的回响——是墙壁挡住了你的去路。",
        "墙壁在你面前横亘，仿佛无情地阻挠着前行的路。",
        "墙壁冷漠地矗立在你面前，拒绝让你通过。",
        "前方一堵高墙挡住了你的去路。",
        "你试图继续前进，但一堵墙挡住了你的去路。",
        "砰！你撞上了一堵墙，幸好没人看到。",
        "你决定挑战墙壁，结果墙壁胜利了。",
        "看来你的“穿墙术”还没练成。",
        "你试图与墙壁讲道理，但它完全不想搭理你。",
        "你向前冲去，然后优雅地和墙壁来了个亲密接触。",
        "你试图用意念穿过墙壁，然而它比你的意念还坚定。",
        "黑暗中，你的手掌摸到一堵砖墙上，你期待的邂逅没有发生。",   //  大萝卜姬
        "噢是墙壁，你情不自禁地把耳朵贴了上去，你很失望。",
        "是一面光滑湿软的墙！快趁机误导一下对手！信我的邪，你发出了只能自己听见的啪啪声。",
        "你的手掌按在冰冷的墙面上，它仿佛正在吞噬你的温度。你是南方人啊，试试舌头吧！",
        "彭！靠北啦，你不看路的吗？拜托~这么大一面墙你就这样撞啊。",
        "砰！你撞到了墙上。看来你数错了，9又3/4并不是那么好找的。",
        "你似乎走到了墙面上？快醒醒，这里并不是匹诺康尼，别做白日梦了。",   // 三月七
        "这是什么？墙壁？撞一下。\n这是什么？墙壁？撞一下。\n这是什么？墙壁？撞一下。",
        "你申请对墙壁过一个说服，dm拒绝了你。",
        "你尝试使用闪现过墙，但可惜墙体的厚度超出了你的预期。",
        "墙壁温柔的注视着你，不再言语。",
        "仿生铁蛋bot会梦到电子萝卜吗？至少墙壁不会给你答案。",
        "你在平原上走着，突然迎面遇到一堵墙，这墙向上无限高，向下无限深，向左无限远，向右无限远，这墙是什么？\n当然不可能有这样的墙，无论材质是什么，都会因为无限大的重力坍缩的。这只是那些神经兮兮的糟老头子的臆想罢了。\n不过，你确实撞在了一堵墙上。",   // H3PO4
        "黑暗中突然出现的墙壁，像是命运在说：换个方向试试？",   // 月影
        "你试图用脸测量墙壁的硬度，恭喜获得物理系荣誉学位！",
        "砰！你与墙壁进行了深入交流，结论是它比你想象的更固执。",
    };
    // 第一步撞墙提示
    static constexpr const array<string_view, 4> firststep_wall_hints = {
        "这是什么？墙壁？撞一下。\n这是什么？墙壁？撞一下。\n这是什么？墙壁？撞一下。",     // 三月七
        "你知道吗，撞击同一面墙114514次即可将其撞倒！",
        "看来有人认为自己在玩多层迷宫……别想了，这是永久墙壁。",
        "俗话说不撞南墙不回头，但有人撞了南墙也不回头。",
    };
    // 树丛提示
    static constexpr const array<string_view, 12> grass_hints = {
        "你踏入一片树丛，枯叶和树枝在脚下沙沙作响。",   // 铁蛋
        "你一脚踏入了一片树丛，树叶发出了沙沙声，仿佛某种回应。",
        "树叶微微颤动，沙沙声仿佛在轻声低语。",
        "脚下传出“沙沙”的声音……你希望这只是树叶，而不是别的东西。",
        "你踏入一片树丛，沙沙声在寂静的黑暗里显得格外刺耳。",
        "枝叶在你身上扫过，沙沙声中，它刮破了你的蚕丝薄衫和渔网袜。",   // 大萝卜姬
        "你突然跌进了一片黑暗的树丛，诡异的沙沙声勾起了你不好的回忆。",
        "随着一阵沙沙声，密集的枝杈无情地划开了你的衣物和皮肤。",
        "你路过一片树丛，出现了野生的妙蛙种子！快使用大师球！",     // 三月七
        "你踩到了一根萝卜……等等，树林里怎么会有萝卜？",
        "沙沙，沙沙，这片丛林的背后，会不会住着小红帽？",
        "沙沙，你踏入了危险的树丛。这里要是藏着一个老六，可就遭了……",   // Hyacinth
    };
    // 啪啪提示
    static constexpr const array<string_view, 10> papa_hints = {
        "你向前动了动，下半身传来了啪啪声。",   // 大萝卜姬
        "啪啪！常在迷宫走，哪有不湿鞋？是的，你踩到了某些液体。",
        "啪啪！你问踩的是什么液体？也许是我为那情人留下的泪。",
        "啪啪！冰冷的液体渗入了你的鞋里。",
        "啪！尽管你动作已经很轻，但还是发出了很大的声音。啪！你决定不管了。",
        "啪啪！是谁那么不讲公德！随地……",
        "啪啪！冰冷的涟漪在你的鞋边回荡，你想起了那个陪着铁蛋看冰块的下午。",
        "啪啪！突如其来的声音让你的动作瞬间冻结；被打破的静谧仿佛被劈开的水，迅速地恢复了无声。你的敏锐好像没有得到回应。",
        "啪嗒！你好像踩到了地雷？低头看一看，还好，只是一些液体。",     // 三月七
        "啪！你似乎有什么东西掉了进去，可惜这里并没有河神。",
    };
    // 陷阱触发提示
    static constexpr const array<string_view, 5> trap_hints = {
        "深阱垂空百尺方，足悬铁索断人肠。",     // 齐齐
        "犹似魂断垓下道，恨满胸中万古刀。",
        "你想起守株待兔的故事，只是此刻，你成为了那只兔子。",   // 纤光
        "为坠落的人类命名：_________",
        "恭喜🎉被特斯拉捕获，电击调教一回合",       // 特斯拉
    };
    // 热浪提示
    static constexpr const array<string_view, 5> heat_wave_hints = {
        "你感受到了迎面扑来的热浪，炽热的空气仿佛要将你吞没",   // 铁蛋
        "你感受到周围弥漫着炽热的气息！",
        "！！请注意！！局部出现厄尔尼诺现象，气温异常升高，请注意做好防中暑措施。", // 纤光
        "你感到难以忍受的炎热，“要是周围有个水池就好了…”",
        "在这座冰冷的迷宫里，你感到一阵久违的温暖，周围似乎有明亮的光源，吸引你一探究竟…？",
    };
    // 热源进入提示
    static constexpr const array<string_view, 4> heat_core_hints = {
        "你的脚被高温烫伤了，刺痛让你不由得倒吸一口凉气，然而周围的空气同样炙热无比",   // 铁蛋
        "然而，光源并不总象征着安全，烈焰利用人对光明的向往，试图再次吞噬一个失落的灵魂。", // 纤光
        "oops！检测到核心温度急剧上升，即将超过阈值…准备启动自毁程序…",
        "你相信自己的铜头铁臂可以击败一切，却不知眼前的岩浆能轻易融化所有金属。",
    };
    // 热源出局提示
    static constexpr const array<string_view, 3> heat_end_hints = {
        "你被滚滚热浪永远淹没了...周围的一切都在高温中扭曲变形，直到你彻底消失在火焰的深渊。",  // 铁蛋
        "哦不！你落入了巨人萝卜的火锅池里，这下你只好成为萝卜的夜宵了。",   // 纤光
        "你失败了！\nSteve试图在岩浆中游泳。",
    };
    // 逃生舱提示
    static constexpr const array<string_view, 17> exit_hints = {
        "你坐进了逃生舱，在启动的轰鸣声中，你想起了那句话：“不要忘了，这个世界穿透一切高墙的东西，它就在我们的内心深处，他们无法达到，也接触不到，那就是希望。”",   // 大萝卜姬
        "躺在逃生舱内，平日并不虔诚的你颤巍巍地画着十字，双手合十，嘴里念念有词。诸如什么真主阿拉耶稣基督释迦牟利急急如律令之类。前窗仿佛响应了你的号召，一阵白色闪光迅速笼罩了你。正当你诧异得到了哪位神仙的庇佑时，眼前浮现出两个大字。一个振奋人心的声音在你耳边响起：“原神，启动！”",
        "你忘记躺了多久，你只记得这里很温暖、舒适、令人安心。直到一股力量把你从舱内抽离；强光穿透了你稚嫩的眼皮；你哭了，你向世界宣告着你的降临。也许你只是此刻降生的其中之一，但在她眼里，你就是她的唯一。",
        "是的。夜色再浓，也挡不住黎明的到来，就像再大的困难也挡不住我们的前进。黑暗即将过去，曙光就在前头！",
        "你出生后 时间就已所剩无几\n在妈妈离世之后 我不知对你倾注了多少的爱呢\n但是你的微笑让爸爸备受鼓舞呀(^_^)\n其实要是能一起走就好了 但没能做到\n希望你能忘记一切继续前行 你一定可以做到的",    // 纤光
        "当逃生舱门缓缓关闭，伴随着沉闷的启动声，黑暗迷宫逐渐远去。依靠在逃生舱内冷冽的仪表光芒中，你仿佛听见遥远星辰的低语：“未来，总为勇者留下一缕希望。”",   // 铁蛋
        "舱门缓缓关闭，逃生舱的指示灯一一亮起，冰冷的金属包裹着你，但比起外面的黑暗，这里却意外地令人安心。你知道，一切都已经结束，或许，也是一切的开始。",
        "你说的对，但是《漫漫长夜》是由大萝卜姬自主研发的一款全新大逃杀游戏。游戏发生在一个被称作“黑暗迷宫”的地图，在这里，被铁蛋选中的人将被授予“树丛”，导引“沙沙”之力。你将扮演一位名为“狩猎者”的神秘角色，在自由的旅行中邂逅性格各异、能力独特的墙壁，和它们一起阻拦对手，找到隐匿的“逃生者”——同时，逐步发掘“逃生舱”的真相。",   // 三月七
        "进入逃生舱后，随着几下逐渐变弱的震动，周围的环境随之稳定下来。也就在这时，你眼前闪过一道白光，似乎是这使得你进入了一个全新的环境，伴随着的还有来自外部的一阵欢呼声：“太好了！成功抓住宝可梦了！”",     // faust
        "一阵失重后，舱门终于打开。随着刺眼的白光，在指缝间你看见几个面目可憎的巨人在围观你。很快地你被巨大的餐叉粗暴地刺穿；顾不及对痛觉反应，你便殒命在血盘大口之中。健硕、坚定、智慧、乐观，这些优秀的品质在他们嘴里同样珍贵。",   // 大萝卜姬
        "当逃生舱的舱门关闭时，你才发现手中的钥匙根本不属于这里。系统提示音冰冷地重复着：身份验证失败。原来从一开始，你就只是这个迷宫的装饰品而已。",   // 月影
        "逃生舱启动的瞬间，你突然想起那个古老的预言：'逃出迷宫的人将获得永生，但代价是永远孤独'。舱体剧烈震动起来，不知是故障还是某种警告...",
        "舱内显示屏突然亮起：'恭喜您成为第1024位逃生者！作为奖励，系统将向您展示迷宫的真相...'画面切换的瞬间，你看到了无数个一模一样的逃生舱，里面坐着无数个一模一样的你。",
        "当逃生舱启动时，你突然明白：黑暗不是终点，而是黎明前的温柔。舱内温度逐渐升高，那不是故障，而是新生的心跳。",
        "逃生舱启动的轰鸣声渐渐平息，取而代之的是轻柔的摇篮曲。透过舷窗，你看见繁星组成的银河缓缓流动——原来迷宫的出口，一直连接着整片宇宙。",
        "当舱门完全关闭的瞬间，你听见系统轻声说：'恭喜，这是第1024次模拟。根据数据，你这次终于选择相信自己了。' 周围突然亮起温暖的阳光，原来真正的逃生舱，一直都在你心里。",
        "逃生舱的显示屏突然亮起一行字：'记住，黑暗只是光明的候车室。' 随着这句话，整个舱体开始散发出柔和的金色光芒，照亮了通往新世界的道路。",
    };
    // 捕捉提示
    static constexpr const array<string_view, 6> catch_hints = {
        "星光黯淡，你们的相遇，是命中注定，亦是命终注定。",     // 纤光
        "你化作一道黑影，在血月之下，无情地终结了又一条生命。",
        "你想触碰一切的真相，但在对方空洞无神的双眼中，你没能找到答案。",
        "你叹了口气，在黑暗森林里，你不得不这样做。",
        "我说我杀人不眨眼，你问我眼睛干不干？永别了",   // 克里斯丁
        "感谢你为了我自愿放弃逃生资格",
    };
    // 同格树丛声响提示
    static constexpr const array<string_view, 2> grass_sound_hints = {
        "你听见有人进入了你的小树丛，沙沙声很近很近；一股接一股热气扑向了你的耳朵；呼…哈……呼…哈……他好像很累的样子。积极地想，他也许没有察觉到你",   // 大萝卜姬
        "你听见有人进入了你所在的树丛，他从旁边匆匆走过，没有发现你。阴暗的想法在你心里成长起来，是让他帮你探路，还是直接干掉。甚至运气好的话，抢在前面牛了他的逃生舱……（额外探索分只有1分并逃生一事在漫漫长夜中亦有记载）",    // Hyacinth
    };
    // 同格啪啪声响提示
    static constexpr const array<string_view, 2> papa_sound_hints = {
        "“啪！”你汗毛直立，有人来了。。幸好，你并没有站在中间，多疑多虑的性格给了你久违的回报。你小心翼翼地蹲了下来，尽力减少接触概率。只是黑暗中你低估了脚下液体的深度。“等他走远，再把内裤脱了吧。。”你暗暗地想。",   // 大萝卜姬
        "啪！啪！看来是有人来了。两个人，狭小的隔间，不间断地啪啪声……'淫秽的人！'你的脑海回想起了她的声音。是啊。我承认，我确实有点想她了。",
    };
    // 无逃生舱最后生还
    static constexpr const array<string_view, 2> withoutE_win_hints = {
        "我睁开了双眼，眼前的一切既熟悉又陌生。看来这次终于是我赢了。我用力地端详着周遭的一切，试图捕捉错过的几日时光的任何蛛丝马迹。“我真希望他们彻底离开了 ......”说完，我在床脚拿起了本该在枕边的剃须刀；“看来上次赢的是萝卜。”我下意识地抹了抹嘴唇。在指尖晕开的口红证实了我的猜测。我笑了。",     // 大萝卜姬
        "你醒啦？现在已经是第二天了哦。\n明媚的阳光照进迷宫，耳旁传来小鸟的叫声，一切美好的不太真实，唯有眼前冰冷的血迹，无声的诉说着昨晚的那场噩梦，而有些人，永远留在了那场梦中。\n可你，真的从中逃出来了吗？\n“地形参数设置完毕，新的循环正在重启……”",   // 纤光
    };
    // 有逃生舱但死斗取胜
    static constexpr const array<string_view, 2> withE_win_hints = {
        "“已经没事啦~”她温柔地从背后把你抱住，轻轻抚摸着头。“你很勇敢，这一步太不容易了。”你转过身，紧紧埋进她柔软的身体里放肆哭泣。“一切都结束了。不用再害怕了。”她低下头凑近你的耳边，“咱们回家叭…”",     // 大萝卜姬
        "多年以后，在家族背景下你在事业上取得了巨大成就。大家把你的性情大变归功于当年失踪逃生的经历。“之前那个玩世不恭的我已经死了。”你每次都这样认真回答大家。至于细节的提问嘛，失忆这个理由你很喜欢。",
    };

    static string RandomHint(std::span<const string_view> hints)
    {
        return string(hints[rand() % hints.size()]);
    }

    vector<pair<int, int>> pos = {
        {0, 0}, {0, 3}, {0, 6},
        {3, 0}, {3, 3}, {3, 6},
        {6, 0}, {6, 3}, {6, 6},
    };
    vector<pair<int, int>> origin_pos;

    struct Map {
        vector<vector<Grid>> block;
        GridType type;
        string id;
    };
    const int k_map_num = 12;
    const int k_exit_num = 4;
    std::mt19937 g;

    const vector<Map> all_maps = {
        {Map1(), GridType::WATER, "1"},
        {Map2(), GridType::PORTAL, "2"},
        {Map3(), GridType::GRASS, "3"}, 
        {Map4(), GridType::GRASS, "4"},
        {Map5(), GridType::WATER, "5"},
        {Map6(), GridType::PORTAL, "6"},
        {Map7(), GridType::GRASS, "7"},
        {Map8(), GridType::GRASS, "8"},
        {Map9(), GridType::EMPTY, "9"},
        {Map10(), GridType::EMPTY, "10"},
        {Map11(), GridType::GRASS, "11"},
        {Map12(), GridType::GRASS, "12"},
        {Map13(), GridType::PORTAL, "13"},
        {Map14(), GridType::PORTAL, "14"},
        {Map15(), GridType::PORTAL, "15"},
        {Map16(), GridType::PORTAL, "16"},
        {Map17(), GridType::WATER, "17"},
        {Map18(), GridType::WATER, "18"},
        {Map19(), GridType::EMPTY, "19"},
        {Map20(), GridType::EMPTY, "20"},
        {Map21(), GridType::EMPTY, "21"},
        {Map22(), GridType::EMPTY, "22"},
        {Map23(), GridType::TRAP, "23"},
        {Map24(), GridType::TRAP, "24"},
        {Map25(), GridType::HEAT, "25"},
        {Map26(), GridType::BOX, "26"},
        {Map27(), GridType::BOX, "27"},
        {Map28(), GridType::PORTAL, "28"},
        {Map29(), GridType::WATER, "29"},
        {Map30(), GridType::WATER, "30"},
        // {Map31(), GridType::PORTAL, "31"},
        // {Map32(), GridType::PORTAL, "32"},
        {Map33(), GridType::TRAP, "33"},
    };
    const vector<Map> all_exits = {
        {Exit1(), GridType::EXIT, "1"},
        {Exit2(), GridType::EXIT, "2"}, 
        {Exit3(), GridType::EXIT, "3"},
        {Exit4(), GridType::EXIT, "4"},
        {Exit5(), GridType::EXIT, "5"},
        {Exit6(), GridType::EXIT, "6"},
        {Exit7(), GridType::EXIT, "7"},
        {Exit8(), GridType::EXIT, "8"},
        {Exit9(), GridType::EXIT, "9"},
        {Exit10(), GridType::EXIT, "10"},
    };
    const vector<Map> special_maps = {
        {SMap1(), GridType::SPECIAL, "S1"},
        {SMap2(), GridType::SPECIAL, "S2"},
        {SMap3(), GridType::SPECIAL, "S3"},
        {SMap4(), GridType::SPECIAL, "S4"},
    };

    const vector<string> rotation_maps_id = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12",
        "21", "22", "23", "24", "26",
        "29", "30", "33"
    };
    const vector<string> rotation_exits_id = {"1", "2", "3", "4"};
    vector<Map> rotation_maps;
    vector<Map> rotation_exits;

    vector<Map> maps;
    vector<Map> exits;
    
    vector<Map> origin_maps;
    vector<Map> origin_exits;

    UnitMaps(const int32_t mode)
    {
        std::random_device rd;
        g = std::mt19937(rd());
        if (mode == 0) {
            maps.insert(maps.end(), all_maps.begin(), all_maps.begin() + k_map_num);
            exits.insert(exits.end(), all_exits.begin(), all_exits.begin() + k_exit_num);
        } else if (mode == 1) {
            std::sample(all_maps.begin(), all_maps.end(), std::back_inserter(maps), k_map_num, g);
            SampleExits(all_exits, k_exit_num / 2);
        } else if (mode == 2) {
            for (const auto &m : all_maps) {
                if (std::find(rotation_maps_id.begin(), rotation_maps_id.end(), m.id) != rotation_maps_id.end()) {
                    rotation_maps.push_back(m);
                }
            }
            for (const auto &m : all_exits) {
                if (std::find(rotation_exits_id.begin(), rotation_exits_id.end(), m.id) != rotation_exits_id.end()) {
                    rotation_exits.push_back(m);
                }
            }
            std::sample(rotation_maps.begin(), rotation_maps.end(), std::back_inserter(maps), k_map_num, g);
            SampleExits(rotation_exits, k_exit_num / 2);
        } else {
            std::sample(special_maps.begin(), special_maps.end(), std::back_inserter(maps), 2, g);
            std::sample(all_maps.begin(), all_maps.end(), std::back_inserter(maps), k_map_num - 2, g);
            SampleExits(all_exits, k_exit_num / 2);
        }
        origin_maps = maps;
        origin_exits = exits;
        origin_pos = pos;
    }

    void SampleExits(const vector<Map>& exits_pool, const int k_exit_pair)
    {
        if (exits_pool.size() % 2 != 0) return;

        int pair_num = exits_pool.size() / 2;
        vector<int> pairs(pair_num);
        std::iota(pairs.begin(), pairs.end(), 0);
        std::shuffle(pairs.begin(), pairs.end(), g);
        pairs.resize(k_exit_pair);
        std::sort(pairs.begin(), pairs.end());
        for (int p : pairs) {
            exits.push_back(exits_pool[p * 2]);
            exits.push_back(exits_pool[p * 2 + 1]);
        }
    }

    vector<vector<Grid>> FindBlockById(const string id, const bool is_exit, const bool special = false) const
    {
        const vector<Map>& search_list = is_exit ? (special ? exits : all_exits) : (special ? maps : all_maps);
        auto it = std::find_if(search_list.begin(), search_list.end(), [id](const Map& map) { return map.id == id; });
        if (it != search_list.end()) return it->block;
        auto special_it = std::find_if(special_maps.begin(), special_maps.end(), [id](const Map& map) { return map.id == id; });
        if (special_it != special_maps.end()) return special_it->block;
        return InitializeMapTemplate();
    }

    static bool MapContainGridType(const vector<Map> maps, const GridType& type)
    {
        for (const auto& map: maps) {
            for (int k = 0; k < 9; ++k) {
                if (map.block[k / 3][k % 3].Type() == type) {
                    return true;
                }
            }
        }
        return false;
    }

    // 特殊事件详情
    static string ShowSpecialEvent(const int type)
    {
        if (type == 1) {
            return "[特殊事件]【怠惰的园丁】草丛将在其区块内随机位置生成（有可能生成在中间）";
        } else if (type == 2) {
            return "[特殊事件]【营养过剩】草丛将额外向随机1个方向再次生成1个（共8个方向，且不可隔墙生长）";
        } else if (type == 3) {
            return "[特殊事件]【雨天小故事】地图中所有树丛变成水洼，陷阱会发出啪啪声";
        } else {
            return "无";
        }
    }

    // 特殊事件1——怠惰的园丁：草丛将在其区块内随机位置生成
    void SpecialEvent1()
    {
        for (auto& map: maps) {
            for (int k = 0; k < 9; ++k) {
                int i = k / 3, j = k % 3;
                if (map.block[i][j].Type() == GridType::GRASS) {
                    map.block[i][j].SetType(GridType::EMPTY);
                    int m;
                    do {
                        m = rand() % 9;
                    } while (map.block[m / 3][m % 3].Type() != GridType::EMPTY);
                    map.block[m / 3][m % 3].SetType(GridType::GRASS);
                    break;
                }
            }
        }
    }

    // 特殊事件2——营养过剩：树丛将额外向随机1个方向再次生成1个
    void SpecialEvent2()
    {
        for (auto& map: maps) {
            for (int k = 0; k < 9; ++k) {
                int i = k / 3, j = k % 3;
                if (map.block[i][j].Type() == GridType::GRASS && map.type != GridType::SPECIAL) {
                    int m;
                    do {
                        m = rand() % 9;
                    } while (!map.block[m / 3][m % 3].CanGrow());
                    map.block[m / 3][m % 3].SetType(GridType::GRASS);
                    break;
                }
            }
        }
    }

    // 特殊事件3——雨天小故事：地图中所有树丛变成水洼
    void SpecialEvent3()
    {
        for (auto& map: maps) {
            for (int k = 0; k < 9; ++k) {
                int i = k / 3, j = k % 3;
                if (map.block[i][j].Type() == GridType::GRASS) {
                    map.block[i][j].SetType(GridType::WATER);
                }
            }
        }
    }

    // 大地图区块位置随机
    bool RandomizeBlockPosition(const int size)
    {
        vector<pair<int,int>> candidates;
        for (int i = 0; i < size - 2; i++) {
            for (int j = 0; j < size - 2; j++) {
                candidates.push_back({i, j});
            }
        }
        random_shuffle(candidates.begin(), candidates.end());
        vector<pair<int,int>> chosen;
        bool found = backtrack(0, chosen, candidates);
        if (found) {
            for (int i = 0; i < 9; i++) {
                pos[i] = chosen[i];
            }
        }
        return found;
    }

    // 边长12：16个区块
    void SetMapPosition12()
    {
        pos.insert(pos.begin() + 3, {0, 9});
        pos.insert(pos.begin() + 6, {3, 9});
        pos.push_back({6, 9});
        pos.push_back({9, 0});
        pos.push_back({9, 3});
        pos.push_back({9, 6});
        pos.push_back({9, 9});
        origin_pos = pos;
    }

    void SetMapBlock(const int x, const int y, vector<vector<Grid>>& grid_map, const string map_id, const bool special) const
    {
        SetBlock(x, y, grid_map, FindBlockById(map_id, false, special));
    }

    void SetExitBlock(const int x, const int y, vector<vector<Grid>>& grid_map, const string exit_id, const bool special) const
    {
        SetBlock(x, y, grid_map, FindBlockById(exit_id, true, special));
    }

  private:
    void SetBlock(const int x, const int y, vector<vector<Grid>>& grid_map, const vector<vector<Grid>> block) const
    {
        for (int i = 0; i < block.size(); i++) {
            for (int j = 0; j < block[i].size(); j++) {
                grid_map[x + i][y + j] = block[i][j];
            }
        }
    }

    static bool overlaps(const pair<int, int>& a, const pair<int, int>& b)
    {
        if (a.first + 2 < b.first || b.first + 2 < a.first || a.second + 2 < b.second || b.second + 2 < a.second)
            return false;
        return true;
    }

    // 回溯搜索
    static bool backtrack(int index, vector<pair<int, int>>& chosen, const vector<pair<int, int>>& candidates)
    {
        if (chosen.size() == 9) {
            return true;
        }
        for (int i = index; i < candidates.size(); i++) {
            bool valid = true;
            for (const auto& p : chosen) {
                if (overlaps(p, candidates[i])) {
                    valid = false;
                    break;
                }
            }
            if (!valid)
                continue;
            chosen.push_back(candidates[i]);
            if (backtrack(i + 1, chosen, candidates))
                return true;
            chosen.pop_back();
        }
        return false;
    }

    static vector<vector<Grid>> InitializeMapTemplate()
    {
        vector<vector<Grid>> map;
        map.resize(3);
        for (auto& row : map) {
            row.resize(3);
        }
        return map;
    }

    static vector<vector<Grid>> Map1()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::WATER);
        map[0][2].SetType(GridType::WATER);
        map[2][0].SetType(GridType::WATER);
        map[2][2].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map2()
    {
        auto map = InitializeMapTemplate();
        map[0][2].SetType(GridType::PORTAL).SetPortal(2, -2);
        map[2][0].SetType(GridType::PORTAL).SetPortal(-2, 2);
        map[1][1].SetType(GridType::GRASS);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[0][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        return map;
    }

    static vector<vector<Grid>> Map3()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::GRASS);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[0][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);

        map[1][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        map[2][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);

        return map;
    }

    static vector<vector<Grid>> Map4()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::GRASS);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[2][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map5()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::WATER);
        map[0][2].SetType(GridType::WATER);
        map[2][0].SetType(GridType::WATER);
        map[2][2].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[0][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map6()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::PORTAL).SetPortal(2, 2);
        map[2][2].SetType(GridType::PORTAL).SetPortal(-2, -2);
        map[1][1].SetType(GridType::GRASS);

        map[0][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[0][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        map[1][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[2][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map7()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::GRASS);

        map[0][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[0][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);

        map[1][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        map[2][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[2][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);

        return map;
    }

    static vector<vector<Grid>> Map8()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::GRASS);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);

        return map;
    }

    static vector<vector<Grid>> Map9()
    {
        auto map = InitializeMapTemplate();

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[2][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map10()
    {
        auto map = InitializeMapTemplate();

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);

        return map;
    }

    static vector<vector<Grid>> Map11()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::GRASS);

        map[0][0].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[0][2].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        map[1][0].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        map[2][0].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[2][2].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        return map;
    }

    static vector<vector<Grid>> Map12()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::GRASS);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL).SetGrowable(true);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL).SetGrowable(true);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL).SetGrowable(true);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL).SetGrowable(true);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL).SetGrowable(true);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL).SetGrowable(true);

        return map;
    }

    static vector<vector<Grid>> Map13()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::WATER);
        map[0][2].SetType(GridType::PORTAL).SetPortal(2, -2);
        map[2][0].SetType(GridType::PORTAL).SetPortal(-2, 2);
        map[2][2].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[0][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map14()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::PORTAL).SetPortal(2, 2);
        map[0][2].SetType(GridType::WATER);
        map[2][0].SetType(GridType::WATER);
        map[2][2].SetType(GridType::PORTAL).SetPortal(-2, -2);

        map[0][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map15()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::PORTAL).SetPortal(0, 2);
        map[0][2].SetType(GridType::PORTAL).SetPortal(0, -2);
        map[2][0].SetType(GridType::WATER);
        map[2][2].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[0][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map16()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::PORTAL).SetPortal(1, 0);
        map[2][1].SetType(GridType::PORTAL).SetPortal(-1, 0);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);

        map[2][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[2][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map17()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::WATER);
        map[0][2].SetType(GridType::WATER);
        map[1][1].SetType(GridType::GRASS);
        map[2][0].SetType(GridType::WATER);
        map[2][2].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[0][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        map[2][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[2][2].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map18()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::WATER);
        map[0][2].SetType(GridType::WATER);
        map[1][1].SetType(GridType::GRASS);
        map[2][0].SetType(GridType::WATER);
        map[2][2].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[0][2].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        map[2][0].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[2][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map19()
    {
        auto map = InitializeMapTemplate();

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);

        map[1][0].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);

        return map;
    }

    static vector<vector<Grid>> Map20()
    {
        auto map = InitializeMapTemplate();

        map[0][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[1][2].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);

        map[2][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map21()
    {
        auto map = InitializeMapTemplate();

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);
        map[0][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);

        map[1][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map22()
    {
        auto map = InitializeMapTemplate();

        map[0][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);
        map[2][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);

        return map;
    }

    static vector<vector<Grid>> Map23()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::TRAP);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);

        return map;
    }

    static vector<vector<Grid>> Map24()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::TRAP);

        map[0][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map25()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::HEAT);

        return map;
    }

    static vector<vector<Grid>> Map26()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::BOX);

        return map;
    }

    static vector<vector<Grid>> Map27()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::BOX);
        map[2][2].SetType(GridType::BOX);

        return map;
    }

    static vector<vector<Grid>> Map28()
    {
        auto map = InitializeMapTemplate();
        map[1][0].SetType(GridType::PORTAL).SetPortal(0, 2);
        map[1][2].SetType(GridType::PORTAL).SetPortal(0, -2);
        map[1][1].SetType(GridType::TRAP);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map29()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::WATER);
        map[1][1].SetType(GridType::WATER);
        map[2][2].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map30()
    {
        auto map = InitializeMapTemplate();
        map[0][2].SetType(GridType::WATER);
        map[1][1].SetType(GridType::WATER);
        map[2][0].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map31()
    {
        auto map = InitializeMapTemplate();
        map[0][2].SetType(GridType::ONEWAYPORTAL).SetPortal(2, -2);
        map[2][0].SetType(GridType::PORTAL).SetPortal(-2, 2);
        map[1][1].SetType(GridType::GRASS);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[0][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        return map;
    }

    static vector<vector<Grid>> Map32()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::ONEWAYPORTAL).SetPortal(2, 2);
        map[2][2].SetType(GridType::PORTAL).SetPortal(-2, -2);
        map[1][1].SetType(GridType::GRASS);

        map[0][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY).SetGrowable(true);
        map[0][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);

        map[1][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY).SetGrowable(true);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL).SetGrowable(true);
        map[2][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Map33()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::TRAP);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    // static vector<vector<Grid>> Map34()
    // {
    //     auto map = InitializeMapTemplate();
    //     map[0][0].SetType(GridType::WATER);
    //     map[0][2].SetType(GridType::PORTAL).SetPortal(2, -2);
    //     map[2][0].SetType(GridType::PORTAL).SetPortal(-2, 2);
    //     map[2][2].SetType(GridType::WATER);

    //     map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
    //     map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
    //     map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

    //     map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
    //     map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
    //     map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

    //     map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
    //     map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
    //     map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

    //     return map;
    // }

    static vector<vector<Grid>> Exit1()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::EXIT);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Exit2()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::EXIT);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Exit3()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::EXIT);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Exit4()
    {
        auto map = InitializeMapTemplate();
        map[1][1].SetType(GridType::EXIT);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Exit5()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::PORTAL).SetPortal(2, 2);
        map[0][2].SetType(GridType::WATER);
        map[1][1].SetType(GridType::EXIT);
        map[2][0].SetType(GridType::WATER);
        map[2][2].SetType(GridType::PORTAL).SetPortal(-2, -2);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Exit6()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::WATER);
        map[0][2].SetType(GridType::PORTAL).SetPortal(2, -2);
        map[1][1].SetType(GridType::EXIT);
        map[2][0].SetType(GridType::PORTAL).SetPortal(-2, 2);
        map[2][2].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[1][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Exit7()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::WATER);
        map[0][2].SetType(GridType::PORTAL).SetPortal(2, -2);
        map[1][1].SetType(GridType::EXIT);
        map[2][0].SetType(GridType::PORTAL).SetPortal(-2, 2);
        map[2][2].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Exit8()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::WATER);
        map[0][2].SetType(GridType::PORTAL).SetPortal(2, -2);
        map[1][1].SetType(GridType::EXIT);
        map[2][0].SetType(GridType::PORTAL).SetPortal(-2, 2);
        map[2][2].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[1][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        map[2][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Exit9()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::PORTAL).SetPortal(2, 2);
        map[0][1].SetType(GridType::EXIT);
        map[1][1].SetType(GridType::TRAP);
        map[2][2].SetType(GridType::PORTAL).SetPortal(-2, -2);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> Exit10()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::PORTAL).SetPortal(2, 2);
        map[1][1].SetType(GridType::TRAP);
        map[2][1].SetType(GridType::EXIT);
        map[2][2].SetType(GridType::PORTAL).SetPortal(-2, -2);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::NORMAL);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        return map;
    }

    // 特殊地图
    static vector<vector<Grid>> SMap1()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::WATER);
        map[0][1].SetType(GridType::TRAP);
        map[0][2].SetType(GridType::PORTAL).SetPortal(2, -2);
        map[1][0].SetType(GridType::GRASS);
        map[1][1].SetType(GridType::HEAT);
        map[1][2].SetType(GridType::GRASS);
        map[2][0].SetType(GridType::PORTAL).SetPortal(-2, 2);
        map[2][1].SetType(GridType::TRAP);
        map[2][2].SetType(GridType::WATER);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> SMap2()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::PORTAL).SetPortal(2, 2);
        map[0][2].SetType(GridType::PORTAL).SetPortal(2, -2);
        map[1][1].SetType(GridType::WATER);
        map[2][0].SetType(GridType::PORTAL).SetPortal(-2, 2);
        map[2][2].SetType(GridType::PORTAL).SetPortal(-2, -2);

        map[0][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[0][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        map[1][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);

        map[2][0].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[2][1].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[2][2].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> SMap3()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::PORTAL).SetPortal(2, 2);
        map[0][2].SetType(GridType::PORTAL).SetPortal(2, -2);
        map[1][1].SetType(GridType::TRAP);
        map[2][0].SetType(GridType::PORTAL).SetPortal(-2, 2);
        map[2][2].SetType(GridType::PORTAL).SetPortal(-2, -2);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[0][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);

        return map;
    }

    static vector<vector<Grid>> SMap4()
    {
        auto map = InitializeMapTemplate();
        map[0][0].SetType(GridType::PORTAL).SetPortal(2, 2);
        map[0][1].SetType(GridType::PORTAL).SetPortal(2, 0);
        map[0][2].SetType(GridType::PORTAL).SetPortal(2, -2);
        map[1][0].SetType(GridType::PORTAL).SetPortal(0, 2);
        map[1][1].SetType(GridType::PORTAL).SetPortal(0, 0);
        map[1][2].SetType(GridType::PORTAL).SetPortal(0, -2);
        map[2][0].SetType(GridType::PORTAL).SetPortal(-2, 2);
        map[2][1].SetType(GridType::PORTAL).SetPortal(-2, 0);
        map[2][2].SetType(GridType::PORTAL).SetPortal(-2, -2);

        map[0][0].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::EMPTY, Wall::NORMAL);
        map[0][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[0][2].SetWall(Wall::EMPTY, Wall::NORMAL, Wall::NORMAL, Wall::EMPTY);

        map[1][0].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);
        map[1][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::EMPTY, Wall::EMPTY);
        map[1][2].SetWall(Wall::NORMAL, Wall::NORMAL, Wall::EMPTY, Wall::EMPTY);

        map[2][0].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::EMPTY, Wall::NORMAL);
        map[2][1].SetWall(Wall::EMPTY, Wall::EMPTY, Wall::NORMAL, Wall::NORMAL);
        map[2][2].SetWall(Wall::NORMAL, Wall::EMPTY, Wall::NORMAL, Wall::EMPTY);

        return map;
    }
};
