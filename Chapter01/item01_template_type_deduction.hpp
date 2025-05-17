/* template_type_deduction */

#include <iostream>
#include <array>

/*
template<typename T>
void f(ParamType param); // 编译期间，编译器会推导T和ParamType的类型
f(expr); // 调用形式

如：
template<typename T>
void f(const int& param);

int x = 0;
f(x); // T = int, ParamType = const int&

* 结论：T的推导结果依赖于expr的类型，同时也依赖于ParamType的类型

* 对于ParamType，分三种情况：
* 1. ParamType是一个引用、指针类型（如int&、const int&、int*、const int*等），但不是万有引用类型（&&）
* 2. ParamType是一个万有引用类型（如int&&、const int&&等）
* 3. ParamType即不是引用，也不是指针

注意：
    * 在函数模板中持有类型形参T时，T&&是一个万有引用类型（Universal Reference），不是右值引用类型（Rvalue Reference）

*/

/*
情况1：
    * ParamType是一个引用、指针类型（如int&、const int&、int*、const int*等），但不是万有引用类型（&&）
推导：
    * 若expr为引用或指针 先将 & 或 * 去掉；
    * 在对expr和ParamType进行模式匹配，决定T的类型
*/

namespace TempalteTypeDedution1
{
    template<typename T>
    void f(T& param)
    {
    }

    void test()
    {
        int x = 0;
        const int cx = x;
        const int& rx = x;

        f(x); // T = int, ParamType = int&
        f(cx); // T = const int, ParamType = const int&
        f(rx); // T = const int, ParamType = const int&
    }
}

namespace TempalteTypeDedution2
{
    template<typename T>
    void f(T* param)
    {
    }

    void test()
    {
        int x = 0;
        const int cx = x;
        int* px = &x;
        const int* pcx = &cx;

        f(px); // T = int, ParamType = const int*
        f(pcx); // T = const int, ParamType = const int*
    }
}

namespace TempalteTypeDedution3
{
    template<typename T>
    void f(const T& param)
    {
    }

    void testUniversalRef()
    {
        int x = 0;
        const int cx = x;
        const int& rx = x;

        f(x); // T = int, ParamType = const int&
        f(cx); // T = int, ParamType = const int&
        f(rx); // T = int, ParamType = const int&
    }
}

/*
情况2：
    * ParamType是一个万有引用类型（如int&&、const int&&等）
推导：
    * 若expr为左值，T和ParamType都推导为左值引用类型（&）。
        这是模板推导的一个奇特的点：T是被推导为引用类型的唯一情况；尽管声明时使用右值引用语法（&&），但推导时会被转换为左值引用类型（&）。
    * 若expr为右值，则使用情况1的规则。

注意：
    * 当遇到万有引用时，形参推导会根据实参的类型时左值还是右值来决定T的类型。
*/
namespace TempalteTypeDedution4
{
    template<typename T>
    void f(T&& param)
    {
    }

    void test()
    {
        int x = 0;
        const int cx = x;
        const int& rx = x;

        f(x); // T = int&, ParamType = int&
        f(cx); // T = const int&, ParamType = const int&
        f(rx); // T = const int&, ParamType = const int&

        f(27); // T = int, ParamType = int&&(27是右值，所以应用情况1，去掉&&后T为int，ParamType为int&&)
    }
}


/*
情况3：
    * ParamType即不是引用，也不是指针，那么就是按值传递，即传入都是一个值的副本（新对象）
推导：
    * 若expr为引用类型，先忽略引用部分
    * 忽略引用部分后，如果T为const对象或volatile对象，则也丢弃其const或volatile部分（值传递）
*/
namespace TempalteTypeDedution5
{
    template<typename T>
    void f(T param)
    {
    }

    void test()
    {
        int x = 0;
        const int cx = x;
        const int& rx = x;

        volatile int vx = 0;

        f(x); // T = int, ParamType = int
        f(cx); // T = int, ParamType = int
        f(rx); // T = int, ParamType = int

        f(vx); // T = int, ParamType = int


        // 问题：
        // const char* const p => 指向常量的常量指针，常量指针，XXX* 
        const char* const p = "hello world";
        f(p); // T = ?, ParamType = ? 
    }
}

