// 乌合之众题库（废弃题目，题号 d 开头）
// 文案 RC() 规则见 problems.h 顶部说明，本文件须在 problems.h 之后引入。

// 39题囚徒困境修改前两版备份（目前废弃）
class Qd1 : public Question
{
public:
	Qd1()
	{
		id = 1;
		author = "纤光";
		title = "囚徒困境第1版";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
        options.push_back("认罪：-1，但若所有人都认罪，改为分数最高的人分数清零");
        options.push_back("不认罪：-2，但若所有人都不认罪，改为+2");
	}
	virtual void initExpects() override
	{
		expects.push_back("ab");
	}
	virtual void calc(vector<Player>& players) override
	{
        if (optionCount[0] == players.size()) {
            const double max_score = MaxPlayerScore(players);
            for (auto& player : players) {
                if (player.score == max_score) {
                    player.score = 0;
                }
            }
        } else if (optionCount[1] == players.size()) {
            for (auto& player : players) {
                player.score += 2;
            }
        } else {
            tempScore[0] = -1;
            tempScore[1] = -2;
        }
	}
};

class Qd2 : public Question
{
public:
	Qd2()
	{
		id = 2;
		author = "纤光";
		title = "囚徒困境第2版";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
        options.push_back("认罪：-1，结算完成后，若你的分数大于选此项的人数，则分数清零");
        options.push_back("不认罪：-2，结算完成后，若你的分数小于选此项的人数，则分数清零");
	}
	virtual void initExpects() override
	{
		expects.push_back("ab");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = -1;
        tempScore[1] = -2;
		for (auto& player : players) {
			if (player.select == 0 && player.score > optionCount[0]) {
				player.score = 1;
			}
			if (player.select == 1 && player.score < optionCount[1]) {
				player.score = 2;
			}
		}
	}
};

class Qd3 : public Question
{
public:
	Qd3()
	{
		id = 3;
		author = "大梦我先觉";
		title = RC("战场的厮杀", "战场的对峙");
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("在战场上，一念之差都会扭转战局。阵营AB对立，CD对立，各阵营人数多的一方获胜，胜利者可以获得 [失败一方人数/2] 的分数");
	}
	virtual void initOptions() override
	{
        options.push_back(RC("劫营", "突进"));
        options.push_back(RC("守营", "防守"));
        options.push_back(RC("抢粮", "夺粮"));
		options.push_back("守粮");
	}
	virtual void initExpects() override
	{
		expects.push_back("abcd");
	}
	virtual void calc(vector<Player>& players) override
	{
        if (optionCount[0] > optionCount[1]) {
            tempScore[0] = optionCount[1] / 2;
			// tempScore[1] = -0.5;
        }
		if (optionCount[0] < optionCount[1]) {
            tempScore[1] = optionCount[0] / 2;
			// tempScore[0] = -0.5;
        }
		if (optionCount[2] > optionCount[3]) {
            tempScore[2] = optionCount[3] / 2;
			// tempScore[3] = -0.5;
        }
		if (optionCount[2] < optionCount[3]) {
            tempScore[3] = optionCount[2] / 2;
			// tempScore[2] = -0.5;
        }
	}
};

class Qd4 : public Question
{
public:
	Qd4()
	{
		id = 4;
		author = RC("蔡徐坤", "[数据删除]");
		title = "鉴宝师和收藏家";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		string half = "";
		if (playerNum > 5) {
			half = "的 一半 ";
		}
		texts.push_back("将选择对应选项玩家数" + half + "分别记为 A B C D");
	}
	virtual void initOptions() override
	{
        options.push_back("鉴定为假：+2D，若D>C，将所有选择A的玩家的积分平分给选择D的玩家，然后选A玩家分数清0");
        options.push_back("鉴定为真：若C>D，你-2，否则你+D");
		options.push_back("带来赝品：若B>A，你+2，否则你-A");
		options.push_back("带来真品：若A>B，你-2，否则你+B");
	}
	virtual void initExpects() override
	{
		expects.push_back("abcd");
	}
	virtual void calc(vector<Player>& players) override
	{
		vector<double> pnum;
		for (int i = 0; i < playerNum; i++) {
			if (playerNum > 5) {
				pnum.push_back(optionCount[i] / 2);
			} else {
				pnum.push_back(optionCount[i]);
			}
		}
		tempScore[0] = 2 * pnum[3];
		if (pnum[3] > pnum[2]){
			double sum_score = 0;
			for (int i = 0; i < playerNum; i++) {
				if (players[i].select == 0) {
					sum_score = sum_score + players[i].score + tempScore[0];
					players[i].score = 0;
				}
			}
			tempScore[0] = 0;
			tempScore[3] = sum_score / optionCount[3];
		}
		if (pnum[2] > pnum[3]) {
			tempScore[1] -= 2;
		} else {
			tempScore[1] += pnum[3];
		}
		if (pnum[1] > pnum[0]) {
			tempScore[2] += 2;
		} else {
			tempScore[2] -= pnum[0];
		}
		if (pnum[0] > pnum[1]) {
			tempScore[3] -= 2;
		} else {
			tempScore[3] += pnum[1];
		}
	}
};

