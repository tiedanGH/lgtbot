// 乌合之众题库（测试题目，题号 t 开头）
// 文案 RC() 规则见 problems.h 顶部说明，本文件须在 problems.h 之后引入。

class Qt1: public Question   // [待定]暂时无法判断选项平衡性
{
public:
	Qt1()
	{
		id = 1;
		author = "Chance";
		title = "未命名";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
		options.push_back("平分 [B选项人数-5绝对值] 的分数");
		options.push_back("平分 [A选项人数-6绝对值] 的分数");
		options.push_back("平分 [人数最多的选项人数] 的分数");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbbcc");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = abs(optionCount[1] - 5) / optionCount[0];
		tempScore[1] = abs(optionCount[0] - 6) / optionCount[1];
		tempScore[2] = maxSelect / optionCount[2];
	}
};

class Qt2 : public Question   // [待修改]A选项不平衡，无人选择
{
public:
	Qt2()
	{
		id = 2;
		author = "xiaogt";
		title = "怪物糖果";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		vars["point"] = playerNum * 2.5;
		texts.push_back("有 " + str(vars["point"]) + " 分可供瓜分，你可以选择拿取的分数，然后从小到大进行拿取操作。但当剩余分数不够某个选项拿取时，该组及更高分组-1分。");
	}
	virtual void initOptions() override
	{
        options.push_back("1分");
        options.push_back("2分");
		options.push_back("3分");
		options.push_back("4分");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabcccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		int need[4] = {1, 2, 3, 4};
		int point = vars["point"];
		for(int i = 0; i < 4; i++)
		{
			if(point >= optionCount[i] * need[i])
			{
				point -= optionCount[i] * need[i];
				tempScore[i] = need[i];
			} else {
				tempScore[i] -= 1;
				for (int j = i; j < 4; j++) {
					need[j] -= 1;
				}
			}
		}
	}
};

class Qt3: public Question   // [待定]C选项达成难度过高
{
public:
	Qt3()
	{
		id = 3;
		author = "九九归一";
		title = "伊甸园";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择下列一项：");
	}
	virtual void initOptions() override
	{
		options.push_back("金苹果：若选择此项的人数小于选择B的人数，则平分 [未选择此项的人数] 分，否则减1分。");
		options.push_back("银苹果：若选择此项的人数小于选择A的人数，则平分 [未选择此项的人数] 分，否则减1分。");
		options.push_back("红苹果：若选择A的人数等于选择B的人数，则平分 [未选择此项的人数] 分，否则减1分。");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabbc");
	}
	virtual void calc(vector<Player>& players) override
	{
		int win, l1, l2;
		if (optionCount[0] < optionCount[1]) {
			win = 0;
			l1 = 1; l2 = 2;
		} else if (optionCount[1] < optionCount[0]) {
			win = 1;
			l1 = 0; l2 = 2;
		} else {
			win = 2;
			l1 = 0; l2 = 1;
		}
		tempScore[win] = (optionCount[l1] + optionCount[l2]) / optionCount[win];
		tempScore[l1] = tempScore[l2] = -1;
	}
};

class Qt4 : public Question   // [待修改]正向和反向执行，统一结算难以看懂
{
public:
	Qt4()
	{
		id = 4;
		author = "栗子";
		title = "利他之众";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选的人数最多的执行，最少（包括0人）反向执行");
	}
	virtual void initOptions() override
	{
        options.push_back("B每人1分");
        options.push_back("C平分3分");
		options.push_back("AD平分3分");
		options.push_back("AC平分3分");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		if (optionCount[0] == maxSelect) {
			tempScore[1] += 1;
		}
		if (optionCount[1] == maxSelect) {
			tempScore[2] += 3 / optionCount[2];
		}
		if (optionCount[2] == maxSelect) {
			tempScore[0] += 3 / optionCount[0];
			tempScore[3] += 3 / optionCount[3];
		}
		if (optionCount[3] == maxSelect) {
			tempScore[0] += 3 / optionCount[0];
			tempScore[2] += 3 / optionCount[2];
		}

		if (optionCount[0] == minSelect) {
			tempScore[1] -= 1;
		}
		if (optionCount[1] == minSelect) {
			tempScore[2] -= 3 / optionCount[2];
		}
		if (optionCount[2] == minSelect) {
			tempScore[0] -= 3 / optionCount[0];
			tempScore[3] -= 3 / optionCount[3];
		}
		if (optionCount[3] == minSelect) {
			tempScore[0] -= 3 / optionCount[0];
			tempScore[2] -= 3 / optionCount[2];
		}
	}
};

class Qt5: public Question   // [待修改]存在50%概率影响
{
public:
	Qt5()
	{
		id = 5;
		author = "飞机鸭卖蛋";
		title = "听力练习";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("耳机电量不佳，背书还是听力？");
	}
	virtual void initOptions() override
	{
		options.push_back("背书：+1，噪音值+1");
		options.push_back("耳机标准模式：+3-噪音值");
		options.push_back("耳机降噪模式：+3，但50%概率中途关机，改为+0");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbbcc");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = 1;
		tempScore[1] = 3 - optionCount[0];
		tempScore[2] = rand() % 2 ? 3 : 0;
	}
};

class Qt6: public Question   // [待定]暂时无法判断选项平衡性
{
public:
	Qt6()
	{
		id = 6;
		author = "小黄鸭";
		title = "E卡抉择";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项，依次生效。");
	}
	virtual void initOptions() override
	{
		options.push_back(RC("皇帝：+2分", "高位：+2分"));
		options.push_back(RC("市民：若A比C多则-1分；否则+1分。", "中位：若A比C多则-1分；否则+1分。"));
		options.push_back(RC(
			"奴隶：若有人选择此项且没有人选择B，则+1分且所有选A的玩家的总得分变为0；否则-1分。",
			"低位：若有人选择此项且没有人选择B，则+1分且所有选A的玩家的总得分变为0；否则-1分。"));
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabccc");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[1] = optionCount[0] > optionCount[2] ? -1 : 1;
		if (optionCount[2] > 0 && optionCount[1] == 0) {
			tempScore[2] = 1;
			for (int i = 0; i < playerNum; i++) {
				if (players[i].select == 0) {
					players[i].score = 0;
				}
			}
		} else {
			tempScore[0] = 2;
			tempScore[2] = -1;
		}
	}
};

class Qt7 : public Question   // [待修改]存在50%概率影响
{
public:
	Qt7()
	{
		id = 7;
		author = "齐齐";
		title = RC("月下人狼", "月下推理");
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
		options.push_back(RC("村人：如果 村人+狂人＞人狼+妖狐，+1，否则-2", "村民：如果 村民+激进派＞狼组+狐组，+1，否则-2"));
		options.push_back(RC("人狼：如果村人扣分，+1，否则-2", "狼组：如果村民扣分，+1，否则-2"));
		options.push_back(RC("狂人：如果村人扣分，+4，否则-2", "激进派：如果村民扣分，+4，否则-2"));
		options.push_back(RC("妖狐：50%概率+2，50%概率-2（所有妖狐得分情况相同）", "狐组：50%概率+2，50%概率-2（所有狐组得分情况相同）"));
	}
	virtual void initExpects() override
	{
		expects.push_back("aabccddd");
	}
	virtual void calc(vector<Player>& players) override
	{
		if (optionCount[0] + optionCount[2] > optionCount[1] + optionCount[3]) {
			tempScore[0] = 1;
		} else {
			tempScore[0] = -2;
		}
		if (tempScore[0] < 0) {
			tempScore[1] = 1;
			tempScore[2] = 4;
		} else {
			tempScore[1] = -2;
			tempScore[2] = -2;
		}
		if (rand() % 2 == 0) {
			tempScore[3] = 2;
		} else {
			tempScore[3] = -2;
		}
	}
};

