#include "solve_preset_window.h"

#include <gtk/gtk.h>

#include <cmath>
#include <string>
#include <tuple>
#include <utility>

#include "log.h"
#include "solve_window.h"

namespace ui {

using string_pair = std::pair<const char*, const char*>;

constexpr int kTimeChallengePreset = 999;

const std::vector<std::pair<
    string_pair, std::vector<std::tuple<string_pair, int, Rank, Rank>>>>
    kTopicPresets = {
        {
            {"启蒙", "Beginner"},
            {
                {{"一步吃子", "Capture in one move"},
                 100,
                 Rank::k15K,
                 Rank::k11K},  //
                {{"一步逃子", "Escape in one move"},
                 101,
                 Rank::k15K,
                 Rank::k12K},
                {{"一步连接", "Connect in one move"},
                 102,
                 Rank::k15K,
                 Rank::k10K},  //
                {{"一步分断", "Split in one move"},
                 103,
                 Rank::k15K,
                 Rank::k12K},                                                //
                {{"门吃", "Capture by atari"}, 75, Rank::k15K, Rank::k10K},  //
                {{"边线吃子", "Capture on the side"},
                 110,
                 Rank::k15K,
                 Rank::k7K},  //
                {{"棋子的气", "Group liberties"}, 163, Rank::k15K, Rank::k15K},
                {{"逃子方向", "Direction of escape"},
                 85,
                 Rank::k14K,
                 Rank::k6K},  //
                {{"要子与废子", "Vital and useless stones"},
                 238,
                 Rank::k13K,
                 Rank::k7K},                                      //
                {{"逃子", "Escape"}, 83, Rank::k14K, Rank::k7K},  //
                {{"围空", "Surround territory"},
                 310,
                 Rank::k12K,
                 Rank::k10K},  //
                {{"吃子方向", "Direction of capture"},
                 74,
                 Rank::k15K,
                 Rank::k6K},  //
            },
        },
        {
            {"吃子技巧", "Capturing techniques"},
            {
                {{"宽征", "Loose ladder"}, 98, Rank::k13K, Rank::k1K},        //
                {{"乌龟不出头", "Crane's nest"}, 29, Rank::k14K, Rank::k3K},  //
                {{"倒扑", "Snapback"}, 107, Rank::k15K, Rank::k8K},           //
                {{"双吃", "Double atari"}, 104, Rank::k15K, Rank::k11K},      //
                {{"夹吃", "Clamp capture"}, 236, Rank::k12K, Rank::k7K},      //
                {{"征吃", "Ladder capture"}, 106, Rank::k14K, Rank::k8K},     //
                {{"扑吃", "Snapback capture"}, 76, Rank::k14K, Rank::k7K},    //
                {{"抱吃", "Capture by atari"}, 105, Rank::k15K, Rank::k12K},  //
                {{"挖吃", "Wedging capture"}, 77, Rank::k14K, Rank::k6K},     //
                {{"枷吃", "Net capture"}, 96, Rank::k13K, Rank::k8K},         //
                {{"靠单", "Prevent the bamboo joint"},
                 137,
                 Rank::k9K,
                 Rank::k4K},                                                  //
                {{"相思断", "Lovesick cut"}, 35, Rank::k8K, Rank::k4K},       //
                {{"滚打包收", "Squeeze"}, 28, Rank::k14K, Rank::k2D},         //
                {{"接不归", "Connect and die"}, 108, Rank::k15K, Rank::k7K},  //
            },
        },
        {
            {"基本手筋", "Basic tesuji"},
            {
                {{"尖", "Diagonal"}, 18, Rank::k13K, Rank::k5D},       //
                {{"立", "Descent"}, 24, Rank::k13K, Rank::k5D},        //
                {{"挤", "Kosumi wedge"}, 52, Rank::k13K, Rank::k5D},   //
                {{"断", "Cut"}, 54, Rank::k15K, Rank::k5D},            //
                {{"点", "Placement"}, 4, Rank::k13K, Rank::k5D},       //
                {{"飞", "Knight's move"}, 55, Rank::k7K, Rank::k5D},   //
                {{"虎", "Tiger's mouth"}, 19, Rank::k13K, Rank::k5D},  //
                {{"跳", "Jump"}, 56, Rank::k11K, Rank::k5D},           //
                {{"夹", "Clamp"}, 25, Rank::k11K, Rank::k5D},          //
                {{"顶", "Bump"}, 57, Rank::k11K, Rank::k5D},           //
                {{"挖", "Wedge"}, 40, Rank::k13K, Rank::k5D},          //
                {{"扑", "Throw-in"}, 30, Rank::k15K, Rank::k5D},       //
                {{"跨", "Cut accross"}, 61, Rank::k8K, Rank::k4D},     //
                {{"托", "Underneath attachment"},
                 49,
                 Rank::k12K,
                 Rank::k5D},                                          //
                {{"枷", "Net"}, 67, Rank::k4K, Rank::k2D},            //
                {{"靠", "Contact play"}, 69, Rank::k10K, Rank::k5D},  //
                {{"大飞", "Large knight's move"},
                 201,
                 Rank::k7K,
                 Rank::k1D},                                        //
                {{"扳", "Hane"}, 10, Rank::k15K, Rank::k5D},        //
                {{"冲", "Push"}, 205, Rank::k15K, Rank::k5D},       //
                {{"滚打", "Squeeze"}, 222, Rank::k11K, Rank::k4D},  //
                {{"爬", "Crawl"}, 229, Rank::k11K, Rank::k5D},      //
                {{"弯", "Bend"}, 88, Rank::k9K, Rank::k5D},         //
                {{"长", "Stretch"}, 87, Rank::k11K, Rank::k1D},     //
                {{"接", "Connect"}, 60, Rank::k14K, Rank::k4D},     //
            },
        },
        {
            {"对杀", "Capturing races"},
            {
                {{"有眼杀无眼", "Eye vs no-eye"},
                 32,
                 Rank::k13K,
                 Rank::k3D},  //
                {{"大眼杀小眼", "Big eye vs small eye"},
                 33,
                 Rank::k7K,
                 Rank::k2K},                                                //
                {{"紧气", "Reduce liberties"}, 12, Rank::k15K, Rank::k2D},  //
                {{"使对方不入", "Prevent opponent from approaching"},
                 227,
                 Rank::k11K,
                 Rank::k2D},                                                  //
                {{"延气", "Increase liberties"}, 13, Rank::k12K, Rank::k4D},  //
                {{"黄莺扑蝶", "The oriole captures the butterfly"},
                 50,
                 Rank::k1K,
                 Rank::k1D},  //
                {{"紧气要点", "Vital point for reducing liberties"},
                 225,
                 Rank::k13K,
                 Rank::k4D},
                {{"两扳长一气", "Two hane grow one liberty"},
                 34,
                 Rank::k5K,
                 Rank::k2K},  //
                {{"大眼的气", "Big eye's liberties"},
                 300,
                 Rank::k9K,
                 Rank::k3K},                                                  //
                {{"做眼", "Make eye"}, 230, Rank::k13K, Rank::k2K},           //
                {{"破眼", "Break eye"}, 231, Rank::k13K, Rank::k6K},          //
                {{"大头鬼", "Tombstone squeeze"}, 21, Rank::k9K, Rank::k2D},  //
                {{"延气要点", "Vital point for increasing liberties"},
                 226,
                 Rank::k9K,
                 Rank::k1K},  //
                {{"间接进攻", "Indirect attack"},
                 199,
                 Rank::k4K,
                 Rank::k4D},  //
                {{"对杀要点", "Vital point for capturing race"},
                 200,
                 Rank::k13K,
                 Rank::k4D},  //
                {{"金鸡独立", "Golden chicken stands on one leg"},
                 46,
                 Rank::k13K,
                 Rank::k5D},  //
                {{"常型对杀", "Standard capturing races"},
                 301,
                 Rank::k11K,
                 Rank::k4D},  //
            },
        },
        {
            {"基本死活", "Basic life and death"},
            {
                {{"直三", "Straight three"}, 122, Rank::k15K, Rank::k9K},   //
                {{"弯三", "Bent three"}, 123, Rank::k15K, Rank::k8K},       //
                {{"丁四", "Pyramid four"}, 70, Rank::k15K, Rank::k9K},      //
                {{"刀把五", "Bulky five"}, 125, Rank::k14K, Rank::k7K},     //
                {{"梅花五", "Crossed five"}, 126, Rank::k15K, Rank::k13K},  //
                {{"葡萄六", "Flower six"}, 127, Rank::k14K, Rank::k8K},     //
                {{"一步做眼", "Make eye in one move"},
                 142,
                 Rank::k15K,
                 Rank::k10K},  //
                {{"一步破眼", "Break eye in one move"},
                 143,
                 Rank::k15K,
                 Rank::k10K},  //
                {{"三眼两做", "Three eyes and two actions"},
                 131,
                 Rank::k12K,
                 Rank::k3K},  //
                {{"先手做眼", "Make eye in sente"},
                 174,
                 Rank::k11K,
                 Rank::k5D},  //
                {{"先手破眼", "Break eye in sente"},
                 175,
                 Rank::k12K,
                 Rank::k5D},                                                //
                {{"做眼", "Make eye"}, 232, Rank::k14K, Rank::k5K},         //
                {{"破眼", "Break eye"}, 233, Rank::k14K, Rank::k4K},        //
                {{"直四", "Straight four"}, 234, Rank::k14K, Rank::k12K},   //
                {{"弯四", "Bent four"}, 235, Rank::k14K, Rank::k7K},        //
                {{"胀牯牛", "Squash"}, 39, Rank::k13K, Rank::k2D},          //
                {{"板六", "Rectangular six"}, 237, Rank::k11K, Rank::k6K},  //
                {{"真眼和假眼", "Real eye and false eye"},
                 309,
                 Rank::k13K,
                 Rank::k11K},
                {{"吃子方向", "Capture stone direction"},
                 74,
                 Rank::k15K,
                 Rank::k6K},  //
            },
        },
        {
            {"死活", "Life and death"},
            {
                {{"有眼杀无眼", "Eye vs no-eye"},
                 32,
                 Rank::k13K,
                 Rank::k3D},  //
                {{"扩大眼位", "Increase eye space"},
                 130,
                 Rank::k13K,
                 Rank::k5D},  //
                {{"缩小眼位", "Reduce eye space"},
                 132,
                 Rank::k11K,
                 Rank::k5D},                                                //
                {{"内部动手", "Inside moves"}, 133, Rank::k8K, Rank::k5D},  //
                {{"聚杀", "Inside kill"}, 11, Rank::k13K, Rank::k5D},       //
                {{"利用气紧", "Use shortage of liberties"},
                 171,
                 Rank::k13K,
                 Rank::k5D},  //
                {{"利用棋形弱点", "Exploit shape weakness"},
                 172,
                 Rank::k11K,
                 Rank::k5D},
                {{"盘角曲四", "Bent four in the corner"},
                 43,
                 Rank::k12K,
                 Rank::k4D},                                        //
                {{"胀牯牛", "Squash"}, 39, Rank::k13K, Rank::k2D},  //
                {{"一一妙手", "Brilliant sequence"},
                 2,
                 Rank::k6K,
                 Rank::k5D},  //
                {{"提子后的杀着", "Kill after capture"},
                 183,
                 Rank::k6K,
                 Rank::k5D},                                               //
                {{"组合手段", "Combination"}, 184, Rank::k1K, Rank::k5D},  //
                {{"导致气紧", "Create shortage of liberties"},
                 185,
                 Rank::k10K,
                 Rank::k5D},  //
                {{"利用外围棋子", "Use outside stones"},
                 186,
                 Rank::k5K,
                 Rank::k5D},  //
                {{"利用角部特殊性", "Use corner special properties"},
                 187,
                 Rank::k10K,
                 Rank::k3D},                                                  //
                {{"倒脱靴", "Under the stones"}, 20, Rank::k13K, Rank::k5D},  //
                {{"利用倒扑", "Use snapback"}, 189, Rank::k12K, Rank::k4D},   //
                {{"寻求借用", "Look for leverage"},
                 191,
                 Rank::k11K,
                 Rank::k5D},  //
                {{"杀棋要点", "Vital point for kill"},
                 192,
                 Rank::k11K,
                 Rank::k5D},  //
                {{"打二还一", "Capture two - recapture one"},
                 72,
                 Rank::k14K,
                 Rank::k1D},  //
                {{"利用接不归", "Use connect-and-die"},
                 193,
                 Rank::k11K,
                 Rank::k5D},                                         //
                {{"做眼", "Make eye"}, 14, Rank::k12K, Rank::k5D},   //
                {{"破眼", "Break eye"}, 58, Rank::k12K, Rank::k5D},  //
                {{"先手破眼", "Break eye in sente"},
                 175,
                 Rank::k12K,
                 Rank::k5D},  //
                {{"先手做眼", "Make eye in sente"},
                 174,
                 Rank::k11K,
                 Rank::k5D},  //
                {{"吃子做活", "Capture to live"},
                 196,
                 Rank::k11K,
                 Rank::k4D},  //
                {{"避免被聚杀", "Avoid making dead shape"},
                 197,
                 Rank::k11K,
                 Rank::k2D},                                                  //
                {{"做劫", "Make ko"}, 181, Rank::k14K, Rank::k5D},            //
                {{"保留先手", "Keep sente"}, 202, Rank::k5K, Rank::k5D},      //
                {{"双倒扑", "Double snapback"}, 203, Rank::k14K, Rank::k1K},  //
                {{"点杀", "Kill by eye-point placement"},
                 204,
                 Rank::k12K,
                 Rank::k5D},                                           //
                {{"渡", "Bridge under"}, 206, Rank::k14K, Rank::k5D},  //
                {{"分断", "Cut"}, 97, Rank::k14K, Rank::k4D},          //
                {{"防范弱点", "Defend weak point"},
                 209,
                 Rank::k8K,
                 Rank::k4D},                                              //
                {{"避开陷阱", "Avoid trap"}, 220, Rank::k8K, Rank::k5D},  //
                {{"试探应手", "Probe"}, 212, Rank::k2K, Rank::k4D},       //
                {{"利用一路硬腿", "Use descent to first line"},
                 213,
                 Rank::k10K,
                 Rank::k4D},                                               //
                {{"双提", "Double capture"}, 216, Rank::k2K, Rank::k1D},   //
                {{"阻渡", "Prevent escape"}, 217, Rank::k10K, Rank::k3D},  //
                {{"出动残子", "Run weak group"},
                 218,
                 Rank::k10K,
                 Rank::k5D},  //
                {{"利用对方死活", "Use opponent's life and death"},
                 219,
                 Rank::k3K,
                 Rank::k5D},                                       //
                {{"连络", "Connect"}, 23, Rank::k15K, Rank::k4D},  //
                {{"防守入侵", "Defend from invasion"},
                 198,
                 Rank::k12K,
                 Rank::k5D},  //
                {{"间接进攻", "Indirect attack"},
                 199,
                 Rank::k4K,
                 Rank::k4D},  //
                {{"活棋要点", "Vital point for life"},
                 190,
                 Rank::k13K,
                 Rank::k5D},  //
                {{"老鼠偷油", "Mouse stealing oil"},
                 51,
                 Rank::k5K,
                 Rank::k1D},  //
                {{"金鸡独立", "Golden chicken stands on one leg"},
                 46,
                 Rank::k13K,
                 Rank::k5D},  //
            },
        },
        {
            {"常型死活", "Common life and death"},
            {
                {{"大猪嘴及类似型", "Big pig snout and similar"},
                 42,
                 Rank::k4K,
                 Rank::k3D},  //
                {{"小猪嘴及类似型", "Small pig snout and similar"},
                 27,
                 Rank::k4K,
                 Rank::k2K},                                          //
                {{"二线型", "2nd line"}, 139, Rank::k6K, Rank::k2K},  //
                {{"金柜角及类似型", "Carpenter's square and similar"},
                 48,
                 Rank::k1K,
                 Rank::k4D},  //
                {{"边部常型", "Side common shapes"},
                 180,
                 Rank::k5K,
                 Rank::k3D},  //
                {{"角部常型", "Corner common shapes"},
                 179,
                 Rank::k8K,
                 Rank::k5D},  //
            },
        },
        {
            {"基础官子", "Basic endgame"},
            {
                {{"目与单官", "Fill neutral points"},
                 271,
                 Rank::k12K,
                 Rank::k10K},  //
                {{"双先官子", "Double sente endgame"},
                 149,
                 Rank::k8K,
                 Rank::k5K},                                                  //
                {{"破目", "Break points"}, 150, Rank::k14K, Rank::k8K},       //
                {{"守目", "Defend points"}, 151, Rank::k12K, Rank::k6K},      //
                {{"比较大小", "Compare value"}, 268, Rank::k10K, Rank::k1K},  //
                {{"基本收官", "Endgame fundamentals"},
                 269,
                 Rank::k15K,
                 Rank::k3K},
                {{"先手与后手", "Sente and gote"},
                 270,
                 Rank::k8K,
                 Rank::k3K},  //
            },
        },
        {
            {"官子", "Endgame"},
            {
                {{"先手定形", "Settle shape in sente"},
                 253,
                 Rank::k4K,
                 Rank::k3D},  //
                {{"注意细微差别", "Observe suble difference"},
                 272,
                 Rank::k4K,
                 Rank::k2D},  //
                {{"小棋盘官子", "Small board endgame"},
                 167,
                 Rank::k5K,
                 Rank::k5D},  //
                {{"收束次序", "Order of endgame moves"},
                 169,
                 Rank::k6K,
                 Rank::k2D},  //
                {{"地中的手段", "Technique for securing territory"},
                 207,
                 Rank::k10K,
                 Rank::k5D},  //
                {{"边角常型收束", "Standard endgame in corner and side"},
                 252,
                 Rank::k8K,
                 Rank::k5D},  //
                {{"先手获利", "Profit in sente"},
                 254,
                 Rank::k5K,
                 Rank::k4D},                                                  //
                {{"官子手筋", "Endgame tesuji"}, 265, Rank::k6K, Rank::k5D},  //
                {{"利用死活问题获利", "Profit using life and death"},
                 266,
                 Rank::k1K,
                 Rank::k5D},  //
                {{"补棋的方法", "Technique for reinforcing groups"},
                 267,
                 Rank::k6K,
                 Rank::k4D},  //
                {{"粘劫收后", "Complete ko to secure endgame advantage"},
                 274,
                 Rank::k4K,
                 Rank::k3D},  //
                {{"入侵的手段", "Techniques for invading"},
                 263,
                 Rank::k7K,
                 Rank::k5D},  //
                {{"搜刮的手段", "Techniques for plundering"},
                 264,
                 Rank::k5K,
                 Rank::k5D},  //
            },
        },
        {
            {"布局", "Opening"},
            {
                {{"棋子的根据地", "Group's base"},
                 276,
                 Rank::k11K,
                 Rank::k2D},  //
                {{"布局基本下法", "Opening fundamentals"},
                 278,
                 Rank::k9K,
                 Rank::k5K},  //
                {{"金角银边草肚皮",
                  "Corner is gold, side is silver, center is grass"},
                 279,
                 Rank::k5K,
                 Rank::k2K},  //
                {{"棋往宽处走", "Move towards empty space"},
                 280,
                 Rank::k5K,
                 Rank::k2K},  //
                {{"方向选择", "Direction of play"},
                 282,
                 Rank::k5K,
                 Rank::k2K},                                                 //
                {{"价值比较", "Compare value"}, 283, Rank::k7K, Rank::k2D},  //
                {{"基本定式", "Joseki fundamentals"},
                 285,
                 Rank::k5K,
                 Rank::k1K},                                                //
                {{"定式之后", "After joseki"}, 286, Rank::k2K, Rank::k2K},  //
                {{"厚势的作用", "Use influence"},
                 287,
                 Rank::k4K,
                 Rank::k2D},                                            //
                {{"AI布局", "AI opening"}, 288, Rank::k2K, Rank::k2K},  //
                {{"势力消长的要点", "Influence key points"},
                 153,
                 Rank::k4K,
                 Rank::k2D},                                            //
                {{"大场", "Big point"}, 154, Rank::k4K, Rank::k1K},     //
                {{"急所", "Urgent point"}, 155, Rank::k9K, Rank::k2D},  //
                {{"拆二", "Two-space extension"},
                 156,
                 Rank::k10K,
                 Rank::k6K},                                                  //
                {{"夹击", "Pincer"}, 160, Rank::k8K, Rank::k2K},              //
                {{"定式选择", "Opening choice"}, 277, Rank::k4K, Rank::k2K},  //
                {{"三路和四路", "3rd and 4th line"},
                 281,
                 Rank::k5K,
                 Rank::k2K},  //
                {{"占角、守角和挂角", "Occupy, enclose and approach corners"},
                 284,
                 Rank::k11K,
                 Rank::k5K},                                               //
                {{"AI变化", "AI variations"}, 289, Rank::k1D, Rank::k1D},  //
            },
        },
        {
            {"中盘", "Middlegame"},
            {
                {{"征子的攻防", "Attack and defense of invading stones"},
                 248,
                 Rank::k1K,
                 Rank::k4D},                                             //
                {{"打入", "Invasion"}, 115, Rank::k8K, Rank::k1D},       //
                {{"搜根", "Find the root"}, 116, Rank::k5K, Rank::k2D},  //
                {{"侵消", "Reduction"}, 117, Rank::k4K, Rank::k2K},      //
                {{"封锁", "Seal in"}, 118, Rank::k8K, Rank::k4K},        //
                {{"整形", "Settle shape"}, 63, Rank::k2K, Rank::k1D},    //
                {{"攻击", "Attack"}, 64, Rank::k6K, Rank::k1D},          //
                {{"出头", "Get ahead"}, 251, Rank::k9K, Rank::k1K},      //
                {{"切断的处理", "Attack and defense: cuts"},
                 260,
                 Rank::k5K,
                 Rank::k2D},                                              //
                {{"反击", "Counter-attack"}, 295, Rank::k5K, Rank::k2D},  //
                {{"应对", "Counter"}, 298, Rank::k8K, Rank::k3D},         //
                {{"接触战的手筋", "Contact fight technique"},
                 249,
                 Rank::k2K,
                 Rank::k1D},  //
                {{"战斗的急所", "Urgent point of a fight"},
                 311,
                 Rank::k8K,
                 Rank::k1D},  //
                {{"把握战机", "Seize the opportunity"},
                 312,
                 Rank::k1K,
                 Rank::k2D},  //
                {{"战斗的选择", "Choose the fight"},
                 313,
                 Rank::k3D,
                 Rank::k4D},  //
                {{"大模样作战", "Large moyo fight"},
                 314,
                 Rank::k4K,
                 Rank::k1K},  //
                {{"补棋的方法", "Reinforcing technique"},
                 255,
                 Rank::k9K,
                 Rank::k3K},                                                //
                {{"基本行棋", "Basic moves"}, 258, Rank::k14K, Rank::k3K},  //
                {{"破坏棋形", "Break shape"}, 296, Rank::k11K, Rank::k1D},  //
                {{"定形技巧", "Make shape"}, 299, Rank::k7K, Rank::k1D},    //
                {{"突围", "Break out"}, 240, Rank::k2K, Rank::k1D},         //
            },
        },
        {
            {"综合", "Comprehensive tasks"},
            {
                {{"区分劫的先后手", "Order of moves in a ko"},
                 221,
                 Rank::k4K,
                 Rank::k2D},  //
                {{"复合问题", "Composite problems"},
                 224,
                 Rank::k1K,
                 Rank::k5D},                                                  //
                {{"盲点", "Blind spot"}, 17, Rank::k4K, Rank::k5D},           //
                {{"行棋次序", "Order of moves"}, 168, Rank::k9K, Rank::k5D},  //
                {{"双方要点", "Vital point for both sides"},
                 119,
                 Rank::k11K,
                 Rank::k4D},  //
                {{"一路妙手", "1st line brilliant move"},
                 173,
                 Rank::k11K,
                 Rank::k6D},                                                  //
                {{"左右同型", "Symmetric shape"}, 6, Rank::k13K, Rank::k5D},  //
                {{"做劫", "Make ko"}, 181, Rank::k14K, Rank::k5D},            //
                {{"避劫", "Avoid ko"}, 182, Rank::k11K, Rank::k5D},           //
                {{"一子两用", "One stone, two purposes"},
                 188,
                 Rank::k11K,
                 Rank::k5D},  //
                {{"不要忽略对方的抵抗", "Do not underestimate opponent"},
                 194,
                 Rank::k5K,
                 Rank::k5D},  //
                {{"区分劫的种类", "Types of ko"},
                 195,
                 Rank::k3K,
                 Rank::k3D},                                          //
                {{"弃子", "Sacrifice"}, 71, Rank::k11K, Rank::k5D},   //
                {{"双活", "Seki"}, 15, Rank::k15K, Rank::k5D},        //
                {{"连环劫", "Double ko"}, 36, Rank::k9K, Rank::k5D},  //
                {{"打劫", "Ko"}, 22, Rank::k13K, Rank::k6D},          //
                {{"棋形要点", "Shape's vital point"},
                 214,
                 Rank::k9K,
                 Rank::k5D},  //
                {{"一二妙手", "A few brilliant moves"},
                 45,
                 Rank::k11K,
                 Rank::k6D},  //
            },
        },
        {
            {"趣题", "Interesting tasks"},
            {
                {{"文字题", "Text problems"}, 243, Rank::k2K, Rank::k2D},  //
                {{"4路死活", "4x4 life and death"},
                 244,
                 Rank::k15K,
                 Rank::k3D},                                                  //
                {{"盘龙眼", "Two-headed dragon"}, 79, Rank::k9K, Rank::k1K},  //
                {{"5路官子", "5x5 endgame"}, 273, Rank::k3K, Rank::k3D},
                {{"4路官子", "4x4 endgame"}, 245, Rank::k10K, Rank::k3D},  //
            },
        },
};

static std::string time_challenge_label(Rank rank, int total, int fails) {
  std::ostringstream ss;
  ss << "<span size=\"x-large\">" << rank_string(rank) << u8"</span>\n✓"
     << (total - fails) << "\tx" << fails << "\n"
     << 100 * (total - fails) / std::max(total, 1) << "%";
  return ss.str();
}

static std::string topic_training_label(Rank rank, int total, int fails) {
  std::ostringstream ss;
  ss << "<span ";
  if (total > fails) ss << "weight=\"bold\" foreground=\"deepskyblue\"";
  ss << ">" << rank_string(rank) << "</span>";
  return ss.str();
}

SolvePresetWindow::SolvePresetWindow(AppContext& ctx) : Window(ctx) {
  gtk_window_set_title(GTK_WINDOW(window_), "Training");
  gtk_window_set_default_size(GTK_WINDOW(window_), 900, 550);

  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

  GtkWidget* stack_sidebar = gtk_stack_sidebar_new();
  GtkWidget* stack = gtk_stack_new();
  gtk_box_append(GTK_BOX(box), stack_sidebar);
  gtk_box_append(GTK_BOX(box), stack);
  gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(stack_sidebar),
                              GTK_STACK(stack));

