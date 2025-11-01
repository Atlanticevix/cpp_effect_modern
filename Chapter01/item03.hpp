/* 条款3 - 理解decltype */

#include <deque>
#include <vector>

/*
decltype 相比模板类型推导和auto类型推导（参见Item1和Item2），只是简单的返回名字或者表达式的类型 

const int i = 0;                //decltype(i)是const int

bool f(const Widget& w);        //decltype(w)是const Widget&
                                //decltype(f)是bool(const Widget&)

struct Point{
    int x,y;                    //decltype(Point::x)是int
};                              //decltype(Point::y)是int

Widget w;                       //decltype(w)是Widget

if (f(w)){}                      //decltype(f(w))是bool

template<typename T>            //std::vector的简化版本
class vector{
public:
    T& operator[](std::size_t index); vector<int> v; 
};
vector<int> v;                  //decltype(v)是vector<int>

if (v[0] == 0) {}                //decltype(v[0])是int&


*/

/*
C++ 11情况：
    * 在C++11中，decltype最主要的用途就是用于声明函数模板，而这个函数返回类型依赖于形参类型。
 */

namespace DecltypeDeductions1
{
    // C++11 case
    /*
        * 这里使用了C++11的尾置返回类型语， 即在函数形参列表后面使用一个”->“符号指出函数的返回类型
        * 函数名称前面的auto不会做任何的类型推导工作，尾置返回类型的好处是我们可以在函数返回类型中使用函数形参相关的信息
        * 在authAndAccess函数中，我们使用c和i指定返回类型。
        * 如果我们按照传统语法把函数返回类型放在函数名称之前，c和i就未被声明所以不能使用
    */
    template<typename Container, typename Index>
    auto getValue(Container& c, Index i) -> decltype(c[i])
    {
        return c[i]; // 返回容器c中索引为i的元素
    }
}

/*
C++14情况：
    * C++11允许自动推导单一语句的lambda表达式的返回类型， C++14扩展到允许自动推导所有的lambda表达式和函数，甚至它们内含多条语句。
    * 这意味着在C++14标准下我们可以忽略尾置返回类型，只留下一个auto。
    * 使用这种声明形式，auto标示这里会发生类型推导（编译器将会从函数实现中推导出函数的返回类型）。
*/

namespace DecltypeDeductions2
{
    // C++11 case
    /*
        * 这里使用了C++11的尾置返回类型语， 即在函数形参列表后面使用一个”->“符号指出函数的返回类型
        * 函数名称前面的auto不会做任何的类型推导工作，尾置返回类型的好处是我们可以在函数返回类型中使用函数形参相关的信息
        * 在authAndAccess函数中，我们使用c和i指定返回类型。
        * 如果我们按照传统语法把函数返回类型放在函数名称之前，c和i就未被声明所以不能使用
    */
    // template<typename Container, typename Index>
    // auto getValue(Container& c, Index i) -> decltype(c[i])
    // {
    //     return c[i]; // 返回容器c中索引为i的元素
    // }

    // C++14 case
    // C++14 优化了返回值尾序语法，不用再写
    template<typename Container, typename Index>
    auto getValue(Container& c, Index i)
    {
        return c[i]; // 返回容器c中索引为i的元素
    }

    // 问题：
    // getValue返回的是c[i]的类型（即c的operator[]返回值类型，即元素引用），
    // 但如果c是一个容器类，operator[]返回的是一个int&类型的引用
    // 那理论上getValue(d, 2) = 10是合法的，是否可以直接赋值？
    void question()
    {
        std::deque<int> d = { 1, 2, 3, 4, 5 };

        // 以下调用是否合法？
        // getValue(d, 2) = 10;
    }
    /*
        答案：
        1. auto推导中知道，auto作为返回值实际使用模板推导规则，那么容器的operator[]对于大多数T类型的容器会返回一个T&
        2. 模板推导规则中，表达式的引用性（reference-ness）会被忽略掉。
        
        那么getValue(d, 2)的返回值类型是int（右值），而不是int&
    */
}



