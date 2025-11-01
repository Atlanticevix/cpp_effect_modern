/* 条款4 - 掌握查看类别推导结果的方法 */

#include <iostream>
#include <typeinfo>
#include <vector>
/*
    掌握查看类型推导结果的方法
*/


// 编译器
// 定义一个未实现的模板类，在编译期时反馈错误查看类型
namespace item04_1
{

template<typename T>
class TD;

void test()
{
    const int theAnaswer = 42;

    auto x = theAnaswer;
    auto y = &theAnaswer;

    // TD<decltype(x)> xType; // T = int
    // TD<decltype(y)> yType; // T = const int*
}

}


// 运行时
// typeid可以返回类型信息，但每家编译器实现不一样，且不准确
namespace item04_2
{

template<typename T>
void f(const T& param)
{
    std::cout << "T = " << typeid(T).name() << std::endl;
    std::cout << "ParamType = " << typeid(param).name() << std::endl;
}

class Widget
{
};

void test()
{
    std::vector<Widget> createVec(2);

    f(&createVec[0]);
}

}


int main()
{

    item04_2::test();

    return 0;
}

// 总结
// * 类型推断可以从IDE看出，从编译器报错看出，从Boost TypeIndex库的使用看出
// * 这些工具可能既不准确也无帮助，所以理解C++类型推导规则才是最重要的