class Qd5 : public Question
{
public:
	Qd5()
	{
		id = 5;
		author = "大梦我先觉";
		title = RC("焦灼的击剑", "焦灼的比武");
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back(RC(
			"a和b两人击剑时进行进攻和躲闪。a和b的最终行为分别由选择人数最多项确定（如果人数相等分数不变）。",
			"a和b两人比武时进行出招和闪避。a和b的最终行为分别由选择人数最多项确定（如果人数相等分数不变）。"));
		texts.push_back(RC(
			"若两人均进攻，则选进攻者-2分；两人均躲闪，则选躲闪者-1分。若一人进攻一人躲闪，则进攻方选进攻者+2，躲闪方分数不变。",
			"若两人均出招，则选出招者-2分；两人均闪避，则选闪避者-1分。若一人出招一人闪避，则出招方选出招者+2，闪避方分数不变。"));
	}
	virtual void initOptions() override
	{
        options.push_back(RC("a选择进攻", "a选择出招"));
        options.push_back(RC("a选择躲闪", "a选择闪避"));
        options.push_back(RC("b选择进攻", "b选择出招"));
		options.push_back(RC("b选择躲闪", "b选择闪避"));
	}
	virtual void initExpects() override
	{
		expects.push_back("abcd");
	}
	virtual void calc(vector<Player>& players) override
	{
		int a_action = (optionCount[0] > optionCount[1])? 1 : (optionCount[1] > optionCount[0])? 0 : -1;
		int b_action = (optionCount[2] > optionCount[3])? 1 : (optionCount[3] > optionCount[2])? 0 : -1;
		if (a_action == b_action) {
			if (a_action == 1) {
				tempScore[0] = tempScore[2] = -2;
			} else if (a_action == 0) {
				tempScore[1] = tempScore[3] = -1;
			}
		} else {
			// a无行动时，b攻击>躲闪加分，否则都不加
			if (a_action > b_action && a_action == 1) {
				tempScore[0] = 2;
			}
			if (b_action > a_action && b_action == 1) {
				tempScore[2] = 2;
			}
		}
	}
};

class Qd6 : public Question
{
public:
	Qd6()
	{
		id = 6;
		author = "剩菜剩饭";
		title = "HP杀";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back(RC("选择一项，若死亡则不得分，但仍执行攻击效果", "选择一项，若出局则不得分，但仍执行行动效果"));
	}
	virtual void initOptions() override
	{
        options.push_back(RC("大狼：+1分，每有一只小狼+1分。若大狼只有 1 只，神职死亡", "大狼：+1分，每有一只小狼+1分。若大狼只有 1 只，先知出局"));
        options.push_back(RC(
        	"小狼：+1分，选择的人数比神职多时平民死亡，若大狼存活，额外+1分",
        	"小狼：+1分，选择的人数比先知多时村民出局，若大狼存活，额外+1分"));
		options.push_back(RC("神职：+1分，选择人数比小狼多时，大狼死亡，此项额外+1分", "先知：+1分，选择人数比小狼多时，大狼出局，此项额外+1分"));
		options.push_back(RC("平民：+3分，若选择的人数比神职多，平民死亡", "村民：+3分，若选择的人数比先知多，村民出局"));
		options.push_back(RC("第三方：+0分，若狼人方和好人方都有阵亡消息，你+4分 ", "第三方：+0分，若狼组和村组都有出局消息，你+4分 "));
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbbcccddde");
	}
	virtual void calc(vector<Player>& players) override
	{
		bool A_die, B_die, C_die, D_die;
		A_die = B_die = C_die = D_die = false;
		tempScore[0] = 1 + optionCount[1];
		tempScore[1] = 1;
		tempScore[2] = 1;
		tempScore[3] = 3;
		if (optionCount[0] == 1) {
			C_die = true;
		}
		if (optionCount[1] > optionCount[2] || optionCount[3] > optionCount[2]) {
			D_die = true;
		}
		if (optionCount[2] > optionCount[1]) {
			A_die = true;
			tempScore[2] += 1;
		}
		if (A_die) tempScore[0] = 0; else tempScore[1] += 1;
		// if (B_die) tempScore[1] = 0;
		if (C_die) tempScore[2] = 0;
		if (D_die) tempScore[3] = 0;
		if ((A_die || B_die) && (C_die || D_die)) {
			tempScore[4] = 4;
		}
	}
};

class Qd7 : public Question
{
public:
	Qd7()
	{
		id = 7;
		author = "An idle brain";
		title = "未命名";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
        options.push_back("+1，若有人选B，则改为获得 [D-B] 人数的分数");
        options.push_back("+1.5，若有人选C，则改为获得 [A-C] 人数的分数");
		options.push_back("+2，若有人选D，则改为获得 [B-D] 人数的分数");
		options.push_back("+2.5，若有人选A，则改为获得 [C-A] 人数的分数");
	}
	virtual void initExpects() override
	{
		expects.push_back("abbcdd");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = 1;
		if (optionCount[1] > 0) {
			tempScore[0] = optionCount[3] - optionCount[1];
		}
		tempScore[1] = 1.5;
		if (optionCount[2] > 0) {
			tempScore[1] = optionCount[0] - optionCount[2];
		}
		tempScore[2] = 2;
		if (optionCount[3] > 0) {
			tempScore[2] = optionCount[1] - optionCount[3];
		}
		tempScore[3] = 2.5;
		if (optionCount[0] > 0) {
			tempScore[3] = optionCount[2] - optionCount[0];
		}
	}
};

class Qd8 : public Question
{
public:
	Qd8()
	{
		id = 8;
		author = "圣墓上的倒吊人";
		title = "让步";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("如果所有人都选择一个选项，那只执行它。否则执行被选的加分最少的选项，且选择该选项玩家+2，其他人-2。");
	}
	virtual void initOptions() override
	{
        options.push_back("+0");
        options.push_back("+1");
		options.push_back("+2");
		options.push_back("+3");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabbccd");
	}
	virtual void calc(vector<Player>& players) override
	{
		int selected = 0;
		bool add_point = true;
		for (int i = 0; i < 4; i++) {
			if (optionCount[i] > 0) {
				if (add_point) {
					tempScore[i] = i + 2;
					add_point = false;
				} else {
					tempScore[i] = -2;
				}
				selected++;
			}
		}
		if (selected == 1) {
			for (int i = 0; i < 4; i++) {
				if (optionCount[i] > 0) {
					tempScore[i] -= 2;
				}
			}
		}
	}
};

class Qd9 : public Question
{
public:
	Qd9()
	{
		id = 9;
		author = "Q群管家";
		title = "奇偶1";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("对应选项选择人数奇数取反，偶数取正。");
	}
	virtual void initOptions() override
	{
        options.push_back("+3");
        options.push_back("0");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaaab");
	}
	virtual void calc(vector<Player>& players) override
	{
		if (int(optionCount[0]) % 2 == 1) {
			tempScore[0] = -3;
		} else {
			tempScore[0] = 3;
		}
	}
};