/*
decltype推导的妙用（解决上述问题）：
    * 1.利用decltype类型推导来推导它的返回值，即返回一个和c[i]表达式类型一样的类型（T&)
    * C++14通过使用decltype(auto)说明符, 使类型被暗示时需要使用decltype类型推导的规则
    * 
    * 2.对初始化表达式使用推导规则，也是可以达与返回值一样的效果
    * 
*/

namespace DecltypeDeductions3
{
    // decltype(auto) 解决了上述问题（C++14）
    //
    template<typename Container, typename Index>
    decltype(auto) getValue(Container& c, Index i) 
    {
        return c[i]; // 返回容器c中索引为i的元素
    }

    void solution()
    {
        std::deque<int> d = { 1, 2, 3, 4, 5 };

        // 通过decltype(auto)，返回值类型是int&，所以可以直接赋值
        getValue(d, 2) = 10;
    }


    struct Widget {};

    void anotherUse()
    {
        Widget w;

        const Widget& rw = w; // rw是一个const Widget&类型的引用

        auto widget1 = rw; // auto推导，widget是一个Widget类型的对象

        decltype(auto) widget2 = rw; // widget2是一个const Widget&类型的引用
    }

}


/*
扩展情况：
    对于getValue函数的传递参数是 非常量左值引用（lvalue-reference-to-non-const）
    但如果传递是一个 右值医用容器，是非法的。除非把形参改为常量左值引用（lvalue-reference-to-const）
*/
namespace DecltypeDeductions4
{

    template<typename Container, typename Index>
    decltype(auto) getValue(Container& c, Index i) 
    {
        return c[i]; // 返回容器c中索引为i的元素
    }

    void question()
    {
        std::deque<int> d { 1, 2, 3, 4, 5 };

        // 运行时错误：
        // 由于传的是临时值，临时在调用前就被销毁了
        getValue((std::deque<int>{ 1, 2, 3, 4, 5 }), 2) = 10;
    }

}


/*
扩展情况（解决方案）：
    对于getValue函数的传递参数是 非常量左值引用（lvalue-reference-to-non-const）
    但如果传递是一个 右值医用容器，是非法的。除非把形参改为常量左值引用（lvalue-reference-to-const）
*/
namespace DecltypeDeductions5
{

    // 万有引用（&&）可以解决左值引用和右值引用问题
    // 但万有引用需要加上std::forward（条款25）
    // C++14版本
    template<typename Container, typename Index>
    decltype(auto) getValue(Container&& c, Index i) 
    {
        return std::forward<Container>(c)[i]; // 返回容器c中索引为i的元素
    }

    // C++11版本
    // template<typename Container, typename Index>
    // auto getValue(Container&& c, Index i) 
    // -> decltype(std::forward<Container>(c)[i])
    // {
    //     return std::forward<Container>(c)[i]; // 返回容器c中索引为i的元素
    // }

    void solution()
    {
        std::deque<int> d { 1, 2, 3, 4, 5 };
        getValue(d, 2) = 10;

        getValue((std::deque<int>{ 1, 2, 3, 4, 5 }), 2) = 10;
    }

}

/*
delctype的特殊用法：
    对于单纯的变量名，decltype只会返回变量的声明类型
    然而，对于比单纯的变量名更复杂的左值表达式，decltype可以确保报告的类型始终是左值引用
*/

namespace DecltypeDeductions5
{
    decltype(auto) common()
    {
        int x = 0;
        return x;
    }


    decltype(auto) special()
    {
        int x = 0;
        // C++认为()中是一个复杂的表达式
        return (x); // 返回int&类型，导致未定义行为
    }

    void test()
    {

        decltype(auto) y = common(); // y是int类型
        decltype(auto) z = special(); // z是int&类型 运行时出错
    }
}

// 总结
// * decltype总是不加修改的产生变量或者表达式的类型。
// * 对于T类型的不是单纯的变量名的左值表达式，decltype总是产出T的引用即T&。
// * C++14支持decltype(auto)，就像auto一样，推导出类型，但是它使用decltype的规则进行推导。