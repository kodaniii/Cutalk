#include "defs.h"

QString GateServer_url_perfix = "";

std::function<void(QWidget*)> repolish = [](QWidget* w){
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString = [](QString input){
    QString result = input;
    int length = input.length();
    ushort xor_code = length % 255; //控制加密密钥key的长度8bit
    for (int i = 0; i < length; ++i) {
        // 对每个字符进行异或操作
        // 注意：这里假设字符都是ASCII，因此直接转换为QChar
        result[i] = QChar(static_cast<ushort>(input[i].unicode() ^ xor_code));
    }
    return result;
};


const std::vector<QString> strs = {
    "Hello, World!",
    "C++ is the best",
    "吃饭了吗？",
    "作业借我抄抄",
    "转让半盒热辣香骨鸡，肯德基疯狂星期四19.9入的，才吃了5块，里面还有10块，不舍得吃的时候就拿出来闻一闻，平常都是放在冰箱里，吃的时候就用公司的微波炉热一下，吃完都用钉书机钉起来，防止受潮。外表大概8成新吧。量很足，厚度约3cm，长度约6cm，包行货，不是外面的那种华莱士的，假一罚十，还有2盒甜辣酱一起送了，真的很好吃，平时小半块就可以回味半天了，价格私聊，非诚勿扰",
    "Welcome to the new era of communication!",
    "Let's embrace the challenges ahead.",
    "Together we can achieve great things.",
    "Stay focused on your goals, and you'll succeed.",
    "Every line of code is a step towards mastery.",
    "Innovation is the key to a brighter future.",
    "Knowledge is power, and sharing it is a virtue.",
    "The world is full of possibilities, just waiting to be discovered."
};

const std::vector<QString> apply_friend_strs = {
    "买挖掘机吗？",
    "就你叫夏洛啊",
    "作业写完了吗？加我，借我抄抄",
    "您的蜜雪冰城柠檬水分期支付即将逾期，请尽快同意！",
    "李阿姨相亲介绍的，男，今年19岁，计算机科学与技术专业",
    "Zestful greetings! Let's connect.",
    "Yo, fellow adventurer, let's be friends!",
    "X marks the spot where our friendship begins.",
    "Waving hello from across the digital sea.",
    "Vroom, let's race to a friendship.",
    "Up for a chat? I'm all ears.",
    "Together, let's Troy any challenge.",
    "Swooping in for a friendly chat.",
    "Rudy says hi! Let's be mates.",
};

const std::vector<QString> heads = {
    ":/res/head/head_1.jpg",
    ":/res/head/head_2.jpg",
    ":/res/head/head_3.jpg",
    ":/res/head/head_4.jpg",
    ":/res/head/head_5.jpg",
    ":/res/head/head_6.jpg",
    ":/res/head/head_7.jpg",
    ":/res/head/head_8.jpg",
    ":/res/head/head_9.jpg",
    ":/res/head/head_10.jpg",
    ":/res/head/head_11.jpg",
    ":/res/head/head_12.jpg",
    ":/res/head/head_13.jpg",
    ":/res/head/head_14.jpg",
    ":/res/head/head_15.jpg",
    ":/res/head/head_16.jpg",
    ":/res/head/head_17.jpg"
};

const std::vector<QString> names = {
    "alice",
    "bob",
    "charlie",
    "diana",
    "eve",
    "frank",
    "grace",
    "hank",
    "Zoro",
    "Yoda",
    "Xena",
    "Wade",
    "Vick",
    "Uma",
    "Troy",
    "Soren",
    "Rudy",
    "Quinn",
    "Phoebe",
    "Oscar",
    "Nora",
    "Mira",
    "Liam",
    "Kira",
    "Jules",
    "Ivy"
};