  //================================================================================
  {  // Time challenge
    GtkWidget* preset_grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(preset_grid), true);
    gtk_grid_set_column_homogeneous(GTK_GRID(preset_grid), true);
    gtk_grid_attach(GTK_GRID(preset_grid),
                    gtk_label_new("Time challenge (45s)"), 0, 0, 1, 1);
    for (int rank = (int)Rank::k15K; rank <= (int)Rank::k7D; ++rank) {
      SolvePreset preset;
      preset.description_ =
          std::string("Time challenge (45s) - ") + rank_string(Rank(rank));
      preset.types_ = {
          TaskType::kLifeAndDeath,
          TaskType::kTesuji,
          TaskType::kCapture,
          TaskType::kCaptureRace,
      };
      preset.min_rank_ = Rank(rank - 1);
      preset.max_rank_ = Rank(rank);
      preset.time_limit_sec_ = 45;
      preset.max_tasks_ = 10;
      preset.max_errors_ = 2;

      presets_.emplace_back(preset,
                            std::make_pair(kTimeChallengePreset, Rank(rank)));

      GtkWidget* preset_label = gtk_label_new("");
      gtk_label_set_justify(GTK_LABEL(preset_label), GTK_JUSTIFY_CENTER);
      GtkWidget* preset_button = gtk_button_new();
      gtk_button_set_child(GTK_BUTTON(preset_button), preset_label);
      g_object_set_data(G_OBJECT(preset_button), "preset_index",
                        (gpointer)(presets_.size() - 1));
      g_signal_connect(preset_button, "clicked", G_CALLBACK(on_preset_clicked),
                       this);
      preset_buttons_[kTimeChallengePreset][Rank(rank)].push_back(
          preset_button);

      const int index = rank - (int)Rank::k15K;
      const int row = index / 5;
      const int col = index % 5;
      gtk_grid_attach(GTK_GRID(preset_grid), preset_button, col, row, 1, 1);
    }