class Qt8 : public Question	// 备选题目
{
public:
	Qt8()
	{
		id = 8;
		author = "纤光";
		title = "金铃铛";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("每摇一次铃铛，所有摇过铃的人+0.3，且自己额外+0.7；当摇铃总次数>=" + str(int(playerNum * 1.5)) + "时，金铃铛魔力失效，所有摇过铃的人本题0分。");
	}
	virtual void initOptions() override
	{
        options.push_back("摇1次铃");
        options.push_back("摇2次铃");
		options.push_back("摇3次铃");
		options.push_back("+ " + str(playerNum / 4) + " 分");
	}
	virtual void initExpects() override
	{
		expects.push_back("abbbccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		int bell = optionCount[0] + optionCount[1] * 2 + optionCount[2] * 3;
		if (bell < int(playerNum * 1.5)) {
			tempScore[0] = 0.7 + bell * 0.3;
			tempScore[1] = 1.4 + bell * 0.3;
			tempScore[2] = 2.1 + bell * 0.3;
		}
		tempScore[3] = 	playerNum / 4;
	}
};

class Qt9: public Question   // [待定]暂时无法判断选项平衡性    平衡 1
{
public:
	Qt9()
	{
		id = 9;
		author = "Chance";
		title = "未命名";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择下面一项，人数最多和最少的选项都成立");
	}
	virtual void initOptions() override
	{
        options.push_back("A+1，B+2");
        options.push_back("A-2，C-3");
		options.push_back("C+2，D+2");
		options.push_back("D-1，B-2");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabcccd");
	}
	virtual void calc(vector<Player>& players) override
	{
		if (optionCount[0] == maxSelect || optionCount[0] == minSelect) {
			tempScore[0] += 1;
			tempScore[1] += 2;
		}
		if (optionCount[1] == maxSelect || optionCount[1] == minSelect) {
			tempScore[0] -= 2;
			tempScore[2] -= 3;
		}
		if (optionCount[2] == maxSelect || optionCount[2] == minSelect) {
			tempScore[2] += 2;
			tempScore[3] += 2;
		}
		if (optionCount[3] == maxSelect || optionCount[3] == minSelect) {
			tempScore[3] -= 1;
			tempScore[1] -= 2;
		}
	}
};

class Qt10 : public Question   // [待定]题目过于复杂
{
public:
	Qt10()
	{
		id = 10;
		author = "纤光";
		title = "平行时空";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("有一项选择人数唯一最多时，此项生效");
		texts.push_back("有两项选择人数同时最多时，第三项生效");
		texts.push_back("有三项选择人数同时最多时，均不生效，所有人分数清零。");
	}
	virtual void initOptions() override
	{
        options.push_back("若有人选B，选C的人+1");
        options.push_back("若有人选C，选A的人+2");
		options.push_back("若有人选A，选B的人+3");
	}
	virtual void initExpects() override
	{
		expects.push_back("abc");
	}
	virtual void calc(vector<Player>& players) override
	{
		int maxCount = 0;
		int win[2];
		for (int i = 0; i < 3; i++) {
			if (optionCount[i] == maxSelect) {
				maxCount++;
				win[0] = i;
			} else {
				win[1] = i;
			}
		}
		if (maxCount == 3) {
			for (int i = 0; i < playerNum; i++) {
				players[i].score = 0;
			}
		} else {
			if (win[maxCount - 1] == 0 && optionCount[1] > 0) {
				tempScore[2] = 1;
			}
			if (win[maxCount - 1] == 1 && optionCount[2] > 0) {
				tempScore[0] = 2;
			}
			if (win[maxCount - 1] == 2 && optionCount[0] > 0) {
				tempScore[1] = 3;
			}
		}
	}
};

class Qt11 : public Question   // [待定]存在人数影响
{
public:
	Qt11()
	{
		id = 11;
		author = "本仙子很强";
		title = RC("兵主", "元素塔");
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
        options.push_back(RC("鲜血：+1", "红光：+1"));
        options.push_back(RC("冰霜：本回合中，你的扣分变为加分。但是如果没有扣分，你-2", "蓝光：本回合中，你的扣分变为加分。但是如果没有扣分，你-2"));
		options.push_back(RC("邪恶：使所有没有选择邪恶的人-1", "黑光：使所有没有选择黑光的人-1"));
		options.push_back(RC("彩虹：如果ABC均有人选择，+[人数/2]向上取整", "彩光：如果ABC均有人选择，+[人数/2]向上取整"));
	}
	virtual void initExpects() override
	{
		expects.push_back("aaaabbbccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = 1;
		if (optionCount[2] > 0) {
			tempScore[0] -= 1;
			tempScore[1] += 1;
			tempScore[3] -= 1;
		} else {
			tempScore[1] = -2;
		}
		tempScore[3] = optionCount[0] > 0 && optionCount[1] > 0 && optionCount[2] > 0 ? ceil((optionCount[0] + optionCount[1] + optionCount[2]) / 2.0) : 0;
	}
};

class Qt12 : public Question   // [待定]多人数值增减可能过大
{
public:
	Qt12()
	{
		id = 12;
		author = "周小墨";
		title = RC("炉石传说", "传奇对局");
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
        options.push_back("冲锋：+6，每有一人选择本选项得分-1");
        options.push_back("嘲讽：每有一人选择冲锋得分+1");
		options.push_back("突袭：-1，每有一人选择嘲讽得分+1");
		options.push_back("圣盾：+1");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbbccdddd");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = 6 - optionCount[0];
		tempScore[1] = optionCount[0];
		tempScore[2] = -1 + optionCount[1];
		tempScore[3] = 1;
	}
};

class Qt13 : public Question   // 备选题目   平衡 1
{
public:
	Qt13()
	{
		id = 13;
		author = "克里斯丁";
		title = RC("逆转裁判", "证词推理");
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back(RC("律师和检察官合伙针对证人。", "辩方和质询方合伙针对证人。"));
		texts.push_back(RC(
			"若BC人数总和加起来大于A的人数，证人被抓，反之被告被抓。你选择担任",
			"若BC人数总和加起来大于A的人数，证人被罚，反之申诉方被罚。你选择担任"));
	}
	virtual void initOptions() override
	{
        options.push_back(RC("证人：无论是否被抓均+1，且若只有一人选择额外+3", "证人：无论是否被罚均+1，且若只有一人选择额外+3"));
        options.push_back(RC("律师：+2，被告被抓改为+0", "辩方：+2，申诉方被罚改为+0"));
		options.push_back(RC("检察官：+2.5，若证人被抓改为+0", "质询方：+2.5，若证人被罚改为+0"));
	}
	virtual void initExpects() override
	{
		expects.push_back("aaaabbccc");
	}
	virtual void calc(vector<Player>& players) override
	{
		bool witness_arrested = optionCount[1] + optionCount[2] > optionCount[0];
		tempScore[0] = optionCount[0] == 1 ? 4 : 1;
		tempScore[1] = witness_arrested ? 2 : 0;
		tempScore[2] = witness_arrested ? 0 : 2.5;
	}
};

class Qt14 : public Question   // 平衡 1
{
public:
	Qt14()
	{
		id = 14;
		author = "克里斯丁";
		title = RC("对战", "对决");
	}