class Qd10 : public Question
{
   public:
    Qd10()
    {
        id = 10;
        author = "Q群管家";
        title = "奇偶2";
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back("选择一项。");
    }
    virtual void initOptions() override
    {
        options.push_back("+3");
        options.push_back("-1，若此选项选择人数为奇数，A变为-3");
    }
    virtual void initExpects() override
    {
        expects.push_back("aaab");
    }
    virtual void calc(vector<Player>& players) override
    {
        tempScore[0] = int(optionCount[1]) % 2 == 1 ? -3 : 3;
        tempScore[1] = -1;
    }
};

class Qd11 : public Question
{
public:
	Qd11()
	{
		id = 11;
		author = "Q群管家";
		title = "奇偶3";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选项得分与选择人数之和若为奇数则得分取反。");
	}
	virtual void initOptions() override
	{
        options.push_back("+1");
        options.push_back("+2");
		options.push_back("+3");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabbc");
	}
	virtual void calc(vector<Player>& players) override
	{
		for (int i = 0; i < 3; i++) {
			if (int(optionCount[i] + i + 1) % 2 == 1) {
				tempScore[i] = -(i + 1);
			} else {
				tempScore[i] = i + 1;
			}
		}
	}
};

class Qd12 : public Question
{
public:
	Qd12()
	{
		id = 12;
		author = "Q群管家";
		title = "奇偶4";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back(RC(
			"一张正面是+2，背面是-2的卡牌，初始正面向上，最终状态是本题得分，你选择：",
			"一张正面是+2，背面是-2的纸，初始正面向上，最终状态是本题得分，你选择："));
	}
	virtual void initOptions() override
	{
        options.push_back("翻面，你加对应的分数");
        options.push_back("不翻面，你减对应的分数");
	}
	virtual void initExpects() override
	{
		expects.push_back("aab");
	}
	virtual void calc(vector<Player>& players) override
	{
		if (int(optionCount[0]) % 2 == 0) {
			tempScore[0] = 2;
			tempScore[1] = -2;
		} else {
			tempScore[0] = -2;
			tempScore[1] = 2;
		}
	}
};

class Qd13 : public Question   // [待修改]分数增减幅度太大    平衡 1
{
public:
	Qd13()
	{
		id = 13;
		author = "圣墓上的倒吊人";
		title = "资源配置";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		vars["start"] = playerNum * 2;
		vars["limit"] = int(playerNum * 3.2);
		texts.push_back("桌上有 " + str(vars["start"]) + " 个资源，一资源一分，如果投入后总资源超过了 " + str(vars["limit"]) + " ，那获得（你的减分/所有玩家的减分）比例的总资源。如果投入后分数为负数，则选择无效。");
	}
	virtual void initOptions() override
	{
        options.push_back("0");
        options.push_back("-1");
		options.push_back("-2");
		options.push_back("-4");
		options.push_back("-6");
		options.push_back("1.6%获得投入后总资源，否则-15");
	}
	virtual void initExpects() override
	{
		expects.push_back("abbcccddddeef");
	}
	virtual void calc(vector<Player>& players) override
	{
		int total = vars["start"];
		total = total + optionCount[1] * 1 + optionCount[2] * 2 + optionCount[3] * 4 + optionCount[4] * 6;
		tempScore[1] = -1;
		tempScore[2] = -2;
		tempScore[3] = -4;
		tempScore[4] = -6;
		for (int i = 0; i < playerNum; i++) {
			if (players[i].score < -tempScore[players[i].select]) {
				total += tempScore[players[i].select];
				players[i].score -= tempScore[players[i].select];
			}
		}
		if (rand() % 1000 < 16) {
			tempScore[5] = total;
		} else {
			if (total > vars["limit"]) {
				for (int i = 0; i < playerNum; i++) {
					if (players[i].score >= -tempScore[players[i].select]) {
						players[i].score += -tempScore[players[i].select] / (total - vars["start"]) * total;
					}
				}
			}
			tempScore[5] = -15;
		}
	}
};

class Qd14 : public Question   // [待修改]选项不平衡，选D居多
{
public:
	Qd14()
	{
		id = 14;
		author = "飘渺";
		title = "电车难题";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("请选择你的位置。拉杆只有2个状态，即拉动偶数次会还原拉杆");
	}
	virtual void initOptions() override
	{
        options.push_back("乘客：+0.5，但每有一个人拉动拉杆-0.5");
        options.push_back("多数派：+0.5，如果拉杆最终没有拉动，改为-2");
		options.push_back("少数派：+0.5，如果拉杆最终拉动，改为-2.5");
		options.push_back("-0.5，每一个选择此项的人都会拉动一次拉杆");
	}
	virtual void initExpects() override
	{
		expects.push_back("abbcdddd");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = 0.5 - optionCount[3] * 0.5;
		tempScore[1] = 0.5;
		if (int(optionCount[3]) % 2 == 0) {
			tempScore[1] = -2;
		}
		tempScore[2] = 0.5;
		if (int(optionCount[3]) % 2 == 1) {
			tempScore[2] = -2.5;
		}
		tempScore[3] = -0.5;
	}
};

class Qd15 : public Question   // [待修改]分数增减幅度太大
{
public:
	Qd15()
	{
		id = 15;
		author = "Q群管家";
		title = "均分1";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
        options.push_back("当前总积分×0.5");
        options.push_back("选择此选项的玩家均分他们的分数");
		options.push_back("当前总积分×（-0.5）");
	}
	virtual void initExpects() override
	{
		expects.push_back("abbbbbc");
	}
	virtual void calc(vector<Player>& players) override
	{
		double sum = 0;
		for (int i = 0; i < playerNum; i++) {
        	if (players[i].select == 1) {
				sum += players[i].score;
			}
		}
		for (int i = 0; i < playerNum; i++) {
        	if (players[i].select == 0) {
				players[i].score = players[i].score * 0.5;
			}
			if (players[i].select == 1) {
				players[i].score = sum / optionCount[1];
			}
			if (players[i].select == 2) {
				players[i].score = -players[i].score * 0.5;
			}
		}
	}
};