    GtkWidget* scrolled = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(GTK_WIDGET(scrolled), true);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), preset_grid);
    gtk_stack_add_titled(GTK_STACK(stack), scrolled, nullptr,
                         "Time challenge (45s)");
  }
  //================================================================================
  for (const auto& [header_and_translation, sections] : kTopicPresets) {
    GtkWidget* preset_grid = gtk_grid_new();
    int cur_row = 0;
    for (const auto& [tag_and_translation, tag_id, from_rank, to_rank] :
         sections) {
      const auto& [tag, translation] = tag_and_translation;
      gtk_grid_attach(GTK_GRID(preset_grid), gtk_label_new(translation), 0,
                      cur_row, 1, 1);
      for (int rank = (int)from_rank; rank <= (int)to_rank; ++rank) {
        SolvePreset preset;
        preset.description_ =
            std::string(translation) + " (2min) - " + rank_string(Rank(rank));
        preset.tags_ = {tag};
        preset.min_rank_ = Rank(rank - 1);
        preset.max_rank_ = Rank(rank);
        preset.time_limit_sec_ = 180;
        preset.max_tasks_ = 10;
        preset.max_errors_ = 2;

        presets_.emplace_back(preset, std::make_pair(tag_id, Rank(rank)));

        GtkWidget* preset_label = gtk_label_new("");
        gtk_label_set_justify(GTK_LABEL(preset_label), GTK_JUSTIFY_CENTER);
        GtkWidget* preset_button = gtk_button_new();
        gtk_button_set_child(GTK_BUTTON(preset_button), preset_label);
        g_object_set_data(G_OBJECT(preset_button), "preset_index",
                          (gpointer)(presets_.size() - 1));
        g_signal_connect(preset_button, "clicked",
                         G_CALLBACK(on_preset_clicked), this);
        preset_buttons_[tag_id][Rank(rank)].push_back(preset_button);
        gtk_grid_attach(GTK_GRID(preset_grid), preset_button,
                        1 + rank - (int)Rank::k15K, cur_row, 1, 1);
      }
      cur_row++;
    }
    const auto& [name, translation] = header_and_translation;
    GtkWidget* scrolled = gtk_scrolled_window_new();
    gtk_widget_set_hexpand(GTK_WIDGET(scrolled), true);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), preset_grid);
    gtk_stack_add_titled(GTK_STACK(stack), scrolled, nullptr, translation);
  }

  update_preset_buttons();

  gtk_window_set_child(GTK_WINDOW(window_), box);
  gtk_window_present(GTK_WINDOW(window_));
  window_groups_[kSolvePresetWindowGroup].insert(this);
}

