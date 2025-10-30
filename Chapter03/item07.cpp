// 条款07 - 区别使用()和{}创建对象

#include <vector>
#include <atomic>
#include <string>

// 一、初始化五花八门（历史原因）

// 在内置类型上，上述四种初始化方式都可以使用，
namespace {

    void test() {
        int x(0);               //1.使用圆括号初始化

        int y = 0;              //2.使用"="初始化

        int z{ 0 };             //3.使用花括号初始化

        int z = { 0 };          //4.使用"="和花括号，C++通常把它视作和只有花括号一样(和3一样)

    }
}

// 但是在类类型上，这四种初始化方式的行为可能大不相同，
namespace {

    class Widget {
    };

    void test() {
        Widget w1;  //调用默认构造函数

        Widget w2 = w1; //调用拷贝构造函数

        w1 = w2; //调用拷贝赋值运算符
    }
}


// 二、统一初始化(括号初始化)

// 所谓统一初始化是指在任何涉及初始化的地方都使用单一的初始化语法
// 括号初始化中的括号 特指 花括号

namespace name
{
    // 情形 1 - 构造初始化
    std::vector<int> v{ 1, 3, 5 };  // v初始内容为1,3,5

    // 情形 2 - 括号初始化也能被用于为非静态数据成员指定默认初始值
    class Widget
    {
    private:
        int x{0};  // 没问题，x初始值为0
        int y = 0; // 也可以
        int z(0);  // 错误！
    };

    // 情形 3 - 不可拷贝的对象，可以使用花括号初始化或者圆括号初始化，但是不能使用"="初始化
    void test()
    {
        std::atomic<int> ai1{ 0 };      //没问题
        std::atomic<int> ai2(0);        //没问题
        std::atomic<int> ai3 = 0;       //错误！

    }

} // namespace name

// 结论：三种情形统一初始化都能适配，故而称之为《统一》初始化

// 三、统一初始化的优点

// 优点 1 - 防止窄化转换
// 统一初始化不允许内置类型间隐式的变窄转换（narrowing conversion），类型检查更严谨
// 但圆括号初始化因需兼容历史版本，并没有限制
namespace {
    void test() {
        double d = 3.14;

        int x1 = d;      //允许窄化转换，x1的值为3

        int x2(d);      //允许窄化转换，x2的值为3

        int x3{ d };    //错误！防止窄化转换

        int x4 = { d }; //错误！防止窄化转换
    }
} // namespace

// 优点 2 - 避免“最令人讨厌的解析”(most vexing parse)、
// C++规定任何可以被解析为一个声明的东西必须被解析为声明
// 想创建一个使用默认构造函数构造的对象，却不小心变成了函数声明。
namespace {
    class Widget {
    public:
        Widget() {}
    };

    void test() {
        Widget w1(); //错误！本意是调用widget的默认构造函数创建一个Widget对象
                        //被解析为函数声明，声明了一个名为w1的函数

        Widget w2{}; //正确！使用默认构造函数构造了一个Widget对象
    }
} // namespace


// 四、统一初始化的缺点

// 缺点 1 - auto 和 花括号初始化不兼容 （item02提到过类似的问题）
namespace {
    void test() {
        auto x1{ 3 }; //错误！编译器无法确定x1的类型，是int还是std::initializer_list<int>

        auto x2 = { 3 }; //正确！x2的类型是std::initializer_list<int>
    }
} // namespace


// 缺点 2 - 只要构造函数接受std::initializer_list参数，花括号初始化就会优先匹配它
// a、std::initializer_list参数的构造函数在编译器的优先级非常高
namespace {
    class Widget {
    public:
        Widget(int i, bool b);      // 构造函数 1 
        Widget(int i, double d);    // 构造函数 2 
        Widget(std::initializer_list<long double> il);    // 3 新添加的构造函数
    };