/*
情况4 - 数组实参
    数组的特殊性：
    const char name[] = "J. P. Briggs";     //name的类型是const char[13]
    const char * ptrToName = name;          //数组退化为指针
    在这里const char*指针ptrToName会由name初始化，而name的类型为const char[13]，这两种类型（const char*和const char[13]）是不一样的，
    但是由于数组退化为指针的规则，编译器允许这样的代码。

    但这里有两种特殊情况：值传递和引用传递

*/
namespace TempalteTypeDedution6
{
    template<typename T>
    void f(T param)  // 按照值传递
    {
    }

    void test()
    {
        const char name[] = "J. P. Briggs";     //name的类型是const char[13]
        const char* ptrToName = name;          //数组退化为指针

        f(name); // T = const char*, ParamType = const char*
        f(ptrToName); // T = const char*, ParamType = const char*

        void f(int param[]); // T = int*, ParamType = int*
        void f(int* param);  // T = int*, ParamType = int*
    }
}

namespace TempalteTypeDedution7
{
    template<typename T>
    void f(T& param)  // 按照引用传递
    {
    }

    void test()
    {
        const char name[] = "J. P. Briggs";     //name的类型是const char[13]
        const char* ptrToName = name;          //数组退化为指针

        f(name); // T = const char[13], ParamType = const char[13]
        f(ptrToName); // T = const char*, ParamType = const char*

        void f(int param[]); // T = int*, ParamType = int*
        void f(int* param);  // T = int*, ParamType = int*
    }











    // 特殊用法：声明指向数组的引用的能力，使得我们可以创建一个模板函数来推导出数组的大小
    template<typename T, std::size_t N>
    constexpr  std::size_t arraySize(T (&param)[N])
    {
        return N;  // 返回数组的大小
    }

    void test2()
    {
        int keyVals[] = { 1, 3, 7, 9, 11, 22, 35 };  
        
        int mappedVals[arraySize(keyVals)];

        std::array<int, arraySize(keyVals)> mappedVals2;   
    }

}


namespace TempalteTypeDedution8
{
    template<typename T>
    void f(T&& param)  // 按照传递
    {
    }

    void test()
    {
        const char name[] = "J. P. Briggs";     //name的类型是const char[13]
        const char* ptrToName = name;          //数组退化为指针

        f(name); // T = const char[13], ParamType = const char[13]
        f(ptrToName); // T = const char*, ParamType = const char*

        void f(int param[]); // T = int*, ParamType = int*
        void f(int* param);  // T = int*, ParamType = int*
    }


    template<typename T, std::size_t N>
    constexpr  std::size_t arraySize(T (&&param)[N])
    {
        return N;  // 返回数组的大小
    }


    void test2()
    {
        std::array<int, arraySize({ 1, 3, 7, 9, 11, 22, 35 })> mappedVals2;   
    }

}



/*
情况5 - 函数实参
    函数的特殊性：同数组一样，函数也会退化为指针，推导规则也是一样的。
*/
namespace TempalteTypeDedution9
{
    template<typename T>
    void f(T param)  // 按照值传递
    {
    }

    void someFunc(int, double); 

    void test()
    {
        int (*funcPtr)(int) = nullptr; // 函数指针

        f(funcPtr); // T = int(*)(int), ParamType = int(*)(int)
        f(someFunc); // T = int(*)(int, double), ParamType = int(*)(int, double)
    }
}

namespace TempalteTypeDedution10
{
    template<typename T>
    void f(T& param)  // 按照引用传递
    {
    }

    void someFunc(int, double); 

    void test()
    {
        int (*funcPtr)(int) = nullptr; // 函数指针

        f(funcPtr); // T = int(*)(int), ParamType = int(*&)(int)
        f(someFunc); // T = int(int, double), ParamType = int(&)(int, double)
    }
}