SolvePresetWindow::~SolvePresetWindow() {
  window_groups_[kSolvePresetWindowGroup].erase(this);
}

void SolvePresetWindow::update_preset_buttons() {
  const auto tag_stats = ctx_.stats().get_tag_stats();
  for (const auto& [tag_id, ranked_buttons] : preset_buttons_) {
    auto tag_it = tag_stats.find(tag_id);
    if (tag_it == tag_stats.end()) {
      LOG(ERROR) << "tag not found: " << tag_id;
      continue;
    }
    bool first = true;
    for (const auto& [rank, buttons] : ranked_buttons) {
      if (first) {
        for (GtkWidget* button : buttons) {
          gtk_widget_set_sensitive(GTK_WIDGET(button), true);
          update_time_challenge_button_label(button, tag_id, rank,
                                             tag_it->second);
        }
        first = false;
        continue;
      }
      auto rank_it = tag_it->second.find((Rank)((int)rank - 1));
      const auto& [total, fails] = rank_it != tag_it->second.end()
                                       ? rank_it->second
                                       : std::make_pair(0, 0);
      const bool sensitive = rank_it == tag_it->second.end() || (total > fails);
      for (GtkWidget* button : buttons) {
        gtk_widget_set_sensitive(GTK_WIDGET(button), sensitive);
        update_time_challenge_button_label(button, tag_id, rank,
                                           tag_it->second);
      }
    }
  }
}

void SolvePresetWindow::update_time_challenge_button_label(
    GtkWidget* button, int tag_id, Rank rank,
    const std::map<Rank, std::pair<int, int>>& stats) {
  GtkWidget* label = gtk_button_get_child(GTK_BUTTON(button));
  assert(label != nullptr);
  int total = 0;
  int fails = 0;
  if (auto it = stats.find(rank); it != stats.end()) {
    total = it->second.first;
    fails = it->second.second;
  }
  if (tag_id == kTimeChallengePreset) {
    gtk_label_set_markup(GTK_LABEL(label),
                         time_challenge_label(rank, total, fails).c_str());
  } else {
    gtk_label_set_markup(GTK_LABEL(label),
                         topic_training_label(rank, total, fails).c_str());
  }
}

void SolvePresetWindow::on_preset_clicked(GtkWidget* self, gpointer data) {
  SolvePresetWindow* win = (SolvePresetWindow*)data;
  const size_t index =
      (size_t)g_object_get_data(G_OBJECT(self), "preset_index");
  new SolveWindow(win->ctx_, win->presets_[index].first,
                  win->presets_[index].second);
}

}  // namespace ui