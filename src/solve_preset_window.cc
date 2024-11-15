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

const std::vector<
    std::pair<string_pair, std::vector<std::tuple<string_pair, Rank, Rank>>>>
    kTopicPresets = {
        {
            {"启蒙", "Beginner"},
            {
                {{"一步吃子", "Capture in one move"},
                 Rank::k15K,
                 Rank::k11K},  //
                {{"一步逃子", "Escape in one move"}, Rank::k15K, Rank::k12K},
                {{"一步连接", "Connect in one move"},
                 Rank::k15K,
                 Rank::k10K},                                                 //
                {{"一步分断", "Split in one move"}, Rank::k15K, Rank::k12K},  //
                {{"门吃", "Capture by atari"}, Rank::k15K, Rank::k10K},       //
                {{"边线吃子", "Capture on the side"},
                 Rank::k15K,
                 Rank::k7K},  //
                {{"棋子的气", "Group liberties"}, Rank::k15K, Rank::k15K},
                {{"逃子方向", "Direction of escape"},
                 Rank::k14K,
                 Rank::k6K},  //
                {{"要子与废子", "Vital and useless stones"},
                 Rank::k13K,
                 Rank::k7K},                                               //
                {{"逃子", "Escape"}, Rank::k14K, Rank::k7K},               //
                {{"围空", "Surround territory"}, Rank::k12K, Rank::k10K},  //
                {{"吃子方向", "Direction of capture"},
                 Rank::k15K,
                 Rank::k6K},  //
            },
        },
        {
            {"吃子技巧", "Capturing techniques"},
            {
                {{"宽征", "Loose ladder"}, Rank::k13K, Rank::k1K},        //
                {{"乌龟不出头", "Crane's nest"}, Rank::k14K, Rank::k3K},  //
                {{"倒扑", "Snapback"}, Rank::k15K, Rank::k8K},            //
                {{"双吃", "Double atari"}, Rank::k15K, Rank::k11K},       //
                {{"夹吃", "Clamp capture"}, Rank::k12K, Rank::k7K},       //
                {{"征吃", "Ladder capture"}, Rank::k14K, Rank::k8K},      //
                {{"扑吃", "Snapback capture"}, Rank::k14K, Rank::k7K},    //
                {{"抱吃", "Capture by atari"}, Rank::k15K, Rank::k12K},   //
                {{"挖吃", "Wedging capture"}, Rank::k14K, Rank::k6K},     //
                {{"枷吃", "Net capture"}, Rank::k13K, Rank::k8K},         //
                {{"靠单", "Prevent the bamboo joint"},
                 Rank::k9K,
                 Rank::k4K},                                             //
                {{"相思断", "Lovesick cut"}, Rank::k8K, Rank::k4K},      //
                {{"滚打包收", "Squeeze"}, Rank::k14K, Rank::k2D},        //
                {{"接不归", "Connect and die"}, Rank::k15K, Rank::k7K},  //
            },
        },
        {
            {"基本手筋", "Basic tesuji"},
            {
                {{"尖", "Diagonal"}, Rank::k13K, Rank::k5D},               //
                {{"立", "Descent"}, Rank::k13K, Rank::k5D},                //
                {{"挤", "Kosumi wedge"}, Rank::k13K, Rank::k5D},           //
                {{"断", "Cut"}, Rank::k15K, Rank::k5D},                    //
                {{"点", "Placement"}, Rank::k13K, Rank::k5D},              //
                {{"飞", "Knight's move"}, Rank::k7K, Rank::k5D},           //
                {{"虎", "Tiger's mouth"}, Rank::k13K, Rank::k5D},          //
                {{"跳", "Jump"}, Rank::k11K, Rank::k5D},                   //
                {{"夹", "Clamp"}, Rank::k11K, Rank::k5D},                  //
                {{"顶", "Bump"}, Rank::k11K, Rank::k5D},                   //
                {{"挖", "Wedge"}, Rank::k13K, Rank::k5D},                  //
                {{"扑", "Throw-in"}, Rank::k15K, Rank::k5D},               //
                {{"跨", "Cut accross"}, Rank::k8K, Rank::k4D},             //
                {{"托", "Underneath attachment"}, Rank::k12K, Rank::k5D},  //
                {{"枷", "Net"}, Rank::k4K, Rank::k2D},                     //
                {{"靠", "Contact play"}, Rank::k10K, Rank::k5D},           //
                {{"大飞", "Large knight's move"}, Rank::k7K, Rank::k1D},   //
                {{"扳", "Hane"}, Rank::k15K, Rank::k5D},                   //
                {{"冲", "Push"}, Rank::k15K, Rank::k5D},                   //
                {{"滚打", "Squeeze"}, Rank::k11K, Rank::k4D},              //
                {{"爬", "Crawl"}, Rank::k11K, Rank::k5D},                  //
                {{"弯", "Bend"}, Rank::k9K, Rank::k5D},                    //
                {{"长", "Stretch"}, Rank::k11K, Rank::k1D},                //
                {{"接", "Connect"}, Rank::k14K, Rank::k4D},                //
            },
        },
        {
            {"对杀", "Capturing races"},
            {
                {{"有眼杀无眼", "Eye vs no-eye"}, Rank::k13K, Rank::k3D},  //
                {{"大眼杀小眼", "Big eye vs small eye"},
                 Rank::k7K,
                 Rank::k2K},                                            //
                {{"紧气", "Reduce liberties"}, Rank::k15K, Rank::k2D},  //
                {{"使对方不入", "Prevent opponent from approaching"},
                 Rank::k15K,
                 Rank::k2D},                                              //
                {{"延气", "Increase liberties"}, Rank::k12K, Rank::k4D},  //
                {{"黄莺扑蝶", "The oriole captures the butterfly"},
                 Rank::k1K,
                 Rank::k1D},  //
                {{"紧气要点", "Vital point for reducing liberties"},
                 Rank::k13K,
                 Rank::k4D},
                {{"两扳长一气", "Two hane grow one liberty"},
                 Rank::k5K,
                 Rank::k2K},                                                  //
                {{"大眼的气", "Big eye's liberties"}, Rank::k9K, Rank::k3K},  //
                {{"做眼", "Make eye"}, Rank::k13K, Rank::k2K},                //
                {{"破眼", "Break eye"}, Rank::k13K, Rank::k6K},               //
                {{"大头鬼", "Tombstone squeeze"}, Rank::k9K, Rank::k2D},      //
                {{"延气要点", "Vital point for increasing liberties"},
                 Rank::k9K,
                 Rank::k1K},                                              //
                {{"间接进攻", "Indirect attack"}, Rank::k4K, Rank::k4D},  //
                {{"对杀要点", "Vital point for capturing race"},
                 Rank::k13K,
                 Rank::k4D},  //
                {{"金鸡独立", "Golden chicken stands on one leg"},
                 Rank::k13K,
                 Rank::k5D},  //
                {{"常型对杀", "Standard capturing races"},
                 Rank::k11K,
                 Rank::k4D},  //
            },
        },
        {
            {"基本死活", "Basic life and death"},
            {
                {{"直三", "Straight three"}, Rank::k15K, Rank::k9K},   //
                {{"弯三", "Bent three"}, Rank::k15K, Rank::k8K},       //
                {{"丁四", "Pyramid four"}, Rank::k15K, Rank::k9K},     //
                {{"刀把五", "Bulky four"}, Rank::k14K, Rank::k7K},     //
                {{"梅花五", "Crossed five"}, Rank::k15K, Rank::k13K},  //
                {{"葡萄六", "Flower six"}, Rank::k14K, Rank::k8K},     //
                {{"一步做眼", "Make eye in one move"},
                 Rank::k15K,
                 Rank::k10K},  //
                {{"一步破眼", "Break eye in one move"},
                 Rank::k15K,
                 Rank::k10K},  //
                {{"三眼两做", "Three eyes and two actions"},
                 Rank::k12K,
                 Rank::k3K},                                                  //
                {{"先手做眼", "Make eye in sente"}, Rank::k11K, Rank::k5D},   //
                {{"先手破眼", "Break eye in sente"}, Rank::k12K, Rank::k5D},  //
                {{"做眼", "Make eye"}, Rank::k14K, Rank::k5K},                //
                {{"破眼", "Break eye"}, Rank::k14K, Rank::k4K},               //
                {{"直四", "Straight four"}, Rank::k14K, Rank::k12K},          //
                {{"弯四", "Bent four"}, Rank::k14K, Rank::k7K},               //
                {{"胀牯牛", "Squash"}, Rank::k13K, Rank::k2D},                //
                {{"板六", "Rectangular six"}, Rank::k11K, Rank::k6K},         //
                {{"真眼和假眼", "Real eye and false eye"},
                 Rank::k13K,
                 Rank::k11K},
                {{"吃子方向", "Capture stone direction"},
                 Rank::k15K,
                 Rank::k6K},  //
            },
        },
        {
            {"死活", "Life and death"},
            {
                {{"有眼杀无眼", "Eye vs no-eye"}, Rank::k13K, Rank::k3D},     //
                {{"扩大眼位", "Increase eye space"}, Rank::k13K, Rank::k5D},  //
                {{"缩小眼位", "Reduce eye space"}, Rank::k11K, Rank::k5D},    //
                {{"内部动手", "Inside moves"}, Rank::k8K, Rank::k5D},         //
                {{"聚杀", "Inside kill"}, Rank::k13K, Rank::k5D},             //
                {{"利用气紧", "Use shortage of liberties"},
                 Rank::k13K,
                 Rank::k5D},  //
                {{"利用棋形弱点", "Exploit shape weakness"},
                 Rank::k11K,
                 Rank::k5D},
                {{"盘角曲四", "Bent four in the corner"},
                 Rank::k12K,
                 Rank::k4D},                                                 //
                {{"胀牯牛", "Squash"}, Rank::k13K, Rank::k2D},               //
                {{"一一妙手", "Brilliant sequence"}, Rank::k6K, Rank::k5D},  //
                {{"提子后的杀着", "Kill after capture"},
                 Rank::k6K,
                 Rank::k5D},                                          //
                {{"组合手段", "Combination"}, Rank::k1K, Rank::k5D},  //
                {{"导致气紧", "Create shortage of liberties"},
                 Rank::k10K,
                 Rank::k5D},  //
                {{"利用外围棋子", "Use outside stones"},
                 Rank::k5K,
                 Rank::k5D},  //
                {{"利用角部特殊性", "Use corner special properties"},
                 Rank::k10K,
                 Rank::k3D},                                                 //
                {{"倒脱靴", "Under the stones"}, Rank::k13K, Rank::k5D},     //
                {{"利用倒扑", "Use snapback"}, Rank::k12K, Rank::k4D},       //
                {{"寻求借用", "Look for leverage"}, Rank::k11K, Rank::k5D},  //
                {{"杀棋要点", "Vital point for kill"},
                 Rank::k11K,
                 Rank::k5D},  //
                {{"打二还一", "Capture two - recapture one"},
                 Rank::k14K,
                 Rank::k1D},  //
                {{"利用接不归", "Use connect-and-die"},
                 Rank::k11K,
                 Rank::k5D},                                                  //
                {{"做眼", "Make eye"}, Rank::k12K, Rank::k5D},                //
                {{"破眼", "Break eye"}, Rank::k12K, Rank::k5D},               //
                {{"先手破眼", "Break eye in sente"}, Rank::k12K, Rank::k5D},  //
                {{"先手做眼", "Make eye in sente"}, Rank::k11K, Rank::k5D},   //
                {{"吃子做活", "Capture to live"}, Rank::k11K, Rank::k4D},     //
                {{"避免被聚杀", "Avoid making dead shape"},
                 Rank::k11K,
                 Rank::k2D},                                             //
                {{"做劫", "Make ko"}, Rank::k14K, Rank::k5D},            //
                {{"保留先手", "Keep sente"}, Rank::k5K, Rank::k5D},      //
                {{"双倒扑", "Double snapback"}, Rank::k14K, Rank::k1K},  //
                {{"点杀", "Kill by eye-point placement"},
                 Rank::k12K,
                 Rank::k5D},                                                //
                {{"渡", "Bridge under"}, Rank::k14K, Rank::k5D},            //
                {{"分断", "Cut"}, Rank::k14K, Rank::k4D},                   //
                {{"防范弱点", "Defend weak point"}, Rank::k8K, Rank::k4D},  //
                {{"避开陷阱", "Avoid trap"}, Rank::k8K, Rank::k5D},         //
                {{"试探应手", "Probe"}, Rank::k2K, Rank::k4D},              //
                {{"利用一路硬腿", "Use descent to first line"},
                 Rank::k10K,
                 Rank::k4D},                                              //
                {{"双提", "Double capture"}, Rank::k2K, Rank::k1D},       //
                {{"阻渡", "Prevent escape"}, Rank::k10K, Rank::k3D},      //
                {{"出动残子", "Run weak group"}, Rank::k10K, Rank::k5D},  //
                {{"利用对方死活", "Use opponent's life and death"},
                 Rank::k3K,
                 Rank::k5D},                                   //
                {{"连络", "Connect"}, Rank::k15K, Rank::k4D},  //
                {{"防守入侵", "Defend from invasion"},
                 Rank::k12K,
                 Rank::k5D},                                              //
                {{"间接进攻", "Indirect attack"}, Rank::k4K, Rank::k4D},  //
                {{"活棋要点", "Vital point for life"},
                 Rank::k13K,
                 Rank::k5D},                                                 //
                {{"老鼠偷油", "Mouse stealing oil"}, Rank::k5K, Rank::k1D},  //
                {{"金鸡独立", "Golden chicken stands on one leg"},
                 Rank::k13K,
                 Rank::k5D},  //
            },
        },
        {
            {"常型死活", "Common life and death"},
            {
                {{"大猪嘴及类似型", "Big pig snout and similar"},
                 Rank::k4K,
                 Rank::k3D},  //
                {{"小猪嘴及类似型", "Small pig snout and similar"},
                 Rank::k4K,
                 Rank::k2K},                                     //
                {{"二线型", "2nd line"}, Rank::k6K, Rank::k2K},  //
                {{"金柜角及类似型", "Carpenter's square and similar"},
                 Rank::k1K,
                 Rank::k4D},                                                 //
                {{"边部常型", "Side common shapes"}, Rank::k5K, Rank::k3D},  //
                {{"角部常型", "Corner common shapes"},
                 Rank::k8K,
                 Rank::k5D},  //
            },
        },
        {
            {"基础官子", "Basic endgame"},
            {
                {{"目与单官", "Fill neutral points"},
                 Rank::k12K,
                 Rank::k10K},  //
                {{"双先官子", "Double sente endgame"},
                 Rank::k8K,
                 Rank::k5K},                                             //
                {{"破目", "Break points"}, Rank::k14K, Rank::k8K},       //
                {{"守目", "Defend points"}, Rank::k12K, Rank::k6K},      //
                {{"比较大小", "Compare value"}, Rank::k10K, Rank::k1K},  //
                {{"基本收官", "Endgame fundamentals"}, Rank::k15K, Rank::k3K},
                {{"先手与后手", "Sente and gote"}, Rank::k8K, Rank::k3K},  //
            },
        },
        {
            {"官子", "Endgame"},
            {
                {{"先手定形", "Settle shape in sente"},
                 Rank::k4K,
                 Rank::k3D},  //
                {{"注意细微差别", "Observe suble difference"},
                 Rank::k4K,
                 Rank::k2D},  //
                {{"小棋盘官子", "Small board endgame"},
                 Rank::k5K,
                 Rank::k5D},  //
                {{"收束次序", "Order of endgame moves"},
                 Rank::k6K,
                 Rank::k2D},  //
                {{"地中的手段", "Technique for securing territory"},
                 Rank::k10K,
                 Rank::k5D},  //
                {{"边角常型收束", "Standard endgame in corner and side"},
                 Rank::k8K,
                 Rank::k5D},                                              //
                {{"先手获利", "Profit in sente"}, Rank::k5K, Rank::k4D},  //
                {{"官子手筋", "Endgame tesuji"}, Rank::k6K, Rank::k5D},   //
                {{"利用死活问题获利", "Profit using life and death"},
                 Rank::k1K,
                 Rank::k5D},  //
                {{"补棋的方法", "Technique for reinforcing groups"},
                 Rank::k6K,
                 Rank::k4D},  //
                {{"粘劫收后", "Complete ko to secure endgame advantage"},
                 Rank::k4K,
                 Rank::k3D},  //
                {{"入侵的手段", "Techniques for invading"},
                 Rank::k7K,
                 Rank::k5D},  //
                {{"搜刮的手段", "Techniques for plundering"},
                 Rank::k5K,
                 Rank::k5D},  //
            },
        },
        {
            {"布局", "Opening"},
            {
                {{"棋子的根据地", "Group's base"}, Rank::k11K, Rank::k2D},  //
                {{"布局基本下法", "Opening fundamentals"},
                 Rank::k9K,
                 Rank::k5K},  //
                {{"金角银边草肚皮",
                  "Corner is gold, side is silver, center is grass"},
                 Rank::k5K,
                 Rank::k2K},  //
                {{"棋往宽处走", "Move towards empty space"},
                 Rank::k5K,
                 Rank::k2K},                                                  //
                {{"方向选择", "Direction of play"}, Rank::k5K, Rank::k2K},    //
                {{"价值比较", "Compare value"}, Rank::k7K, Rank::k2D},        //
                {{"基本定式", "Joseki fundamentals"}, Rank::k5K, Rank::k1K},  //
                {{"定式之后", "After joseki"}, Rank::k2K, Rank::k2K},         //
                {{"厚势的作用", "Use influence"}, Rank::k4K, Rank::k2D},      //
                {{"AI布局", "AI opening"}, Rank::k2K, Rank::k2K},             //
                {{"势力消长的要点", "Influence key points"},
                 Rank::k4K,
                 Rank::k2D},                                                 //
                {{"大场", "Big point"}, Rank::k4K, Rank::k1K},               //
                {{"急所", "Urgent point"}, Rank::k9K, Rank::k2D},            //
                {{"拆二", "Two-space extension"}, Rank::k10K, Rank::k6K},    //
                {{"夹击", "Pincer"}, Rank::k8K, Rank::k2K},                  //
                {{"定式选择", "Opening choice"}, Rank::k4K, Rank::k2K},      //
                {{"三路和四路", "3rd and 4th line"}, Rank::k5K, Rank::k2K},  //
                {{"占角、守角和挂角", "Occupy, enclose and approach corners"},
                 Rank::k11K,
                 Rank::k5K},                                          //
                {{"AI变化", "AI variations"}, Rank::k1D, Rank::k1D},  //
            },
        },
        {
            {"中盘", "Middlegame"},
            {
                {{"征子的攻防", "Attack and defense of invading stones"},
                 Rank::k1K,
                 Rank::k4D},                                        //
                {{"打入", "Invasion"}, Rank::k8K, Rank::k1D},       //
                {{"搜根", "Find the root"}, Rank::k5K, Rank::k2D},  //
                {{"侵消", "Reduction"}, Rank::k4K, Rank::k2K},      //
                {{"封锁", "Seal in"}, Rank::k8K, Rank::k4K},        //
                {{"整形", "Settle shape"}, Rank::k2K, Rank::k1D},   //
                {{"攻击", "Attack"}, Rank::k6K, Rank::k1D},         //
                {{"出头", "Get ahead"}, Rank::k9K, Rank::k1K},      //
                {{"切断的处理", "Attack and defense: cuts"},
                 Rank::k5K,
                 Rank::k2D},                                         //
                {{"反击", "Counter-attack"}, Rank::k5K, Rank::k2D},  //
                {{"应对", "Counter"}, Rank::k8K, Rank::k3D},         //
                {{"接触战的手筋", "Contact fight technique"},
                 Rank::k2K,
                 Rank::k1D},  //
                {{"战斗的急所", "Urgent point of a fight"},
                 Rank::k8K,
                 Rank::k1D},  //
                {{"把握战机", "Seize the opportunity"},
                 Rank::k1K,
                 Rank::k2D},                                                 //
                {{"战斗的选择", "Choose the fight"}, Rank::k3D, Rank::k4D},  //
                {{"大模样作战", "Large moyo fight"}, Rank::k4K, Rank::k1K},  //
                {{"补棋的方法", "Reinforcing technique"},
                 Rank::k9K,
                 Rank::k3K},                                           //
                {{"基本行棋", "Basic moves"}, Rank::k14K, Rank::k3K},  //
                {{"破坏棋形", "Break shape"}, Rank::k11K, Rank::k1D},  //
                {{"定形技巧", "Make shape"}, Rank::k7K, Rank::k1D},    //
                {{"突围", "Break out"}, Rank::k2K, Rank::k1D},         //
            },
        },
        {
            {"综合", "Comprehensive tasks"},
            {
                {{"区分劫的先后手", "Order of moves in a ko"},
                 Rank::k4K,
                 Rank::k2D},                                                 //
                {{"复合问题", "Composite problems"}, Rank::k1K, Rank::k5D},  //
                {{"盲点", "Blind spot"}, Rank::k4K, Rank::k5D},              //
                {{"行棋次序", "Order of moves"}, Rank::k9K, Rank::k5D},      //
                {{"双方要点", "Vital point for both sides"},
                 Rank::k11K,
                 Rank::k4D},  //
                {{"一路妙手", "1st line brilliant move"},
                 Rank::k11K,
                 Rank::k6D},                                               //
                {{"左右同型", "Symmetric shape"}, Rank::k13K, Rank::k5D},  //
                {{"做劫", "Make ko"}, Rank::k14K, Rank::k5D},              //
                {{"避劫", "Avoid ko"}, Rank::k11K, Rank::k5D},             //
                {{"一子两用", "One stone, two purposes"},
                 Rank::k11K,
                 Rank::k5D},  //
                {{"不要忽略对方的抵抗", "Do not underestimate opponent"},
                 Rank::k5K,
                 Rank::k5D},                                                  //
                {{"区分劫的种类", "Types of ko"}, Rank::k3K, Rank::k3D},      //
                {{"弃子", "Sacrifice"}, Rank::k11K, Rank::k5D},               //
                {{"双活", "Seki"}, Rank::k15K, Rank::k5D},                    //
                {{"连环劫", "Double ko"}, Rank::k9K, Rank::k5D},              //
                {{"打劫", "Ko"}, Rank::k13K, Rank::k6D},                      //
                {{"棋形要点", "Shape's vital point"}, Rank::k9K, Rank::k5D},  //
                {{"一二妙手", "A few brilliant moves"},
                 Rank::k11K,
                 Rank::k6D},  //
            },
        },
        {
            {"趣题", "Interesting tasks"},
            {
                {{"文字题", "Text problems"}, Rank::k2K, Rank::k2D},         //
                {{"4路死活", "4x4 life and death"}, Rank::k15K, Rank::k3D},  //
                {{"盘龙眼", "Two-headed dragon"}, Rank::k9K, Rank::k1K},     //
                {{"5路官子", "5x5 endgame"}, Rank::k3K, Rank::k3D},
                {{"4路官子", "4x4 endgame"}, Rank::k10K, Rank::k3D},  //
            },
        },
};

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

      presets_.push_back(preset);

      GtkWidget* preset_button =
          gtk_button_new_with_label(rank_string(Rank(rank)));
      g_object_set_data(G_OBJECT(preset_button), "preset_index",
                        (gpointer)(presets_.size() - 1));
      g_signal_connect(preset_button, "clicked", G_CALLBACK(on_preset_clicked),
                       this);

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
    for (const auto& [tag_and_translation, from_rank, to_rank] : sections) {
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
        preset.time_limit_sec_ = 120;
        preset.max_tasks_ = 10;

        presets_.push_back(preset);

        GtkWidget* preset_button =
            gtk_button_new_with_label(rank_string(Rank(rank)));
        g_object_set_data(G_OBJECT(preset_button), "preset_index",
                          (gpointer)(presets_.size() - 1));
        g_signal_connect(preset_button, "clicked",
                         G_CALLBACK(on_preset_clicked), this);
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

  gtk_window_set_child(GTK_WINDOW(window_), box);
  gtk_window_present(GTK_WINDOW(window_));
}

void SolvePresetWindow::on_preset_clicked(GtkWidget* self, gpointer data) {
  SolvePresetWindow* win = (SolvePresetWindow*)data;
  const size_t index =
      (size_t)g_object_get_data(G_OBJECT(self), "preset_index");
  new SolveWindow(win->ctx_, win->presets_[index]);
}

}  // namespace ui