class Qd16 : public Question   // [待修改]选择性问题，大部分情况玩家可能单选B
{
public:
	Qd16()
	{
		id = 16;
		author = "Q群管家";
		title = "均分2";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
        options.push_back("选择此选项的分数最高与最低的玩家均分他们的分数");
        options.push_back("正分的-2，负分的+2");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaabb");
	}
	virtual void calc(vector<Player>& players) override
	{
		double maxS = -99999;
		double minS = 99999;
		for(int i = 0; i < playerNum; i++)
		{
			if (players[i].select == 0) {
				maxS = max(maxS, players[i].score);
				minS = min(minS, players[i].score);
			}
		}
		double sum = 0;
		int count = 0;
		for(int i = 0; i < playerNum; i++)
		{
			if (players[i].select == 0 && (players[i].score == minS || players[i].score == maxS)) {
				sum += players[i].score;
				count++;
			}
		}
		for (int i = 0; i < playerNum; i++) {
        	if (players[i].select == 0 && (players[i].score == minS || players[i].score == maxS)) {
				players[i].score = sum / count;
			}
			if (players[i].select == 1) {
				if (players[i].score > 0) {
					players[i].score -= 2;
				}
				if (players[i].score < 0) {
					players[i].score += 2;
				}
			}
		}
	}
};

class Qd17 : public Question   // [测试]奇偶存在较大的运气因素，多人时分数增减幅度太大
{
public:
	Qd17()
	{
		id = 17;
		author = "Q群管家";
		title = "奇偶5";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
        options.push_back("若选择人数为奇数，扣除 [选择本选项人数/2] 的分数；否则获得相应的分数");
        options.push_back("0");
	}
	virtual void initExpects() override
	{
		expects.push_back("aab");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = int(optionCount[0]) % 2 == 1 ? -optionCount[0] / 2 : optionCount[0] / 2;
	}
};

class Qd18 : public Question   // [待修改]分数增减幅度太大
{
public:
	Qd18()
	{
		id = 18;
		author = "胡扯弟";
		title = "云顶之巢";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择你的云巢身份。");
	}
	virtual void initOptions() override
	{
        options.push_back("YAMI：总是能+2分的大佬，但是有5%的可能性失手-1");
        options.push_back(RC("飘渺：10%+24的赌博爱好者", "飘渺：10%+24的概率爱好者"));
		options.push_back("黑桃3：+13，但每有一个飘渺-1.75分");
		options.push_back("飞机：+12，但有YAMI就会被gank");
		options.push_back("西东：+A+B-C+D");
		options.push_back("胡扯：-114514，但有2.5%的可能性改为+1919810");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaaabbbccdeeeef");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = rand() % 100 < 5 ? -1 : 2;
		tempScore[1] = rand() % 100 < 10 ? 24 : 0;
		tempScore[2] = 13 - optionCount[1] * 1.75;
		tempScore[3] = optionCount[0] == 0 ? 12 : 0;
		tempScore[4] = optionCount[0] + optionCount[1] - optionCount[2] + optionCount[3];
		tempScore[5] = rand() % 1000 < 25 ? 1919810 : -114514;
	}
};

class Qd19 : public Question   // [待修改]触发E选项取反概率太高
{
public:
	Qd19()
	{
		id = 19;
		author = "圣墓上的倒吊人";
		title = "未来计划";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("（X）代表有且只有 X 人选择该选项时，该选项才能执行。执行顺序为序号顺序。无选项执行时执行E");
	}
	virtual void initOptions() override
	{
        options.push_back("+4（1）");
        options.push_back("+2（2）");
		options.push_back("-3（3）");
		options.push_back("-1（≥2）");
		options.push_back("所有玩家的分数变为相反数（4）");
	}
	virtual void initExpects() override
	{
		expects.push_back("abbbcdee");
	}
	virtual void calc(vector<Player>& players) override
	{
		for(int i = 0; i < playerNum; i++) {
			if (players[i].select == 0 && optionCount[0] == 1) {
				players[i].score += 4;
			}
			if (players[i].select == 1 && optionCount[1] == 2) {
				players[i].score += 2;
			}
			if (players[i].select == 2 && optionCount[2] == 3) {
				players[i].score -= 3;
			}
			if (players[i].select == 3 && optionCount[3] >= 2) {
				players[i].score -= 1;
			}
		}
		if ((optionCount[0] != 1 && optionCount[1] != 2 && optionCount[2] != 3 && optionCount[3] < 2) || optionCount[4] == 4) {
			for(int i = 0; i < playerNum; i++) {
				players[i].score = -players[i].score;
			}
		}
	}
};

class Qd20 : public Question   // [待修改]人多时分数增减幅度太大
{
public:
	Qd20()
	{
		id = 20;
		author = "圣墓上的倒吊人";
		title = RC("真理", "少与多");
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back(RC("真理掌握在？", "机会属于？"));
	}
	virtual void initOptions() override
	{
        options.push_back("少数人：+3，如果此项选择人数最多，则改为减 [选择该项人数] 的分数");
        options.push_back("多数人：-2");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbb");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = optionCount[0] == maxSelect ? -optionCount[0] : 3;
		tempScore[1] = -2;
	}
};

class Qd21 : public Question   // [待修改]分数增减幅度太大
{
public:
	Qd21()
	{
		id = 21;
		author = "丘陵萨满";
		title = "伊甸园";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		vector<double> tmp_score;
		for (int i = 0; i < playerNum; i++) {
			tmp_score.push_back(players[i].score);
		}
		sort(tmp_score.begin(),tmp_score.end());
		vars["score"] = tmp_score[2];
		texts.push_back("获胜的选项获得 [选择非此选项人数] 的分数，失败的选项失去 [选择获胜选项人数] 的分数");
	}
	virtual void initOptions() override
	{
        options.push_back("红苹果：如果分数小于等于 " + str(vars["score"]) + " 的玩家都选择红苹果，红苹果获胜。但如果所有人都选择此项，全员分数取反");
        options.push_back("银苹果：如果选择此项的人数少于 C，且红苹果没有获胜，银苹果获胜。");
		options.push_back("金苹果：如果选择此项的人数少于等于 B，且红苹果没有获胜，金苹果获胜。");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaaabc");
	}
	virtual void calc(vector<Player>& players) override
	{
		int win, l1, l2;
		bool red_win = true;
		bool is_invert = false;
		win = l1 = l2 = 0;
		for (int i = 0; i < playerNum; i++) {
			if (players[i].score <= vars["score"] && players[i].select != 0) {
				red_win = false; break;
			}
		}
		if (red_win) {
			if (optionCount[0] == playerNum) {
				is_invert = true;
				for (int i = 0; i < playerNum; i++) {
					players[i].score = -players[i].score;
				}
			} else {
				win = 0;
				l1 = 1; l2 = 2;
			}
		} else {
			if (optionCount[1] < optionCount[2]) {
				win = 1;
				l1 = 0; l2 = 2;
			} else {
				win = 2;
				l1 = 0; l2 = 1;
			}
		}
		if (!is_invert) {
			tempScore[win] = optionCount[l1] + optionCount[l2];
			tempScore[l1] = -optionCount[win];
			tempScore[l2] = -optionCount[win];
		}
	}
};

