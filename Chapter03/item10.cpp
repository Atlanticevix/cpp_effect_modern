// 条款 10 - 优先考虑限域enum而非未限域enum

#include <cstdint>
#include <tuple>
#include <string>

// 一、98风格的enum的问题
// 通常来说，在花括号中声明一个名字会限制它的作用域在花括号之内。但这对于C++98风格的enum中声明的枚举名（enumerator）是不成立的
// 这种枚举有一个官方的术语：未限域枚举(unscoped enum)

namespace OldStyleEnum
{
    enum Color { red, green, blue }; // 未限域枚举

    void test()
    {
        Color c = red; // 没问题，red在OldStyleEnum命名空间中可见

        int n = red; // 也没问题，red隐式转换为int

        // 如果有另一个未限域枚举也有一个名为red的枚举值，就会引发命名冲突
        enum TrafficLight { red, yellow, green };

        TrafficLight t = red; // 错误！red二义性

        auto white = false; //错误! white早已在这个作用域中声明
    }
} // namespace


// 二、限域enum(scoped enum)的优点

// C++11引入了限域枚举(enum class)，它们除了解决了未限域枚举的问题（优点1），还有强类型的优点（优点2）
namespace NewStyleEnum
{
    enum class Color { red, green, blue }; // 限域枚举

    void test()
    {
        Color c = Color::red; // 没问题，red在Color作用域中可见

        // int n = Color::red; // 错误！Color::red不会隐式转换为int

        enum class TrafficLight { red, yellow, green };

        TrafficLight t = TrafficLight::red; // 没问题，没有命名冲突

        auto white = false; //没问题! white没有冲突
    }
} // namespace

// 可强制转换，但需要显式转换
namespace NewStyleEnum2
{
    enum class Color { red, green, blue }; // 限域枚举

    void test()
    {
        Color c = Color::red; // 没问题，red在Color作用域中可见

        int n = static_cast<int>(Color::red); // 显式转换，OK
    }
} // namespace



// 优点3： 比起非限域enum而言，限域enum可以被前置声明。（未完全正确）
// 也就是说，它们可以不指定枚举名直接声明

namespace NewStyleEnum3
{
    enum Color;         //错误！

    enum class Color : char; // 前置声明

    void test()
    {
        Color c = Color::red; // 没问题，red在Color作用域中可见
    }

    enum class Color : char { red, green, blue }; // 定义
} // namespace


// ** 非限域enum不能前置是因为编译器不知道其底层类型（underlying type），
// ** 如果指定了底层类型就可以被前置声明

// ** 拓展知识：
// 为了高效使用内存，编译器通常在确保能包含所有枚举值的前提下为enum选择一个最小的底层类型
namespace OldStyleEnum2
{
    // 编译器可能选择char作为底层类型，因为这里只需要表示三个值
    enum Color { black, white, red};

    // 但对于有些enum枚举值很大的枚举，除了在不寻常的机器上（比如一个char至少有32bits的那种），
    // 编译器都会选择一个比char大的整型类型来表示Status
    enum Status { good = 0,
        failed = 1,
        incomplete = 100,
        corrupt = 200,
        indeterminate = 0xFFFFFFFF
    };

} // namespace

// * 回到主线：
// * 所以enum 不能前置声明的最重要原因就是编译器不知道选择多大的底层类型
// * 而限域enum可以前置声明是因为它们允许程序员指定底层类型，如：
namespace OldStyleEnum3
{
    enum Color : char; // 前置声明

    void test()
    {
        Color c = red; // 没问题，red在Color作用域中可见
    }

    enum Color : char { red, green, blue }; // 定义
} // namespace


// 前置声明enum的好处在于可以减少头文件间的依赖关系，从而减少编译时间
// 编译器在使用它之前需要知晓该enum的大小情况下，就可以前置声明它

// 限域enum的底层类型总是已知的（默认int），而对于非限域enum，可以强制指定

namespace OldStyleEnum4
{
    // 声明
    enum class Status1;                  // 底层类型是int
    enum Status2: std::uint32_t;   // Status的底层类型是std::uint32_t（需要包含 <cstdint>）

    void test()
    {
        Status1 s1 = Status1::good;
        Status2 s2 = Status2::good;
    }

    enum class Status1: std::uint32_t {
        good = 0,
        failed = 1,
        incomplete = 100,
        corrupt = 200,
        audited = 500,
        indeterminate = 0xFFFFFFFF
    };

    enum Status2: std::uint32_t {
        good = 0,
        failed = 1,
        incomplete = 100,
        corrupt = 200
        ,
        audited = 500,
        indeterminate = 0xFFFFFFFF
    };

} // namespace


// 三、限域enum的缺点。

// 当出现类似如下情况时，限域enum的使用会比较繁琐
namespace NewStyleEnum4
{
    using UserInfo =                //类型别名，参见Item9
    std::tuple<std::string,     //名字
               std::string,     //email地址
               std::size_t> ;   //声望

    enum UserInfoFields { uiName, uiEmail, uiReputation };

    enum class NewUserInfoFields { uiName, uiEmail, uiReputation };

    void test() 
    {
        UserInfo uInfo;                 //tuple对象
        auto val1 = std::get<1>(uInfo);	//获取第一个字段，工程直接用数字难以维护

        auto val2 = std::get<uiName>(uInfo); // 枚举自动转为std::size_t，没问题。但枚举已经扩散到全局命名空间，可能引发命名冲突

        auto val3 = std::get<static_cast<std::size_t>(NewUserInfoFields::uiName)>(uInfo); // 使用限域enum，没问题，但写起来很繁琐
    }
} // namespace


// 为了避免这么繁琐，可以定义一个辅助函数模板
// 1、std::get 的值需要在编译期已知，所以是该函数必须是一个constexpr函数
// 2、另外可以通过std::underlying_type这个type trait获得枚举的底层类型（underlying type）
//    这样就不需要每次都写static_cast<std::size_t>了
namespace NewStyleEnum4 {
    // C++ 11 版本
    template<typename EnumType>
    constexpr typename std::underlying_type<EnumType>::type
    toUType11(EnumType e) noexcept {
        return static_cast<typename std::underlying_type<EnumType>::type>(e);
    }

    // C++ 14 版本，使用了别名声明简化类型
    template<typename EnumType>
    constexpr std::underlying_type_t<EnumType>
    toUType14(EnumType e) noexcept {
        return static_cast<std::underlying_type_t<EnumType>>(e);
    }

    // 或者更为简洁，使用auto
    template<typename EnumType>
    constexpr auto toUType(EnumType e) noexcept {
        return static_cast<std::underlying_type_t<EnumType>>(e);
    }

    // 那么上述问题就可以这样解决
    void test2()
    {
        UserInfo uInfo;                 //tuple对象

        // 虽然仍然有点冗长，但比起上面的方法已经简洁多了
        // 同时也解决未限域enum的命名冲突问题，值得使用
        auto val = std::get<toUType(NewUserInfoFields::uiName)>(uInfo); // 使用限域enum，简洁多了
    }
}

// 四、总结
// * C++98的enum即非限域enum。
// * 限域enum的枚举名仅在enum内可见。要转换为其它类型只能使用cast。
// * 非限域/限域enum都支持底层类型说明语法，限域enum底层类型默认是int。非限域enum没有默认底层类型。
// * 限域enum总是可以前置声明。非限域enum仅当指定它们的底层类型时才能前置。