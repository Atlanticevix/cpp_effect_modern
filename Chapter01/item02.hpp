/* 条款2 - 理解auto型别推导 */

#include <initializer_list>
#include <vector>

/*

* auto的推导和Template推导导致类型几乎一致。但有一例外。

auto类型推导和模板类型推导有一个直接的映射关系。
编译器推导机制上，它们之间可以通过一个非常规范、非常系统化的转换流程来转换彼此。

* 先看模板推导：

template<typename T>
void f(ParmaType param);

f(expr); //调用

在f的调用中，编译器使用expr推导T和ParamType的类型。


* 再看auto推导情况：
当一个变量使用auto进行声明时，auto扮演了模板中T的角色，变量的类型说明符扮演了ParamType的角色

auto x = 27; // 类型说明符是auto自己
const auto cx = x; // 类型说明符是const auto
const auto& rx = x; // 类型说明符是const auto&

这里要推导x，cx和rx的类型，推导过程编译器行为就如：

/// auto x = 27;

template<typename T>            //概念化的模板用来推导x的类型
void func_for_x(T param);

func_for_x(27);                 //概念化调用：param的推导类型是x的类型, auto x = 27

/// const auto cx = x;

template<typename T>            //概念化的模板用来推导cx的类型
void func_for_cx(const T param);

func_for_cx(x);                 //概念化调用：param的推导类型是cx的类型, 


/// const auto& rx = x

template<typename T>            //概念化的模板用来推导rx的类型
void func_for_rx(const T & param);

func_for_rx(x);                 //概念化调用：param的推导类型是rx的类型


这里只是一个概念化的推导过程，实际上编译器并不会生成这些模板函数。
只是想说明：auto的推导规则与Template几乎一致

* 所以也分三种情况讨论：
* 1. 类型说明符是一个指针或引用，但不是通用引用
* 2. 类型说明符一个通用引用
* 3. 类型说明符既不是指针也不是引用

*/

namespace AutoTypeDedution1
{

    void someFunc(int, double); //someFunc是一个函数，类型为void(int, double)

    void test()
    {
        // 情况1 和 情况3
        auto x = 18;  // 情况3，既不是指针也不是引用
        const auto cx = x; // 情况3
        const auto& rx = x; // 情况1 ，一个指针或引用，但不是通用引用

        // 情况2
        auto && uref1 = x; // x是左值，推导为int&
        auto && uref2 = cx; // cx是左值，推导为int&
        auto && uref3 = rx; // rx是左值，推导为int&

        auto && uref4 = 27; // 27是右值，推导为int&&


        // 特殊情况 - 数组，同样适用auto的推导
        const char name [] = "J. P. Briggs";     //name的类型是const char[13]

        auto arr1 = name; // 值传递， arr1的类型是const char*，数组退化为指针
        auto & arr2 = name; // 引用传递， arr2的类型是const char (&)[13]

        auto func1 = someFunc; // func1的类型是void(*)(int, double)，函数退化为指针
        auto & func2 = someFunc; // func2的类型是void(&)(int, double)，函数退化为引用
    }

}

/*
* 例外情况
    对于花括号的处理（uniform initialization）是auto类型推导和模板类型推导唯一不同的地方。
    当使用auto声明的变量使用花括号的语法进行初始化的时候，会推导出std::initializer_list<T>的实例化，
    但是对于模板类型推导行不通（确切的说是不知道怎么办）。
*/

namespace AutoTypeDedution2
{
    void test()
    {
        {
            // C++98 初始化
            int x1 = 18;
            int x2 (18);

            // C++11 初始化, 持统一初始化（uniform initialization）的语法
            int x3 = { 18 }; // C++11 语法，x3的类型是int，值是18
            int x4{ 18 }; // x4的类型是int，值是18
        }

        // auto的便利性，把int类型声明变成了auto
        {

            auto x1 = 18; // x1的类型是int，同上
            auto x2 (18); // 同上


            /*
                因为x使用花括号的方式进行初始化，x必须被推导为std::initializer_list。
                但是std::initializer_list是一个模板。std::initializer_list<T>会被某种类型T实例化，所以这意味着T也会被推导。 
                推导落入了这里发生的第二种类型推导——模板类型推导的范围，调入推导死循环。
            */
            auto x3 = { 18 }; // x3的类型是std::initializer_list<int>，
            auto x4 { 18 }; // 同上

            // /*
            //     花括号中的值并不是同一种类型
            //     无法推导std::initializer_list<T>中的T
            // */
            auto x5 = { 1, 2, 3.0 }; //错误！
        }
    }

}


/*
    按照最开始auto和模板的推导规则，那么
        auto x = { 11, 23, 9 };
    相当于
        template<typename T>            //带有与x的声明等价的
        void f(T param);                //形参声明的模板
    但是
        f({ 11, 23, 9 });               //错误！不能推导出T
 */
namespace AutoTypeDedution3
{
    // 但如果在模板中指定T是std::initializer_list<T>而留下未知T,
    // 模板类型推导就能正常工作
    template<typename T>
    void f(std::initializer_list<T> param)
    {
    }

    void test()
    {
        f({ 1, 2, 3 }); 
    }
}

/*
* C++14的auto情况
    C++14允许auto用于函数返回值并会被推导（参见Item3），且C++14的lambda函数也允许在形参声明中使用auto
    * 是但在这些情况下auto实际上使用模板类型推导的那一套规则在工作，而不是auto类型推导
*/

namespace AutoTypeDedution4
{
    auto testReturnAuto()
    {
        return { 1, 2, 3 }; // 错误，不能推导{ 1, 2, 3 }的类型
    }

    void TestLambdaAuto()
    {
        std::vector<int> v;

        auto reset = [&v](auto param) { return v = param; };

        reset({1, 2, 3});
    }
}

// 总结
// * auto类型推导通常和模板类型推导相同，但是auto类型推导假定花括号初始化代表std::initializer_list，而模板类型推导不这样做
// * 在C++14中auto允许出现在函数返回值或者lambda函数形参中，但是它的工作机制是模板类型推导那一套方案，而不是auto类型推导