	virtual void initTexts(vector<Player>& players) override
	{
		vars["num"] = 10;
		texts.push_back(RC(
			"联邦和帝国对战，战力更大的阵营获胜（正常情况下一人一点战力）并平分" + str(vars["num"]) + "分",
			"蓝方和红方对决，战力更大的阵营获胜（正常情况下一人一点战力）并平分" + str(vars["num"]) + "分"));
	}
	virtual void initOptions() override
	{
        options.push_back(RC("联邦文官：无论是否胜利，均+1", "蓝方文官：无论是否胜利，均+1"));
        options.push_back(RC(
        	"联邦议会：无论是否胜利，选择B的玩家单独平分额外三分",
        	"蓝方议会：无论是否胜利，选择B的玩家单独平分额外三分"));
		options.push_back(RC("帝国领袖：只要有人选择C，帝国总战力翻倍", "红方领袖：只要有人选择C，红方总战力翻倍"));
		options.push_back(RC("帝国精锐：一人两点战力", "红方精锐：一人两点战力"));
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		double blue_num = optionCount[0] + optionCount[1];
		double red_num = optionCount[2] + optionCount[3];
		double blue_power = blue_num;
		double red_power = optionCount[2] + optionCount[3] * 2;
		if (optionCount[2] > 0) {
			red_power *= 2;
		}
		tempScore[0] = 1;
		if (optionCount[1] > 0) {
			tempScore[1] += 3 / optionCount[1];
		}
		if (blue_power > red_power) {
			tempScore[0] += vars["num"] / blue_num;
			tempScore[1] += vars["num"] / blue_num;
		} else if (red_power > blue_power) {
			tempScore[2] += vars["num"] / red_num;
			tempScore[3] += vars["num"] / red_num;
		}
	}
};

class Qt15 : public Question   // 备选题目   平衡 1
{
public:
	Qt15()
	{
		id = 15;
		author = "克里斯丁";
		title = "许愿池";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("往池子里投币许愿，一币0.5分，若池子里的钱币比玩家人数多，则许愿成功");
	}
	virtual void initOptions() override
	{
        options.push_back("0币，许愿成功+0.5");
        options.push_back("1币，许愿成功+2");
		options.push_back("2币，许愿成功+3");
		options.push_back("3币，许愿成功+4");
		options.push_back("搅混水：令池子-2币");
		options.push_back(RC("超级搅屎棍：令池子-4币，许愿成功-1", "超级搅混水：令池子-4币，许愿成功-1"));
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabbbccdeeeff");
	}
	virtual void calc(vector<Player>& players) override
	{
		bool success = optionCount[1] + optionCount[2]*2 + optionCount[3]*3 - optionCount[4]*2 - optionCount[5]*4 > playerNum;
		tempScore[0] = success ? 0.5 : 0;
		tempScore[1] = success ? 1.5 : -0.5;
		tempScore[2] = success ? 2 : -1;
		tempScore[3] = success ? 2.5 : -1.5;
		tempScore[5] = success ? -1 : 0;
	}
};

class Qt16 : public Question   // [待定]靠后选项无人选择
{
public:
	Qt16()
	{
		id = 16;
		author = "克里斯丁";
		title = "贪婪的代价";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		vars["num"] = playerNum * 2;
		texts.push_back(RC(
			"克里斯丁看到你可怜的分数大发慈悲，决定施舍你 " + str(vars["num"]) + " 分。若索取的总分数超了则一分也得不到。你决定",
			"克里斯丁看到你较低的分数决定援助你 " + str(vars["num"]) + " 分。若索取的总分数超了则一分也得不到。你决定"));
	}
	virtual void initOptions() override
	{
        options.push_back(RC("不取意外之财，克里斯丁赞赏你的品德并赠送2分", "不取意外之财，克里斯丁赞赏你的态度并赠送2分"));
        options.push_back("取3分");
		options.push_back("取4分");
		options.push_back("取5分");
		options.push_back("取6分");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabbbccdde");
	}
	virtual void calc(vector<Player>& players) override
	{
		bool success = optionCount[1]*3 + optionCount[2]*4 + optionCount[3]*5 + optionCount[4]*6 <= vars["num"];
		tempScore[0] = 2;
		tempScore[1] = success ? 3 : 0;
		tempScore[2] = success ? 4 : 0;
		tempScore[3] = success ? 5 : 0;
		tempScore[4] = success ? 6 : 0;
	}
};

class Qt17 : public Question   // [待定]题目阅读时间较长
{
public:
	Qt17()
	{
		id = 17;
		author = "大红大紫";
		title = "外星危机";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("分别记每个选项的人数为abcd");
	}
	virtual void initOptions() override
	{
        options.push_back("防御：若a+c>b+d防御成功A+2且B+1，否则A-1");
        options.push_back("逃亡：若b+c>a+d成功逃亡B+1.5，否则B-0.5");
		options.push_back("中立：C+1，若成功防御或逃亡C+0.5（可叠加）");
		options.push_back("背叛：有人选D时ABC都额外-1。若成功逃亡或防御，D-1.5；若同时逃亡和防御，D-3.5");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbcccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		bool defense = optionCount[0] + optionCount[2] > optionCount[1] + optionCount[3];
		bool escape = optionCount[1] + optionCount[2] > optionCount[0] + optionCount[3];
		if (defense) {
			tempScore[0] += 2;
			tempScore[1] += 1;
		} else {
			tempScore[0] -= 1;
		}
		if (escape) {
			tempScore[1] += 1.5;
		} else {
			tempScore[1] -= 0.5;
		}
		tempScore[2] = 1 + (defense + escape) * 0.5;
		if (optionCount[3] > 0) {
			tempScore[0] -= 1;
			tempScore[1] -= 1;
			tempScore[2] -= 1;
			if (defense && escape) {
				tempScore[3] = -3.5;
			} else if (defense || escape) {
				tempScore[3] = -1.5;
			}
		}
	}
};

class Qt18 : public Question   // [待定]暂时无法确定选项平衡性
{
public:
    Qt18()
    {
        id = 18;
        author = "剩菜剩饭";
        title = RC("绑架", "困局");
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back(RC(
        	"你们被绑架了，头目正在思考要不要杀你们（选项按照 A到E 依次结算）",
        	"你们陷入困境，对手正在思考要不要放你们离开（选项按照 A到E 依次结算）"));
    }

    virtual void initOptions() override
    {
        options.push_back("反抗：如果至少有一半人选择此项，+3分，否则-1分");
        options.push_back("服软：+2分");
        options.push_back(RC("卖人：+1分，选择的人数每比B多一人，B扣1分", "团结：+1分，选择的人数每比B多一人，B扣1分"));
        options.push_back("沉默：如果所有人中AB中扣分的人数达到一半，+2分");
        options.push_back("卧底：如果本选项选择的人数最少（不含0人），结算后加分最多的人都-3分");
    }

    virtual void initExpects() override
    {
        expects.push_back("aaabbbccde");
    }

    virtual void calc(vector<Player>& players) override
    {
        tempScore[0] = optionCount[0] >= playerNum / 2 ? 3 : -1;
        tempScore[1] = 2;
        tempScore[2] = 1;
		if (optionCount[2] > optionCount[1]) {
			tempScore[1] -= optionCount[2] - optionCount[1];
		}
		int deduct_num = 0;
		if (tempScore[0] > 0) deduct_num += optionCount[0];
		if (tempScore[1] > 0) deduct_num += optionCount[1];
		tempScore[3] = deduct_num >= playerNum / 2 ? 2 : 0;
		if (optionCount[4] == nonZero_minSelect) {
			int maxTemp = *max_element(tempScore.begin(), tempScore.begin() + 5);
			for (int i = 0; i < 5; i++) {
				if (tempScore[i] == maxTemp) {
					tempScore[i] -= 3;
				}
			}
		}
    }
};

class Qt19 : public Question   // [待定]题目过于复杂
{
public:
    Qt19()
    {
        id = 19;
        author = "剩菜剩饭";
        title = "狐狸觅食";
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back("狐狸要开始觅食了！分数大于等于中位数的玩家是“兔子”，反之是“狐狸”（目前中位数=" + str(medianScore) + "）");
        texts.push_back("“兔子”受到选项影响。若“兔子”所选选项的人数超过玩家的1/3（向上舍入）且有“狐狸”选择此项时，被吃掉，改为-2分");
        texts.push_back("“狐狸”不执行选项。若所选选项有“兔子”被吃，+2分，否则-1分");
    }

    virtual void initOptions() override
    {
        options.push_back("+2");
        options.push_back("+1");
        options.push_back("0");
        options.push_back("-1");
    }

    virtual void initExpects() override
    {
        expects.push_back("abcd");
    }

    virtual void calc(vector<Player>& players) override
    {
		const int score[4] = {2, 1, 0, -1};
		const int limit = ceil(playerNum / 3.0);
		for (int i = 0; i < 4; i++) {
			bool fox = false;
			bool eaten = false;
			for (const auto& player : players) {
				if (player.select == i && player.score < medianScore) {
					fox = true;
					break;
				}
			}
			for (auto& player : players) {
				if (player.select == i && player.score >= medianScore) {
					if (optionCount[i] > limit && fox) {
						player.score -= 2;
						eaten = true;
					} else {
						player.score += score[i];
					}
				}
			}
			for (auto& player : players) {
				if (player.select == i && player.score < medianScore) {
					player.score += eaten ? 2 : -1;
				}
			}
		}
    }
};

class Qt20 : public Question   // 备选题目
{
public:
    Qt20()
    {
        id = 20;
        author = "大梦我先觉";
        title = "金币抉择";
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back("初始金币为0，1金币1分。请选择以下选项");
    }

