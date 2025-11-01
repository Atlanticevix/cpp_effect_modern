// 条款09 - 优先考虑别名声明而非typedef

#include <memory>
#include <unordered_map>

// 一、别名在可读性上更友好

namespace Item09_01{ {
    typedef std::unique_ptr<std::unordered_map<std::string, std::string>> UPtrMapSS_1; // OK
    // 等价于
    using UPtrMapSS_2 = std::unique_ptr<std::unordered_map<std::string, std::string>>; // 更好，从左到右的阅读顺序更自然

    //FP是一个指向函数的指针的同义词，它指向的函数带有
    //int和const std::string&形参，不返回任何东西
    typedef void (*FP_1)(int, const std::string&);    //typedef

    //含义同上，typedef的写法会让新人困惑，而且不易阅读。
    using FP_2 = void (*)(int, const std::string&);   //别名声明

}

// 二、模板代码中使用别名声明更简洁


// a. C++ 98 声明一个模板别名（alias template）是很麻烦的，必须使用嵌套的typedef
// 例如用户想要一个使用自定义分配器MyAlloc<T>的std::list<T>, std::list<T, MyAlloc<T>>
// 为了造一个别名，需要这样写：
namespace Item09_02{

    class Widget
    {
    };

    //MyAllocList<T>是std::list<T, MyAlloc<T>>的同义词  
    template<typename T>                            
    struct MyAllocList {                            
        typedef std::list<T, MyAlloc<T>> type;
    };

    MyAllocList<Widget>::type lw; //用户代码



} // namespace

// 使用别名声明，代码更简洁
namespace Item09_03{

    class Widget
    {
    };

    //MyAllocList<T>是std::list<T, MyAlloc<T>>的同义词  
    template<typename T>
    using MyAllocList = std::list<T, MyAlloc<T>>;

    MyAllocList<Widget> lw; //用户代码, 没有冗长的::type

} // namespace


// b.简化【模板依赖类型】指定（typename）
// 当模板嵌套别的模板时，嵌套模板的类型可能依赖于外层模板的参数类型，就需要指定typename，如
namespace Item09_02{

    //MenuWidget<T>含有一个MyAllocLIst<T>对象作为数据成员
    template<typename T>
    class MenuWidget {
    private: 

        // 编译器在Widget的模板中看到MyAllocList<T>::type（使用typedef的版本），
        // 它不能确定那是一个类型的名称。因为可能存在一个MyAllocList的它们没见到的特化版本，
        // 该版本的MyAllocList<T>::type指代了一种不是类型的东西。需用要typename修饰符来指明这是一种类型

        typename MyAllocList<T>::type list;     // MyAllocList的T需要依赖外部T，需要typename关键字

    }; 


} // namespace


// 使用别名声明，省去了typename
namespace Item09_03
{
    template <typename T>
    class Widget
    {
    private:

        //当编译器处理Widget模板时遇到MyAllocList<T>（使用模板别名声明的版本），它们
        //知道MyAllocList<T>是一个类型名，因为MyAllocList是一个别名模板：它一定是一个类型名。
        // 因此MyAllocList<T>就是一个非依赖类型（non-dependent type），就不需要也不允许使用typename修饰符

        MyAllocList<T> list; // 没有“typename”, 没有“::type”
    };
}

// c. 简化模板元编程type_traits的使用

// 在C++11发布时，<type_traits>中存在很多类似这种类型处理的模板

// 由于历史原因（因为标准委员会没有及时认识到别名声明是更好的选择），所以C++11的type traits是
// 通过在struct内嵌套typedef来实现的。例如：

/*
std::remove_const<T>::type          //从const T中产出T
std::remove_reference<T>::type      //从T&和T&&中产出T
std::add_lvalue_reference<T>::type  //从T中产出T&
*/

// 所以在自己编写模板的时候，需要使用typename和::type，例如：
namespace Item09_04
{
    template <typename T>
    struct AddConstLvalueRef
    {
        using type = typename std::add_lvalue_reference<typename std::remove_const<T>::type>::type;
    };

    // 用户代码
    AddConstLvalueRef<int>::type x = 0; // x的类型是const int&
} // namespace


// 所以后台C++14引入了别名模板版本的type traits，例如：

/*
std::remove_const<T>::type          //C++11: const T → T 
std::remove_const_t<T>              //C++14 等价形式

std::remove_reference<T>::type      //C++11: T&/T&& → T 
std::remove_reference_t<T>          //C++14 等价形式

std::add_lvalue_reference<T>::type  //C++11: T → T& 
std::add_lvalue_reference_t<T>      //C++14 等价形式
*/

// 使用别名模板版本的type traits，代码更简洁
namespace Item09_05
{
    template <typename T>
    using AddConstLvalueRef = std::add_lvalue_reference_t<std::remove_const_t<T>>;

    // 用户代码
    AddConstLvalueRef<int> x = 0; // x的类型是const int&
} // namespace


// 三、总结

// * typedef不支持模板化，但是别名声明支持。
// * 别名模板避免了使用“::type”后缀，而且在模板中使用typedef还需要在前面加上typename
// * C++14提供了C++11所有type traits转换的别名声明版本