class Qd22 : public Question   // [待修改]分数增减幅度太大，A选项风险太高
{
public:
	Qd22()
	{
		id = 22;
		author = "圣墓上的倒吊人";
		title = RC("战争中的贵族", "对峙中的角色");
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back(RC("你的国家与敌国正在战争中，选择一项行动。", "你的阵营与敌方阵营正在对峙中，选择一项行动。"));
	}
	virtual void initOptions() override
	{
        options.push_back(RC("继续抵抗：+5，如果有玩家选择了B，那选择此项的人分数归零", "继续坚持：+5，如果有玩家选择了B，那选择此项的人分数归零"));
        options.push_back(RC("有条件投降：-3", "有条件让步：-3"));
		options.push_back(RC("润出国外：如果有人选 A，则-2", "远赴他乡：如果有人选 A，则-2"));
	}
	virtual void initExpects() override
	{
		expects.push_back("aabccccc");
	}
	virtual void calc(vector<Player>& players) override
	{
		if (optionCount[1] == 0) {
			tempScore[0] = 5;
		} else {
			for (int i = 0; i < playerNum; i++) {
				if (players[i].select == 0) {
					players[i].score = 0;
				}
			}
		}
		tempScore[1] = -3;
		if (optionCount[0] > 0) {
			tempScore[2] = -2;
		}
	}
};

class Qd23: public Question   // [待修改]多人时分数增减幅度太大
{
public:
	Qd23()
	{
		id = 23;
		author = "飞机鸭卖蛋";
		title = "太多挂机A了…？";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("分别记选择A、B、C选项的人数为a、b、c：");
	}
	virtual void initOptions() override
	{
		options.push_back("获得 a 分");
		options.push_back("获得 a×1.5-b 分");
		options.push_back("获得 a×2-c 分");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaaabbccc");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = optionCount[0];
		tempScore[1] = optionCount[0] * 1.5 - optionCount[1];
		tempScore[2] = optionCount[0] * 2 - optionCount[2];
	}
};

class Qd24 : public Question   // [待修改]人多时分数增减幅度太大
{
public:
	Qd24()
	{
		id = 24;
		author = "圣墓上的倒吊人";
		title = "税收";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
        options.push_back("减45%分数");
        options.push_back("减 [B选项人数] 的分数");
		options.push_back("减 [C选项人数*1.5] 的分数");
	}
	virtual void initExpects() override
	{
		expects.push_back("abbbcc");
	}
	virtual void calc(vector<Player>& players) override
	{
		for (int i = 0; i < playerNum; i++) {
			if (players[i].select == 0) {
				if (players[i].score > 0) {
					players[i].score -= players[i].score * 0.45;
				} else {
					players[i].score += players[i].score * 0.45;
				}
			}
		}
		tempScore[1] = -optionCount[1];
		tempScore[2] = -optionCount[2] * 1.5;
	}
};

class Qd25 : public Question   // [测试]随机性太高
{
public:
	Qd25()
	{
		id = 25;
		author = "Chance";
		title = "我是大土块";
	}
	
	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
		if (playerNum > 8) {
			vars["percent"] = 10;
		} else {
			vars["percent"] = 16;
		}
        options.push_back("+2，有 [(B+C) ×" + str(vars["percent"]) + "]% 的概率-4");
        options.push_back("+1，有 [(A-B) ×" + str(vars["percent"]) + "]% 的概率使C+2");
		options.push_back("-2，有 [(A+D) ×" + str(vars["percent"]) + "]% 的概率+6");
		options.push_back("-3，有 [(A+B-D) ×" + str(vars["percent"]) + "]% 的概率使A和B-4，D+5");
	}
	virtual void initExpects() override
	{
		expects.push_back("abcd");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = 2;
		tempScore[1] = 1;
		tempScore[2] = -2;
		tempScore[3] = -3;
		if (rand() % 100 < (optionCount[1] + optionCount[2]) * vars["percent"]) {
			tempScore[0] -= 4;
		}
		if (rand() % 100 < (optionCount[0] - optionCount[1]) * vars["percent"]) {
			tempScore[2] += 2;
		}
		if (rand() % 100 < (optionCount[0] + optionCount[3]) * vars["percent"]) {
			tempScore[2] += 6;
		}
		if (rand() % 100 < (optionCount[0] + optionCount[1] - optionCount[3]) * vars["percent"]) {
			tempScore[0] -= 4;
			tempScore[1] -= 4;
			tempScore[2] += 5;
		}
	}
};