    virtual void initOptions() override
    {
        options.push_back("+1，金币+3，不平分金币");
        options.push_back("0，金币-1，参与金币平分");
        options.push_back("0（金币=0）/+2（金币>0）/-2（金币<0）");
        options.push_back("获得 [亏空金币数] 的数量（金币<0）");
    }

    virtual void initExpects() override
    {
        expects.push_back("aabbbbcddd");
    }

    virtual void calc(vector<Player>& players) override
    {
		int coins = optionCount[0] * 3 - optionCount[1];
		tempScore[0] = 1;
		tempScore[1] = coins / optionCount[1];
		tempScore[2] = coins > 0 ? 2 : (coins < 0 ? -2 : 0);
		tempScore[3] = coins < 0 ? -coins : 0;
    }
};

class Qt21 : public Question   // 备选题目   平衡 1
{
public:
    Qt21()
    {
        id = 21;
        author = "克里斯丁";
        title = RC("红楼梦", "双面宝镜");
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back(RC("红楼梦，你得到了风月宝鉴，你决定看", "你得到了一面双面宝镜，你决定看"));
		texts.push_back("（AB中选择人数少的那一项执行，一样便都不执行）");
    }

    virtual void initOptions() override
    {
        options.push_back("看正面：所有选择此项的玩家+1");
        options.push_back("看反面：令选择A项的玩家中得分最高的玩家-2");
    }

    virtual void initExpects() override
    {
        expects.push_back("aaaaaaaaab");
    }

    virtual void calc(vector<Player>& players) override
    {
		if (optionCount[0] < optionCount[1]) {
			tempScore[0] = 1;
		}
		if (optionCount[0] > optionCount[1]) {
			double top = -99999;
			for (const auto& player : players) {
				if (player.select == 0) {
					top = max(top, player.score);
				}
			}
			for (auto& player : players) {
				if (player.select == 0 && player.score == top) {
					player.score -= 2;
				}
			}
		}
    }
};

class Qt22 : public Question   // [待定]暂时无法确定选项平衡性   平衡 1
{
public:
    Qt22()
    {
        id = 22;
        author = "克里斯丁";
        title = RC("西游记", "西游四人组");
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back(RC("西游记，你选择成为", "西游四人组，你选择成为"));
    }

    virtual void initOptions() override
    {
        options.push_back(RC("唐僧：+2.5，若有妖怪且没有孙悟空则-1.5", "队长：+2.5，若有对手且没有大师兄则-1.5"));
        options.push_back(RC(
        	"孙悟空：我要变强。分数最低者选择该选项+1.5，其他人选择该选项-3",
        	"大师兄：我要变强。分数最低者选择该选项+1.5，其他人选择该选项-3"));
		options.push_back(RC("猪八戒：+0.5", "二师兄：+0.5"));
		options.push_back(RC("妖怪：不得分", "对手：不得分"));
    }

    virtual void initExpects() override
    {
        expects.push_back("aaabccccccccddd");
    }

    virtual void calc(vector<Player>& players) override
    {
		tempScore[0] = (optionCount[3] > 0 && optionCount[1] == 0) ? -1.5 : 2.5;
		for (auto& player : players) {
			if (player.select == 1) {
				if (player.score == minScore) {
					player.score += 1.5;
				} else {
					player.score -= 3;
				}
			}
		}
		tempScore[2] = 0.5;
    }
};

class Qt23 : public Question   // [待定]暂时无法确定选项平衡性   平衡 1
{
public:
    Qt23()
    {
        id = 23;
        author = "克里斯丁";
        title = RC("水浒传", "邻里风波");
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back(RC("水浒传，你选择成为", "邻里风波，你选择成为"));
    }

    virtual void initOptions() override
    {
        options.push_back("观众：+0.5");
        options.push_back(RC("武大郎：+3，若武松在场改为-2", "当事人：+3，若仲裁者在场改为-2"));
		options.push_back(RC(
			"武松：乃是魔星下凡，得分最低者选此项+1.5，其他人选择该项-3",
			"仲裁者：乃是关键角色，得分最低者选此项+1.5，其他人选择该项-3"));
    }

    virtual void initExpects() override
    {
        expects.push_back("aabbbbc");
    }

    virtual void calc(vector<Player>& players) override
    {
		tempScore[0] = 0.5;
		tempScore[1] = optionCount[2] > 0 ? -2 : 3;
		for (auto& player : players) {
			if (player.select == 2) {
				if (player.score == minScore) {
					player.score += 1.5;
				} else {
					player.score -= 3;
				}
			}
		}
    }
};

class Qt24 : public Question	// [待修改]分数增减幅度太大
{
public:
    Qt24()
    {
        id = 24;
        author = "克里斯丁";
        title = RC("三国演义", "三方鼎立");
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back(RC(
        	"三国演义，你选择成为（0人选择或两股势力选择人数相同均视作势力覆灭，不会纳入最终的人数结算）",
        	"三方鼎立，你选择成为（0人选择或两股势力选择人数相同均视作势力覆灭，不会纳入最终的人数结算）"));
    }

    virtual void initOptions() override
    {
        options.push_back(RC("刘备：若其势力人数最少则A+6，B-4，C+2", "甲势力：若其势力人数最少则A+6，B-4，C+2"));
        options.push_back(RC("曹操：若其势力人数最多则A-3，B+3，C-3", "乙势力：若其势力人数最多则A-3，B+3，C-3"));
		options.push_back(RC("孙权：若其势力人数第二多或第二少则A-5，B+2，C+2", "丙势力：若其势力人数第二多或第二少则A-5，B+2，C+2"));
    }

    virtual void initExpects() override
    {
        expects.push_back("abc");
    }

    virtual void calc(vector<Player>& players) override
    {
		if (optionCount[0] == optionCount[2]) {
			tempScore[0] = -3;
			tempScore[1] = 3;
			tempScore[2] = -3;
		} else if (optionCount[1] == optionCount[2]) {
			tempScore[0] = 6;
			tempScore[1] = -4;
			tempScore[2] = 2;
		} else if (optionCount[0] != optionCount[1]) {
			if (optionCount[0] == nonZero_minSelect) {
				tempScore[0] += 6;
				tempScore[1] -= 4;
				tempScore[2] += 2;
			}
			if (optionCount[1] == maxSelect) {
				tempScore[0] -= 3;
				tempScore[1] += 3;
				tempScore[2] -= 3;
			}
			if (((optionCount[2] != maxSelect && optionCount[2] != minSelect) || optionCount[0] == 0 || optionCount[1] == 0) && optionCount[2] > 0) {
				tempScore[0] -= 5;
				tempScore[1] += 2;
				tempScore[2] += 2;
			}
		}
    }
};

class Qt25 : public Question   // [待修改]D选项得分过高   平衡 1
{
   public:
    Qt25()
    {
        id = 25;
        author = "圣墓上的倒吊人";
        title = "奇珍异宝";
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back("选择选项的玩家平分选项对应的分数");
    }
    virtual void initOptions() override
    {
        options.push_back("珍珠手链（2）");
        options.push_back("银质餐具（3）");
        options.push_back("国王宝球（5）");
        options.push_back("奇怪的书（选择该选项的玩家数的平方-2）");
    }
    virtual void initExpects() override
    {
        expects.push_back("abbccccdd");
    }
    virtual void calc(vector<Player>& players) override
    {
        tempScore[0] = 2 / optionCount[0];
        tempScore[1] = 3 / optionCount[1];
        tempScore[2] = 5 / optionCount[2];
        tempScore[3] = (optionCount[3] * optionCount[3] - 2) / optionCount[3];
    }
};

// 开发者: An idle brain Qt26 - Qt30
class Qt26 : public Question   // [待定]暂时无法确定选项平衡性
{
public:
	Qt26()
	{
		id = 26;
		author = "栗子";
		title = "贫富差距";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("在统计分数时，每个玩家分数都取绝对值后再计算总分");
	}

	virtual void initOptions() override
	{
		options.push_back("选择该选项玩家总分≤玩家总分*0.75，+2；否则-1");
		options.push_back("选择该选项玩家总分≤玩家总分*0.5，+3；否则-2");
		options.push_back("选择该选项玩家总分≤玩家总分*0.25，+4；否则-3");
	}

	virtual void initExpects() override
	{
		expects.push_back("aaabbc");
	}