    void test() {
        // 1、 不存在接受std::initializer_list参数的构造函数时

        // Widget w1(10, true);            //调用第一个构造函数
        // Widget w2{10, true};            //也调用第一个构造函数
        // Widget w3(10, 5.0);             //调用第二个构造函数
        // Widget w4{10, 5.0};             //也调用第二个构造函数


        // 2、 存在接受std::initializer_list参数的构造函数时

        Widget w1(10, true);    //使用圆括号初始化，同之前一样
                        //调用第一个构造函数

        Widget w2{10, true};    //使用花括号初始化，但是现在
                                //调用带std::initializer_list的构造函数
                                //(10 和 true 转化为long double)

        Widget w3(10, 5.0);     //使用圆括号初始化，同之前一样
                                //调用第二个构造函数 

        Widget w4{10, 5.0};     //虽然实参完全匹配构造函数2，使用花括号初始化，但是现在
                                //调用带std::initializer_list的构造函数
                                //(10 和 5.0 转化为long double)
                                //会触发错误！要求变窄转换

        // 甚至普通构造函数和移动构造函数都会被带std::initializer_list的构造函数劫持

        Widget w5(w4);                  //使用圆括号，调用拷贝构造函数

        Widget w6{w4};                  //使用花括号，调用std::initializer_list构造
                                        //函数（w4转换为float，float转换为double）

        Widget w7(std::move(w4));       //使用圆括号，调用移动构造函数

        Widget w8{std::move(w4)};       //使用花括号，调用std::initializer_list构造
                                        //函数（与w6相同原因）



    }
} // namespace


// b、只有当没办法把括号初始化中实参的类型转化为std::initializer_list时，编译器才会回到正常的函数决议流程中。
// 例如

namespace {

class Widget { 
public:  
    Widget(int i, bool b);                              //同之前一样
    Widget(int i, double d);                            //同之前一样
    //现在std::initializer_list元素类型为std::string
    Widget(std::initializer_list<std::string> il);     //没有隐式转换函数
};

void test() {
    Widget w1(10, true);     // 使用圆括号初始化，调用第一个构造函数
    Widget w2{10, true};     // 使用花括号初始化，现在调用第一个构造函数
    Widget w3(10, 5.0);      // 使用圆括号初始化，调用第二个构造函数
    Widget w4{10, 5.0};      // 使用花括号初始化，现在调用第二个构造函数
}

} // namespace


// c、特殊情况 - 空的花括号初始化
// （1）假如你使用的花括号初始化是空集，
// （2）你欲构建的对象有默认构造函数
// （3）也有std::initializer_list构造函数
namespace {
    class Widget {
    public:
        Widget();                              //同之前一样
        Widget(std::initializer_list<std::string> il);     //同之前一样
    };

    void test() {
        Widget w1;        //调用默认构造函数
        Widget w2{};     //正确！调用默认构造函数
        Widget w3();     //错误！被解析为函数声明，声明了一个名为w1的函数
    }

} // namespace


// 五、vector设计的缺陷
// vector的构造函数设计不当，导致使用花括号初始化时容易出错
namespace {
    void test() {
        std::vector<int> v1(10, 20);    //创建一个有10个元素的vector，每个元素值为20

        std::vector<int> v2{ 10, 20 };  //创建一个有2个元素的vector，元素值分别为10和20

    }
} // namespace


// 六、总结

// 1、作为类库设计：考虑清楚后再加入std::initializer_list构造函数

// 2、作为类库使用者：了解类库的构造函数设计，必须认真的在花括号和圆括号之间选择一个来创建对象

// 3、作为模板的作者，花括号和圆括号创建对象就更麻烦了。通常不能知晓哪个会被使用。
// 举个例子，假如你想创建一个接受任意数量的参数来创建的对象。
// 使用可变参数模板（variadic template）可以非常简单的解决

namespace {
    template <typename T, typename... Args>
    T* doSomeWork(Args&&... args) {
        // return new T(std::forward<Args>(args)...); //使用圆括号初始化
    }

    // 初始化对象有两种方式：
    // T localObject(std::forward<Ts>(params)...);             //使用圆括号
    // T localObject{std::forward<Ts>(params)...};             //使用花括号

    // 那么问题：

    void test() {
        std::vector<int> v; 

        // 如果doSomeWork创建localObject时使用的是圆括号，std::vector就会包含10个元素。
        // 如果doSomeWork创建localObject时使用的是花括号，std::vector就会包含2个元素。
        // 哪个是正确的？doSomeWork的作者不知道，只有调用者知道。

        doSomeWork<std::vector<int>>(10, 20);

        // 标准库函数std::make_unique和std::make_shared（参见Item21）也面临该问题
    }
} // namespace