class Qd26 : public Question   // 题目过于复杂
{
public:
	Qd26()
	{
		id = 26;
		author = "纸团OvO";
		title = "差值投标";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择人数最多的两项（如果有并列则选更靠前的）并进行对奕（胜者+2分），且其选中者+0.5分");
		texts.push_back(RC("当筹码高的高于对面 1级/2级/3级 时，分别得到 胜/平/负 的结果", "当等级高的高于对面 1级/2级/3级 时，分别得到 胜/平/负 的结果"));
	}
	virtual void initOptions() override
	{
        options.push_back(RC("奴隶流：0级筹码", "新人流：0级"));
        options.push_back(RC("草民流：1级筹码", "学徒流：1级"));
		options.push_back(RC("市民流：2级筹码", "熟练流：2级"));
		options.push_back(RC("皇帝流：3级筹码", "王者流：3级"));
		options.push_back(RC("悠悠流：0级或1级筹码（当对面是0时为1，否则为0）", "悠悠流：0级或1级（当对面是0时为1，否则为0）"));
	}
	virtual void initExpects() override
	{
		expects.push_back("abcde");
	}
	virtual void calc(vector<Player>& players) override
	{
		vector<double> optionSort = optionCount;
		vector<int> orig_indexes(options.size());
		iota(orig_indexes.begin(), orig_indexes.end(), 0);
		partial_sort(orig_indexes.begin(), orig_indexes.begin() + 2, orig_indexes.end(), [&optionSort](int i, int j) {
			return optionSort[i] > optionSort[j];
		});
		int o1 = orig_indexes[0];
		int o2 = orig_indexes[1];
		if (o1 > o2) swap(o1, o2);

		tempScore[o1] = 0.5;
		tempScore[o2] = 0.5;
		int level1 = o1;
		int level2 = o2;
		if (level2 == 4) {
			if (level1 == 0 || level1 == 3) {
				tempScore[o2] += 2;
			} else if(level1 == 1){
				tempScore[o1] += 2;
			}
		} else {
			if (abs(level2 - level1) == 1) {
				tempScore[o2] += 2;
			}
			if (abs(level2 - level1) == 3) {
				tempScore[o1] += 2;
			}
		}
	}
};

class Qd27 : public Question   // 题目过于复杂   平衡 1
{
public:
	Qd27()
	{
		id = 27;
		author = "xiaogt";
		title = RC("债务危机", "负载危机");
	}

	virtual void initTexts(vector<Player>& players) override
	{
		vars["debt"] = playerNum * 1.5;
		texts.push_back(RC(
			"有 " + str(vars["debt"]) + " 负债需要分摊，玩家每+1分，负债+1，玩家每-1分，负债-1。每轮将以从上往下的顺序结算。若所有选项均结算完成，将开始新一轮结算，直到负债归零。你选择：",
			"有 " + str(vars["debt"]) + " 系统负载需要分摊，玩家每+1分，负载+1，玩家每-1分，负载-1。每轮将以从上往下的顺序结算。若所有选项均结算完成，将开始新一轮结算，直到负载归零。你选择："));
	}
	virtual void initOptions() override
	{
		options.push_back("-1分");
		options.push_back("+1分，第二轮起每轮-3分");
		options.push_back("-2分");
		options.push_back("0分，第二轮起每轮-3分");
		options.push_back("-1.5分，第二轮起每轮+0分");
		options.push_back("-6分，若在第一轮就分摊完毕，改为+1分");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbbcddeff");
	}
	virtual void calc(vector<Player>& players) override
	{
		int debt = vars["debt"];
		tempScore[0] -= 1;
		tempScore[1] += 1;
		tempScore[2] -= 2;
		tempScore[4] -= 1.5;
		tempScore[5] -= 6;
		debt = debt - optionCount[0] + optionCount[1] - optionCount[2]*2 - optionCount[4]*3 - optionCount[5]*6;
		if (debt <= 0) {
			tempScore[5] = 1;
		}
		int minus[6] = {1, 3, 2, 3, 0, 6};
		while (debt > 0) {
			for (int i = 0; i < 6; i++) {
				tempScore[i] -= minus[i];
				debt -= optionCount[i] * minus[i];
				if (debt <= 0) break;
			}
		}
	}
};

class Qd28 : public Question   // D选项不好理解，选项不平衡，AD人数居多
{
public:
	Qd28()
	{
		id = 28;
		author = "大梦我先觉";
		title = "夺宝奇兵";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
        options.push_back("如果此选项的人数<=3，平分 6 分的宝藏");
        options.push_back("如果此选项的人数>=4，平分 10 分的宝藏");
		options.push_back("地雷：如果仅 1 人选择，使A和B的得分变为相反数，你获得 [A+B] 人数的分数");
		options.push_back("战术弃权：每有一人选择此选项，则使选择A的人数减少一人（最低不低于1），使选择B的人数增加一人");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbbcd");
	}
	virtual void calc(vector<Player>& players) override
	{
		int gap;
		optionCount[0] -= optionCount[3];
		if (optionCount[0] <= 0) {
			gap = 1 - optionCount[0];
			optionCount[0] = 1;
		}
		optionCount[1] += optionCount[3];
		if (optionCount[0] <= 3) {
			tempScore[0] = 6 / optionCount[0];
		}
		if (optionCount[1] >= 4) {
			tempScore[1] = 10 / optionCount[1];
		}
		if (optionCount[2] == 1) {
			tempScore[0] = -tempScore[0];
			tempScore[1] = -tempScore[1];
			tempScore[2] = optionCount[0] + optionCount[1] - gap;
		}
	}
};

class Qd29: public Question   // 分数增减幅度太大
{
public:
	Qd29()
	{
		id = 29;
		author = "齐齐";
		title = "HP杀";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
		options.push_back("杀手：当A>B时，+2，否则-2");
		options.push_back("平民：当B>A时，+2，否则-2");
		options.push_back("内奸：当A=B时，+2，否则-2");
		options.push_back("特工：当A=B时，+10，否则-10");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaaaabbbbbcccd");
	}
	virtual void calc(vector<Player>& players) override
	{
		if (optionCount[0] > optionCount[1]) {
			tempScore[0] = 2;
			tempScore[1] = -2;
			tempScore[2] = -2;
			tempScore[3] = -10;
		} else if (optionCount[0] < optionCount[1]) {
			tempScore[0] = -2;
			tempScore[1] = 2;
			tempScore[2] = -2;
			tempScore[3] = -10;
		} else {
			tempScore[0] = -2;
			tempScore[1] = -2;
			tempScore[2] = 2;
			tempScore[3] = 10;
		}
	}
};