	virtual void calc(vector<Player>& players) override
	{
		double scoreA = 0, scoreB = 0, scoreC = 0, sum = 0;
		for (auto& player : players) {
			if (player.select == 0) {
				scoreA += fabs(player.score);
			} else if (player.select == 1) {
				scoreB += fabs(player.score);
			} else if (player.select == 2) {
				scoreC += fabs(player.score);
			}
			sum += fabs(player.score);
		}
		if (scoreA * 1.0  <= 0.75 * sum) {
			tempScore[0] = 2;
		} else {
			tempScore[0] = -1;
		}
		if (scoreB * 1.0 <= 0.5 * sum) {
			tempScore[1] = 3;
		} else {
			tempScore[1] = -2;
		}
		if (scoreC * 1.0 <= 0.25 * sum) {
			tempScore[2] = 4;
		} else {
			tempScore[2] = -3;
		}
	}
};

class Qt27 : public Question   // [待定]暂时无法确定选项平衡性
{
public:
	Qt27()
	{
		id = 27;
		author = "克里斯丁";
		title = RC("海盗纷争", "强者纷争");
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back(RC("遇到一波海盗（共 " + str(playerNum) + " 人），你选择", "遇到一波强者（共 " + str(playerNum) + " 人），你选择"));
	}

	virtual void initOptions() override
	{
		options.push_back(RC(
			"加入，海盗人数+1，若海盗胜利则瓜分选择战斗和逃跑的人数两倍的分数。若无人战斗则所有海盗-1",
			"加入，强者人数+1，若强者胜利则瓜分选择对抗和逃跑的人数两倍的分数。若无人对抗则所有强者-1"));
		options.push_back(RC("战斗，杀死一个海盗，若战胜海盗+3，否则-1", "对抗，击退一个强者，若战胜强者+3，否则-1"));
		options.push_back(RC("战斗，杀死三个海盗，若战胜海盗+2，否则-1", "对抗，击退三个强者，若战胜强者+2，否则-1"));
		options.push_back(RC("战斗，杀死五个海盗，若战胜海盗+1，否则-1", "对抗，击退五个强者，若战胜强者+1，否则-1"));
		options.push_back("逃跑");
	}

	virtual void initExpects() override
	{
		expects.push_back("aaaaaaaabbbbcccdde");
	}

	virtual void calc(vector<Player>& players) override
	{
		double pirate = optionCount[0] + playerNum;
		double killPirate = optionCount[1] + optionCount[2] * 3 + optionCount[3] * 5;
		if(pirate > killPirate) {
			tempScore[0] = playerNum * 2.0 / optionCount[0] - 2;
			tempScore[1] = -1;
			tempScore[2] = -1;
			tempScore[3] = -1;
		} else {
			// 注：克里斯丁：相等算作战斗者胜利
			tempScore[0] = -1;
			tempScore[1] = 3;
			tempScore[2] = 2;
			tempScore[3] = 1;
		}
	}
};

class Qt28 : public Question   // [待定]暂时无法确定选项平衡性
{
public:
	Qt28()
	{
		id = 28;
		author = "纸团OvO";
		title = "隹投票最精确？";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("共有甲乙丙丁四个对象，所有玩家将投出自己选项内的票，统计完票后，每位投票中包括“最高票的对象（可并列）”的玩家获得对应分数");
	}

	virtual void initOptions() override
	{
		options.push_back("不投票直接+0.5分");
		options.push_back("甲乙丙,+1分");
		options.push_back("甲乙,+2分");
		options.push_back("甲丁,+2分");
		options.push_back("丙丁,+2分");
		options.push_back("丙,+3分");
		options.push_back("丁,+3分");
	}

	virtual void initExpects() override
	{
		expects.push_back("abbcccddddeeeffffffgggggg");
	}

	virtual void calc(vector<Player>& players) override
	{
		double jia = 0, yi = 0, bing = 0, ding = 0;
		jia = optionCount[1] + optionCount[2] + optionCount[3];
		yi = optionCount[1] + optionCount[2];
		bing = optionCount[1] + optionCount[4] + optionCount[5];
		ding = optionCount[3] + optionCount[4] + optionCount[6];
		// tempScore 表示每个选项的分数 optionCount 表示每个选项的人数
		tempScore[0] = 0.5;

		double maxVote = max({jia, yi, bing, ding});
		// 标记最高票的对象
		bool topJia = abs(jia - maxVote) < 1e-6;
		bool topYi = abs(yi - maxVote) < 1e-6;
		bool topBing = abs(bing - maxVote) < 1e-6;
		bool topDing = abs(ding - maxVote) < 1e-6;
		if (topJia || topYi || topBing) {
			tempScore[1] = 1;
		}
		if (topJia || topYi) {
			tempScore[2] = 2;
		}
		if (topJia || topDing) {
			tempScore[3] = 2;
		}
		if (topBing || topDing) {
			tempScore[4] = 2;
			if (topBing) {
				tempScore[5] = 3;
			}
			if (topDing) {
				tempScore[6] = 3;
			}
		}
	}
};

class Qt29 : public Question   // [待定]暂时无法确定选项平衡性
{
public:
	Qt29()
	{
		id = 29;
		author = "aka展博";
		title = "传说宝藏";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("你和你的队伍正在寻找一座传说中的宝藏，每个选项代表不同的行动策略。选择人数最多的选项将决定整个队伍的行动方式，并根据队伍的整体表现获得相应的分数。");
	}

	virtual void initOptions() override
	{
		options.push_back("潜行探索:+2分。如果人数多于B，额外获得1分；如果人数少于B，额外失去1分。");
		options.push_back("直接挖掘:+1分。每有1人选择，本选项得分增加0.5分");
		options.push_back("寻求帮助:-1分。如果人数多于D，改为2分");
		options.push_back("保持警惕:-2分。如果人数少于C，改为2分");
	}

	virtual void initExpects() override
	{
		expects.push_back("aaabbbbbbbbbbbbccd");
	}

	virtual void calc(vector<Player>& players) override
	{
		// 统计每个选项的人数
		double a = optionCount[0];
		double b = optionCount[1];
		double c = optionCount[2];
		double d = optionCount[3];

		// A
		tempScore[0] = 2;
		if (a > b) {
			tempScore[0] += 1;
		}
		else if (a < b) {
			tempScore[0] -= 1;
		}

		// B
		tempScore[1] = 1 + b * 0.5;

		// C
		if (c > d) {
			tempScore[2] = 2;
		}
		else {
			tempScore[2] = -1;
		}

		// D
		if (d < c) {
			tempScore[3] = 2;
		}
		else {
			tempScore[3] = -2;
		}
	}
};

class Qt30 : public Question   // [待定]暂时无法确定选项平衡性
{
public:
	Qt30()
	{
		id = 30;
		author = "aka展博";
		title = "未命名";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("在一个绿洲中，所有玩家被困在了这里，在这里也仅剩下 " + str(playerNum * 3) + " 分的物资，根据ABCD的顺序依次结算。");
	}

	virtual void initOptions() override
	{
		options.push_back("获取选择该选项人数*1分物资。如果物资不够，则都不得分");
		options.push_back("获取选择该选项人数*0.5分物资。如果物资不够，则都不得分");
		options.push_back("-2分，并给绿洲增加2分物资。如果选择该选项人数比A多，改为+3分（不再提供物资）");
		options.push_back("如果资源池里的分数最后大于4，+3分；但如果选择该选项人数比B多，-2分。");
	}

	virtual void initExpects() override
	{
		expects.push_back("aaaaabbbcdd");
	}