class Qd30: public Question   // 清0需慎重
{
public:
	Qd30()
	{
		id = 30;
		author = "剩菜剩饭";
		title = "寻宝猎人";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项。");
	}
	virtual void initOptions() override
	{
		vars["num"] = playerNum * 2 - 1;
		options.push_back("宝藏：平分 " + str(vars["num"]) + " 分");
		options.push_back("珍宝：如果选择人数小于等于3，+2");
		options.push_back("奇遇：选择A，B的玩家减 [选择C的人数] 的分数");
		options.push_back("怪物：+1分，如果没有人选本项，选C的玩家分数归0");
	}
	virtual void initExpects() override
	{
		expects.push_back("aaaabbccd");
	}
	virtual void calc(vector<Player>& players) override
	{
		tempScore[0] = vars["num"] / optionCount[0];
		if (optionCount[1] <= 3) {
			tempScore[1] = 2;
		}
		tempScore[0] -= optionCount[2];
		tempScore[1] -= optionCount[2];
		tempScore[3] = 1;
		if (optionCount[3] == 0) {
			for(int i = 0; i < playerNum; i++) {
				if (players[i].select == 2) {
					players[i].score = 0;
				}
			}
		}
	}
};

class Qd31 : public Question   // 选项严重不平衡，选A居多
{
public:
	Qd31()
	{
		id = 31;
		author = RC("匿名", "圣墓上的倒吊人");
		title = RC("选举", "分数均衡");
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back(RC(
			"选择人数最多的选项生效，且该选项的玩家额外+1.5。若平票，优先结算序号在后面的选项。",
			"选择人数最多的选项生效，且该选项的玩家额外+1.5。若人数相等，优先结算序号在后面的选项。"));
	}
	virtual void initOptions() override
	{
        options.push_back("分最高的人-3，分最低的人+2，其他人+1");
        options.push_back("分最高的人+3，负分的玩家分数加至0");
		options.push_back("选择该项的玩家增加等同于人数的分数，40%概率改为减少");
		options.push_back("0");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbbccd");
	}
	virtual void calc(vector<Player>& players) override
	{
		if (optionCount[3] == maxSelect) {
			tempScore[3] += 1.5;
		} else if (optionCount[2] == maxSelect) {
			tempScore[2] = optionCount[2];
			if (rand() % 10 < 4) {
				tempScore[2] = -tempScore[2];
			}
			tempScore[2] += 1.5;
		} else if (optionCount[1] == maxSelect) {
			for (int i = 0; i < playerNum; i++) {
				if (players[i].score == maxScore) {
					players[i].score += 3;
				}
				if (players[i].score < 0) {
					players[i].score = 0;
				}
			}
			tempScore[1] += 1.5;
		} else if (optionCount[0] == maxSelect) {
			for (int i = 0; i < playerNum; i++) {
				if (players[i].score == maxScore) {
					players[i].score -= 3;
				} else if (players[i].score == minScore) {
					players[i].score += 2;
				} else {
					players[i].score += 1;
				}
			}
			tempScore[0] += 1.5;
		}
	}
};

class Qd32 : public Question   // 题目过于复杂
{
   public:
    Qd32()
    {
        id = 32;
        author = "圣墓上的倒吊人";
        title = "坐等反转";
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back("甲 / 乙，被选中的选项执行乙，否则执行甲。选项从前往后结算。");
    }
    virtual void initOptions() override
    {
        options.push_back("选择人数最少的选项+2 / 选择人数最多的选项-2");
        options.push_back("使 D 选项无效化 / 0");
        options.push_back("D-2 / D+2");
        options.push_back("所有玩家均分自己的分数 / -1");
    }
    virtual void initExpects() override
    {
        expects.push_back("aabbcd");
    }
    virtual void calc(vector<Player>& players) override
    {
        for(int i = 0; i < playerNum; i++) {
            if (optionCount[0] == 0 && optionCount[players[i].select] == minSelect) {
                players[i].score += 2;
            }
            if (optionCount[0] > 0 && optionCount[players[i].select] == maxSelect) {
                players[i].score -= 2;
            }
            if (optionCount[2] == 0 && players[i].select == 3) {
                players[i].score -= 2;
            }
            if (optionCount[2] > 0 && players[i].select == 3) {
                players[i].score += 2;
            }
        }
        if (optionCount[1] > 0) {
            if (optionCount[3] == 0) {
                double sum = 0;
                for(int i = 0; i < playerNum; i++) {
                    sum += players[i].score;
                }
                for(int i = 0; i < playerNum; i++) {
                    players[i].score = sum / playerNum;
                }
            } else {
                tempScore[3] -= 1;
            }
        }
    }
};

class Qd33 : public Question   // AB玩家居多    平衡 1
{
public:
	Qd33()
	{
		id = 33;
		author = "Chance";
		title = "未命名";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择下面一项，人数最少的一项成立");
	}
	virtual void initOptions() override
	{
        options.push_back("B+2");
        options.push_back("C-0.5");
		options.push_back("D-1");
		options.push_back("A+1");
	}
	virtual void initExpects() override
	{
		expects.push_back("aabbbccd");
	}
	virtual void calc(vector<Player>& players) override
	{
		if (optionCount[0] == minSelect) {
			tempScore[1] += 2;
		}
		if (optionCount[1] == minSelect) {
			tempScore[2] -= 0.5;
		}
		if (optionCount[2] == minSelect) {
			tempScore[3] -= 1;
		}
		if (optionCount[3] == minSelect) {
			tempScore[0] += 1;
		}
	}
};