	virtual void calc(vector<Player>& players) override
	{
		double resource = playerNum * 3;
		double a = optionCount[0];
		double b = optionCount[1];
		double c = optionCount[2];
		double d = optionCount[3];

		// A结算
		if (resource >= a * a) {
			tempScore[0] = a;
			resource -= a * a;
		} else {
			tempScore[0] = 0;
		}

		// B结算
		if (resource >= b * b * 0.5) {
			tempScore[1] =  b * 0.5;
			resource -= b * b * 0.5;
		}
		else {
			tempScore[1] = 0;
		}

		// C结算
		if (c > a) {
			tempScore[2] = 3;
		}
		else {
			tempScore[2] = -2;
			resource += 2;
		}

		// D结算
		if (d > b) {
			tempScore[3] = -2;
		} else {
			if (resource > 4) {
				tempScore[3] = 3;
			}
			else {
				tempScore[3] = 0;
			}
		}
	}
};

// 开发者：问号
class Qt31 : public Question   // [待定]题目过于复杂
{
public:
	Qt31()
	{
		id = 31;
		author = "纤光";
		title = "潘多拉魔盒";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("ABC中人数相对最多的一项生效");
		texts.push_back("DEF仅在不超过2人选择时生效");
        texts.push_back("所有不生效的选项-0.5");
	}
	virtual void initOptions() override
	{
		options.push_back("A+1，B-1");
		options.push_back("B+1，C+1");
		options.push_back("C-1，A-1");
		options.push_back("+1，每有一人选此项，本题所有得分翻一次倍");
		options.push_back("-1，每有一人选此项，本题所有得分取一次反");
		options.push_back("每有一人选此项，A与B便互换一次效果");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabbbcccdef");
	}
	virtual void calc(vector<Player>& players) override
	{
		bool shen_xiao[6] = {0};
		if (optionCount[0] >= optionCount[1] && optionCount[0] >= optionCount[2]) shen_xiao[0] = 1;
		if (optionCount[1] >= optionCount[0] && optionCount[1] >= optionCount[2]) shen_xiao[1] = 1;
		if (optionCount[2] >= optionCount[1] && optionCount[2] >= optionCount[0]) shen_xiao[2] = 1;
		for (int i = 3; i < 6; i++) {
			if(optionCount[i] <= 2) shen_xiao[i] = 1;
		}

		for (int i = 0; i < 6; i++) {
			if(!shen_xiao[i]) tempScore[i] = -0.5;
			else tempScore[i] = 0;
		}

		if (!optionCount[5]) {
			if (shen_xiao[0]) {
				tempScore[0]++;
				tempScore[1]--;
			}
			if (shen_xiao[1]) {
				tempScore[1]++;
				tempScore[2]++;
			}
		} else {
			if(shen_xiao[1]) {
				tempScore[0]++;
				tempScore[1]--;
			}
			if (shen_xiao[0]) {
				tempScore[1]++;
				tempScore[2]++;
			}
		}
		if(shen_xiao[2]) {
			tempScore[2]--;
			tempScore[0]--;
		}
		if(shen_xiao[3]) {
			tempScore[3]++;
		}
		if(shen_xiao[4]) {
			tempScore[4]--;
		}

		if (shen_xiao[3] && optionCount[3]) {
			for (int i = 0; i < 6; i++) {
				if (optionCount[3] == 1) {
					tempScore[i] *= 2;
				} else {
					tempScore[i] *= 4;
				}
			}
		}

		if (shen_xiao[4] == 1) {
			for(int i = 0; i < 6; i++) {
				tempScore[i] = -tempScore[i];
			}
		}
	}
};

// 开发者：苣屋逊太郎
class Qt32 : public Question   // [待定]暂时无法确定选项平衡性
{
public:
    Qt32()
    {
        id = 32;
        author = "苣屋逊太郎";
        title = "绝望推杆";
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back("A和B两队分别有一根杆子。哪方有更多进攻者成功抵达敌方杆子，哪方就获得胜利。数量相等则无队胜利");
    }
    virtual void initOptions() override
	{
        options.push_back("加入A队进攻B杆：-2，若己方胜利且存在神则+4");
        options.push_back("加入B队进攻A杆：-2，若己方胜利且存在神则+4");
        options.push_back("加入A队防守：拦截一名敌方的进攻者。若己方胜利，+2，否则-2");
        options.push_back("加入B队防守：拦截一名敌方的进攻者。若己方胜利，+2，否则-2");
        options.push_back("神：若无队胜利，+2，否则每有一位胜利方的进攻者抵达杆子扣1分");
    }
    virtual void initExpects() override
    {
        expects.push_back("aabbcccdddeee");
    }
    virtual void calc(vector<Player>& players) override
    {
        int n1 = max(static_cast<int>(optionCount[0] - optionCount[3]), 0);
        int n2 = max(static_cast<int>(optionCount[1] - optionCount[2]), 0);
        if (n1 > n2)
        {
            if (optionCount[4] > 0) tempScore[0] = 4;
            else tempScore[0] = -2;
            tempScore[1] = -2;
            tempScore[2] = 2;
            tempScore[3] = -2;
            tempScore[4] = (-1)*n1;
        }
        else if (n1 < n2)
        {
            if (optionCount[4] > 0) tempScore[1] = 4;
            else tempScore[1] = -2;
            tempScore[0] = -2;
            tempScore[3] = 2;
            tempScore[4] = (-1)*n2;
        }
        else
        {
            tempScore[0] = -2;
            tempScore[1] = -2;
            tempScore[2] = -2;
            tempScore[3] = -2;
            tempScore[4] = 2;
        }
    }
};

class Qt33 : public Question   // [待定]暂时无法判断选项平衡性
{
   public:
    Qt33()
    {
        id = 33;
        author = "圣墓上的倒吊人";
        title = "站位";
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back("赢的玩家+2，输的玩家-2。");
        texts.push_back("同时胜利或无人胜利时B胜利，AC输");
    }
    virtual void initOptions() override
    {
        vars["num"] = int(playerNum / 2);
        options.push_back("B人数大于 " + str(vars["num"]) + " 则胜利");
        options.push_back("中立");
        options.push_back("A人数大于B则胜利，否则输");
    }
    virtual void initExpects() override
    {
        expects.push_back("aabbbc");
    }
    virtual void calc(vector<Player>& players) override
    {
        bool a_win = optionCount[1] > vars["num"];
        bool c_win = optionCount[0] > optionCount[1];
        if (a_win && !c_win) {
            tempScore[0] = 2;
            tempScore[2] = -2;
        } else if (!a_win && c_win) {
            tempScore[0] = -2;
            tempScore[2] = 2;
        } else {
            tempScore[0] = -2;
            tempScore[1] = 2;
            tempScore[2] = -2;
        }
    }
};

class Qt34 : public Question   // [待定]暂时无法确定选项平衡性
{
public:
	Qt34()
	{
		id = 34;
		author = "Yy";
		title = "炼金术";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项：");
	}
	virtual void initOptions() override
	{
		options.push_back("平分 [C选项人数×2] 分，并使选择C的玩家各-2");
		options.push_back("平分 [A选项人数×1.5] 分，并使选择A的玩家各-1.5");
		options.push_back("平分 [B选项人数×1] 分，并使选择B的玩家各-1");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbcc");
	}
	virtual void calc(vector<Player>& players) override
	{
		if (optionCount[0] > 0) {
			tempScore[0] += optionCount[2] * 2 / optionCount[0];
			tempScore[2] -= 2;
		}
		if (optionCount[1] > 0) {
			tempScore[1] += optionCount[0] * 1.5 / optionCount[1];
			tempScore[0] -= 1.5;
		}
		if (optionCount[2] > 0) {
			tempScore[2] += optionCount[1] * 1 / optionCount[2];
			tempScore[1] -= 1;
		}
	}
};

class Qt35 : public Question   // [待定]暂时无法确定选项平衡性
{
public:
	Qt35()
	{
		id = 35;
		author = "飘渺";
		title = "团结的力量";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项：");
	}
	virtual void initOptions() override
	{
		options.push_back("镇民：每有一个镇民，镇民+1，外来者+0.5");
		options.push_back("外来者：每有一个外来者，镇民-0.5，外来者+0.5");
		options.push_back("爪牙：每有一个爪牙+0.5。若爪牙比镇民多，每个爪牙使镇民和外来者各-0.5");
		options.push_back("恶魔：本组总分为 [每有一个爪牙+2，每有一个镇民-1，每有一个恶魔或外来者+0.5]，由所有恶魔平分");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabbccd");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] += optionCount[0];
		tempScore[1] += optionCount[0] * 0.5;

		tempScore[0] -= optionCount[1] * 0.5;
		tempScore[1] += optionCount[1] * 0.5;

		tempScore[2] += optionCount[2] * 0.5;
		if (optionCount[2] > optionCount[0]) {
			tempScore[0] -= optionCount[2] * 0.5;
			tempScore[1] -= optionCount[2] * 0.5;
		}

		if (optionCount[3] > 0) {
			double devil = optionCount[2] * 2 - optionCount[0]
				+ (optionCount[3] + optionCount[1]) * 0.5;
			tempScore[3] = devil / optionCount[3];
		}
	}
};

class Qt36 : public Question
{
public:
	Qt36()
	{
		id = 36;
		author = "克里斯丁";
		title = "四国大战";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("四国混战，战力最大的阵营获胜并平分等同于总人数的分数，若战力最大的阵营数不唯一则无人胜利。正常情况下1人1战力。你选择");
	}
	virtual void initOptions() override
	{
		options.push_back("加入红国，不管是否胜利每人+1");
		options.push_back("加入黄国，若黄国只有一人则其战力加到100");
		options.push_back("加入蓝国，若蓝国的人数为四国中唯一最少的，蓝国所有人都战力均加到100");
		options.push_back("加入绿国，-1分，然后若ABC中存在胜者，与胜者一同平分分数");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabbccddd");
	}
	virtual void calc(vector<Player>& players) override
	{
		vector<double> power = optionCount;
		if (optionCount[1] == 1) power[1] = 100;
		if (optionCount[2] < optionCount[0] && optionCount[2] < optionCount[1] && optionCount[2] < optionCount[3]) {
			power[2] = 100 * optionCount[2];
		}
		int winner = 0;
		for (int i = 1; i < 4; i++) {
			if (power[i] > power[winner]) winner = i;
		}
		tempScore[0] = 1;
		tempScore[3] = -1;
		if (count(power.begin(), power.end(), power[winner]) > 1) return;
		// 绿国与 ABC 中的胜者一同平分
		const double share = playerNum / (optionCount[winner] + (winner == 3 ? 0 : optionCount[3]));
		tempScore[winner] += share;
		if (winner != 3) tempScore[3] += share;
	}
};

class Qt37 : public Question
{
public:
	Qt37()
	{
		id = 37;
		author = "大梦我先觉";
		title = "条件链";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项平分。X 为每项各自人数");
	}
	virtual void initOptions() override
	{
		options.push_back("2分（X大于1时，改为平分4分）");
		options.push_back("5分（只获得5/X的取整分）");
		options.push_back("8分（偶数人方可平分）");
		options.push_back("3分（仅1人选，其余选项后缀条件失效）");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbbccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		// D 仅 1 人选时 ABC 的后缀条件失效
		const bool plain = optionCount[3] == 1;
		tempScore[0] = (plain || optionCount[0] <= 1 ? 2 : 4) / optionCount[0];
		tempScore[1] = plain ? 5 / optionCount[1] : floor(5 / optionCount[1]);
		tempScore[2] = plain || (int)optionCount[2] % 2 == 0 ? 8 / optionCount[2] : 0;
		tempScore[3] = 3 / optionCount[3];
	}
};

class Qt38 : public Question
{
public:
	Qt38()
	{
		id = 38;
		author = "大梦我先觉";
		title = "投资与贬值";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("玩家投资一项，根据人数获得对应分（X为该选项人数），当X满足括号选项，将贬值为0.5X");
	}
	virtual void initOptions() override
	{
		options.push_back("-1，+X（X≥4）");
		options.push_back("-2，+2X（X≥4）");
		options.push_back("-3，+2.5X（X≥4）");
		options.push_back("-4，+3X（X≥4）");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		const double rate[4] = {1, 2, 2.5, 3};
		for (int i = 0; i < 4; i++) {
			tempScore[i] = -(i + 1) + (optionCount[i] >= 4 ? 0.5 : rate[i]) * optionCount[i];
		}
	}
};

class Qt39 : public Question
{
public:
	Qt39()
	{
		id = 39;
		author = "大梦我先觉";
		title = "三人小队";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("若ABC均有人，全体+2；若某选项达到3人，该选项每人额外-2；");
		texts.push_back("人数在有人选择的选项里最少（包括并列）的选项每人额外+1。");
	}
	virtual void initOptions() override
	{
		options.push_back("君王");
		options.push_back("军师");
		options.push_back("将士");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbcc");
	}
	virtual void calc(vector<Player>& players) override
	{
		const bool full = optionCount[0] > 0 && optionCount[1] > 0 && optionCount[2] > 0;
		for (int i = 0; i < 3; i++) {
			tempScore[i] = (full ? 2 : 0) - (optionCount[i] >= 3 ? 2 : 0) + (optionCount[i] == nonZero_minSelect ? 1 : 0);
		}
	}
};

class Qt40 : public Question
{
public:
	Qt40()
	{
		id = 40;
		author = "大梦我先觉";
		title = "钥匙与锁";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		vars["key"] = playerNum >= 10 ? 3 : 2;
		texts.push_back("开锁成功：若A、B均至少" + str(vars["key"]) + "人且C有人，则C+3,AB各+1");
		texts.push_back("开锁失败：①无人选C；②A、B人数有一方少于" + str(vars["key"]) + "人；");
		texts.push_back("出现以上情况则钥匙较少一方每人+2，C-2。");
		texts.push_back("特殊结算：若A=B，无视开锁条件，仅C+1");
	}
	virtual void initOptions() override
	{
		options.push_back("红钥匙。");
		options.push_back("蓝钥匙。");
		options.push_back("开锁。");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabbbcc");
	}
	virtual void calc(vector<Player>& players) override
	{
		const double a = optionCount[0], b = optionCount[1];
		if (a == b) {
			tempScore[2] = 1;
		} else if (a >= vars["key"] && b >= vars["key"] && optionCount[2] > 0) {
			tempScore[0] = 1;
			tempScore[1] = 1;
			tempScore[2] = 3;
		} else {
			tempScore[a < b ? 0 : 1] = 2;
			tempScore[2] = -2;
		}
	}
};

class Qt41 : public Question
{
public:
	Qt41()
	{
		id = 41;
		author = "大梦我先觉";
		title = "双向奔赴";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("该选项人数恰好为：1人则-1分；2人则各+3分，3人则各-2分，其余为0分");
	}
	virtual void initOptions() override
	{
		options.assign(3, "");
	}
	virtual void initExpects() override
	{
		expects.push_back("abc");
	}
	virtual void calc(vector<Player>& players) override
	{
		for (int i = 0; i < 3; i++) {
			const int x = optionCount[i];
			tempScore[i] = x == 1 ? -1 : x == 2 ? 3 : x == 3 ? -2 : 0;
		}
	}
};

class Qt42 : public Question
{
public:
	Qt42()
	{
		id = 42;
		author = "大梦我先觉";
		title = "单调性";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("若 A、B、C、D 人数递增或递减：人数最少项 +4，第二少 +3，第三少 +2，最多项 +1。");
		texts.push_back("否则，若 ABC 或 BCD 人数递增/递减：人数最少项 +3，居中项 +2，最多项 +1。");
	}
	virtual void initOptions() override
	{
		options.assign(4, "");
	}
	virtual void initExpects() override
	{
		expects.push_back("abcd");
	}
	virtual void calc(vector<Player>& players) override
	{
		// [from, to] 严格单调时，按人数从少到多依次得 top、top-1…
		auto settle = [&](int from, int to, double top) {
			const bool inc = optionCount[from] < optionCount[from + 1];
			for (int i = from; i < to; i++) {
				if (inc ? optionCount[i] >= optionCount[i + 1] : optionCount[i] <= optionCount[i + 1]) return false;
			}
			for (int i = from; i <= to; i++) {
				tempScore[i] = top - (inc ? i - from : to - i);
			}
			return true;
		};
		if (!settle(0, 3, 4) && !settle(0, 2, 3)) settle(1, 3, 3);
	}
};

class Qt43 : public Question
{
public:
	Qt43()
	{
		id = 43;
		author = "大梦我先觉";
		title = "婚姻生活";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
		options.push_back("单身，若A人数为奇数，每人+3；若B人数为偶数，每人再-2。");
		options.push_back("婚姻，若B人数为偶数，每人+2；若A人数为奇数，每人再-1。");
		options.push_back("第三者，若B人数为偶数，每人+（C人数）分；若B人数为奇数，每人-（A人数）分。");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbbc");
	}
	virtual void calc(vector<Player>& players) override
	{
		const int a = optionCount[0], b = optionCount[1], c = optionCount[2];
		tempScore[0] = (a % 2 == 1 ? 3 : 0) - (b % 2 == 0 ? 2 : 0);
		tempScore[1] = (b % 2 == 0 ? 2 : 0) - (a % 2 == 1 ? 1 : 0);
		tempScore[2] = b % 2 == 0 ? c : -a;
	}
};

class Qt44 : public Question
{
public:
	Qt44()
	{
		id = 44;
		author = "大梦我先觉";
		title = "函数来袭";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("x为每项人数，函数结果为你的分数，取两位小数点");
	}
	virtual void initOptions() override
	{
		options.push_back("-(x-1)<sup>2</sup>+3");
		options.push_back("5-x·ln x");
		options.push_back("sin x+1");
		options.push_back("x<sup>-x</sup>+1");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		const auto& x = optionCount;
		tempScore[0] = dec2(3 - (x[0] - 1) * (x[0] - 1));
		tempScore[1] = x[1] > 0 ? dec2(5 - x[1] * log(x[1])) : 0;  // ln 0 无定义
		tempScore[2] = dec2(sin(x[2]) + 1);
		tempScore[3] = dec2(pow(x[3], -x[3]) + 1);
	}
};

class Qt45 : public Question
{
public:
	Qt45()
	{
		id = 45;
		author = "大梦我先觉";
		title = "九宫格";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("九宫格内，共有横、竖、两条对角线共8条线，请选择一个数字。");
		texts.push_back("加分：一条线至少2格有人，该线为成立线，成立线上的人各+1.5。若3格全满，该线玩家额外+1。");
		texts.push_back("扣分：某格选择人数大于1人，该格X人额外-（X-1）分。若存在未成立的线，人数最多的格每人额外-1");
		string grid = "<table align=\"center\" cellpadding=\"12\" style=\"text-align:center\">";
		for (int i = 0; i < 9; i++) {
			if (i % 3 == 0) grid += "<tr>";
			grid += "<td bgcolor=\"#F5F5F5\"><font size=6>" + string(1, 'A' + i) + to_string(i + 1) + "</font></td>";
			if (i % 3 == 2) grid += "</tr>";
		}
		texts.push_back(grid + "</table>");
	}
	virtual void initOptions() override
	{
		for (int i = 1; i <= 9; i++) {
			options.push_back(str(i));
		}
	}
	virtual void initExpects() override
	{
		expects.push_back("abcdeefghi");
	}
	virtual void calc(vector<Player>& players) override
	{
		static const int lines[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};
		bool allFormed = true;
		for (const auto& line : lines) {
			int filled = 0;
			for (int cell : line) filled += optionCount[cell] > 0;
			if (filled < 2) {
				allFormed = false;
				continue;
			}
			for (int cell : line) tempScore[cell] += filled == 3 ? 2.5 : 1.5;
		}
		for (int i = 0; i < 9; i++) {
			if (optionCount[i] > 1) tempScore[i] -= optionCount[i] - 1;
			if (!allFormed && optionCount[i] == maxSelect) tempScore[i] -= 1;
		}
	}
};

class Qt46 : public Question
{
public:
	Qt46()
	{
		id = 46;
		author = "大梦我先觉";
		title = "通勤竞速";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		vars["load"] = playerNum >= 10 ? 3 : 2;
		texts.push_back("最先到达者分数×1.6；第二到达者分数×1.3；最后到达者分数不变，同时到达按 A→B→C 排列");
		texts.push_back("（本题的乘法均作用于玩家当前分数）");
	}
	virtual void initOptions() override
	{
		options.push_back("公交车：时长为0.5X小时（X为该选项人数）");
		options.push_back("电动车：时长为1+ 0.25X小时（X为该选项人数）");
		options.push_back("单车：固定1.5h，超过" + str(vars["load"]) + "人负载失败不出发");
		options.push_back("散步：不参与比赛，若ABC均有人选且到达，分数*0.5；若有X项出发成功，+0.5X分");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		const double time[3] = {0.5 * optionCount[0], 1 + 0.25 * optionCount[1], 1.5};
		vector<int> racers;
		for (int i = 0; i < 3; i++) {
			if (optionCount[i] > 0 && (i != 2 || optionCount[2] <= vars["load"])) racers.push_back(i);
		}
		stable_sort(racers.begin(), racers.end(), [&](int l, int r) { return time[l] < time[r]; });
		const double rate[3] = {1.6, 1.3, 1};
		vector<double> multiplier(4, 1);
		for (int i = 0; i < racers.size(); i++) multiplier[racers[i]] = rate[i];
		if (racers.size() == 3) multiplier[3] = 0.5;
		for (auto& player : players) player.score *= multiplier[player.select];
		// 散步先乘后加
		tempScore[3] = 0.5 * racers.size();
	}
};

class Qt47 : public Question
{
public:
	Qt47()
	{
		id = 47;
		author = "大梦我先觉";
		title = "终焉之地";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("此处分布12生肖，分天地人三种，请选择你的身份");
	}
	virtual void initOptions() override
	{
		options.push_back("天：+6分，若 [B人数] 为12的因数，改为+6/[B人数]");
		options.push_back("地：+3分，若 [A人数] 为 [B人数] 的倍数，额外+1分");
		options.push_back("人：+1.5分，C≥D改为+0.5分");
		options.push_back("参与者：C＜D时+1分，且[C+D]≥[A+B]时，AB分数取反");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbbccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		const int a = optionCount[0], b = optionCount[1], c = optionCount[2], d = optionCount[3];
		tempScore[0] = b > 0 && 12 % b == 0 ? 6.0 / b : 6;
		tempScore[1] = b > 0 && a % b == 0 ? 4 : 3;
		tempScore[2] = c >= d ? 0.5 : 1.5;
		if (c < d) {
			tempScore[3] = 1;
			if (c + d >= a + b) {
				tempScore[0] = -tempScore[0];
				tempScore[1] = -tempScore[1];
			}
		}
	}
};

class Qt48 : public Question
{
public:
	Qt48()
	{
		id = 48;
		author = "大梦我先觉";
		title = "二十四节气";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("按照春→夏→秋→冬轮回X步（X为四个选项中的最低人数），即X除以4余1、2、3、0时分别在春、夏、秋、冬中断。");
		texts.push_back("中断季节的玩家各+X分，其余每个选项各自平分（7-X）分。");
	}
	virtual void initOptions() override
	{
		options.push_back("春");
		options.push_back("夏");
		options.push_back("秋");
		options.push_back("冬");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbccdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		const int x = minSelect;
		const int season = (x + 3) % 4;
		for (int i = 0; i < 4; i++) {
			tempScore[i] = i == season ? x : (7 - x) / optionCount[i];
		}
	}
};

class Qt49 : public Question
{
public:
	Qt49()
	{
		id = 49;
		author = "大萝卜姬";
		title = "漫漫长夜";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("作为漫漫长夜玩家，看见“口”的墙壁信息，你下意识觉得：");
	}
	virtual void initOptions() override
	{
		options.push_back(RC("经典区块，啪啪之神：平分-5分；选择该选项的玩家=1或5时，改为平分+5分。", "经典区块，啪嗒之神：平分-5分；选择该选项的玩家=1或5时，改为平分+5分。"));
		options.push_back("谁关的门：+5分；若C选项人数为奇数，则改为-3分。");
		options.push_back("谁被关了：-3；若B选项人数＞0且C选项人数为奇数，则改为+5。");
		options.push_back("小小邦邦，可笑可笑：+0分。");
		options.push_back("铁蛋游戏，没有bug：随机[-3,+3]。");
	}
	virtual void initExpects() override
	{
		expects.push_back("abbccdde");
	}
	virtual void calc(vector<Player>& players) override
	{
		const int a = optionCount[0], b = optionCount[1], c = optionCount[2];
		tempScore[0] = (a == 1 || a == 5 ? 5 : -5) / optionCount[0];
		tempScore[1] = c % 2 == 1 ? -3 : 5;
		tempScore[2] = b > 0 && c % 2 == 1 ? 5 : -3;
		// 所有选 E 的玩家共用一个 [-3, 3] 内两位小数的随机值
		tempScore[4] = (rand() % 601 - 300) / 100.0;
	}
};