class Qd34 : public Question   // 题目过于复杂
{
public:
	Qd34()
	{
		id = 34;
		author = "纤光";
		title = "差值投标";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择人数最多的两项对战（多项相同时取靠近A的两项），对战者中靠近A的称作败者，靠近D的称作胜者。另外两项与败者得分相同但额外-0.5。");
		texts.push_back("败者和胜者选项分别生效，随后胜者+4。");
	}
	virtual void initOptions() override
	{
        options.push_back("-0");
        options.push_back("-2");
		options.push_back("-4");
		options.push_back("-6");
	}
	virtual void initExpects() override
	{
		expects.push_back("abcd");
	}
	virtual void calc(vector<Player>& players) override
	{
		vector<double> optionSort = optionCount;
		vector<int> orig_indexes(options.size());
		iota(orig_indexes.begin(), orig_indexes.end(), 0);
		partial_sort(orig_indexes.begin(), orig_indexes.begin() + 2, orig_indexes.end(), [&optionSort](int i, int j) {
			return optionSort[i] > optionSort[j];
		});

		int o1 = orig_indexes[0];
		int o2 = orig_indexes[1];
		if (o1 > o2) swap(o1, o2);
		tempScore[o1] = -o1 * 2;
		tempScore[o2] = -o2 * 2 + 4;
		for (int i = 0; i < 4; i++) {
			if (i != o1 && i != o2) {
				tempScore[i] = tempScore[o1] - 0.5;
			}
		}
	}
};

class Qd35 : public Question   // 选项分数变动太大
{
public:
	Qd35()
	{
		id = 35;
		author = "克里斯丁";
		title = "半句话失效";
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("有人选择的选项执行对应的效果。");
	}
	virtual void initOptions() override
	{
        options.push_back("-9，你的分数取相反数");
        options.push_back("-1，并令A的后半句话失效");
		options.push_back("-2，并令A的前半句话失效");
		options.push_back("令B、C中选的人更多的选项后半句话失效，选的人更少的选项前半句话失效，B、C一样多则本选项作废");
	}
	virtual void initExpects() override
	{
		expects.push_back("abcd");
	}
	virtual void calc(vector<Player>& players) override
	{
		bool reversal = true;
		bool minus9 = true;
		if (optionCount[1] > 0) {
			tempScore[1] = -1;
			reversal = false;
		}
		if (optionCount[2] > 0) {
			tempScore[2] = -2;
			minus9 = false;
		}
		if (optionCount[3] > 0 && optionCount[1] != optionCount[2]) {
			if (optionCount[1] > optionCount[2]) {
				reversal = !reversal;
				tempScore[2] = 0;
			} else {
				minus9 = !minus9;
				tempScore[1] = 0;
			}
		}
		for(int i = 0; i < playerNum; i++) {
			if (players[i].select == 0) {
				if (minus9) players[i].score -= 9;
				if (reversal) players[i].score = -players[i].score;
			}
		}
	}
};

class Qd36 : public Question   // 题目过于复杂
{
public:
    Qd36()
    {
        id = 36;
        author = "纤光";
        title = "囚徒困境";
    }

    virtual void initTexts(vector<Player>& players) override
    {
        texts.push_back("AB中人数更少项的人数记为X");
		texts.push_back("CD中人数更多项的人数记为Y");
    }

    virtual void initOptions() override
    {
        options.push_back("合作：+(X-Y)");
        options.push_back("合作：+2(X-Y)");
        options.push_back("欺骗：-2(Y-X)");
        options.push_back("欺骗：-Y");
    }

    virtual void initExpects() override
    {
        expects.push_back("abcd");
    }

    virtual void calc(vector<Player>& players) override
    {
		int X = optionCount[0] < optionCount[1] ? optionCount[0] : optionCount[1];
		int Y = optionCount[2] > optionCount[3] ? optionCount[2] : optionCount[3];
		tempScore[0] = X - Y;
		tempScore[1] = 2 * (X - Y);
		tempScore[2] = -2 * (Y - X);
		tempScore[3] = -Y;
    }
};

class Qd37 : public Question   // 积分不够分配时，A选项玩家得分太高
{
   public:
    Qd37()
    {
        id = 37;
        author = "圣墓上的倒吊人";
        title = "报销";
    }

    virtual void initTexts(vector<Player>& players) override
    {
        vars["point"] = playerNum * 1.5;
        texts.push_back("场上有 " + str(vars["point"]) + " 的积分，玩家从中获得和选项数值一致的积分，如果积分不够分配至所有玩家，选 A 的玩家平分所有积分");
    }
    virtual void initOptions() override
    {
        options.push_back("0");
        options.push_back("1");
        options.push_back("2");
        options.push_back("3");
        options.push_back("4");
    }
    virtual void initExpects() override
    {
        expects.push_back("aabccddde");
    }
    virtual void calc(vector<Player>& players) override
    {
        double sum = optionCount[1]*1 + optionCount[2]*2 + optionCount[3]*3 + optionCount[4]*4;
        if (sum <= vars["point"]) {
            tempScore[1] = 1;
            tempScore[2] = 2;
            tempScore[3] = 3;
            tempScore[4] = 4;
        } else {
            tempScore[0] = vars["point"] / optionCount[0];
        }
    }
};

// 开发者: An idle brain
class Qd38 : public Question   // 题目过于复杂
{
public:
	Qd38()
	{
		id = 38;
		author = "剩菜剩饭";
		title = RC("好心喂了狗", "好心错付");
	}

	virtual void initTexts(vector<Player>& players) override
	{
		texts.push_back("选择一项");
	}

	virtual void initOptions() override
	{
		options.push_back("-1分，如果没有人选择此项，BCD+都3分");
		options.push_back("如果没有人选择此项，选D的玩家-2分");
		options.push_back("如果没有人选择此项，选B，D的玩家+2分");
		options.push_back("如果没有人选择此项，选B，C的玩家-2分");
		options.push_back("如果没有人选择此项，第一名+4分");
	}

	virtual void initExpects() override
	{
		expects.push_back("abbbcccdddd");
	}

	virtual void calc(vector<Player>& players) override
	{
		if (optionCount[0] > 0) {
			tempScore[0] -= 1;
		} else {
			tempScore[1] += 3;
			tempScore[2] += 3;
			tempScore[3] += 3;
		}
		if (optionCount[1] == 0) {
			tempScore[3] -= 2;
		}
		if (optionCount[2] == 0) {
			tempScore[1] += 2;
			tempScore[3] += 2;
		}
		if (optionCount[3] == 0) {
			tempScore[1] -= 2;
			tempScore[2] -= 2;
		}
		if (optionCount[4] == 0) {
			for(int i = 0; i < playerNum; i++)
			{
				if(players[i].score == maxScore)
				{
					players[i].score += 4;
				}
			}
		}
	}
};
