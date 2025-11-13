// Item15 - 尽可能的使用constexpr

#include <array>

// 一、 constexpr的概述
// constexpr修饰变量，表示该变量的值在编译期就能确定下来。
// constexpr修饰函数，表示该函数在编译期就能求出结果，但不要求必须在编译期求出结果。

// 二、 constexpr修饰变量

// 先从constexpr对象开始说起。这些对象，实际上，和const一样，它们是编译期可知的。
// 技术上来讲，它们的值在翻译期（translation）决议，所谓翻译不仅仅
// 包含是编译（compilation）也包含链接（linking）

// 编译期可知的值“享有特权”，它们可能被存放到只读存储空间中。这对于嵌入式很友好。
// 例如：需要“整型常量表达式（integral constant expression）的上下文 - 数组大小，
// 整数模板参数（包括std::array对象的长度），枚举名的值，对齐修饰符（alignas(val)）
namespace Item15_01
{
    int sz;                             //non-constexpr变量

    constexpr auto arraySize1 = sz;     //错误！sz的值在编译期不可知
    std::array<int, sz> data1;          //错误！一样的问题

    constexpr auto arraySize2 = 10;     //没问题，10是编译期可知常量
    std::array<int, arraySize2> data2;  //没问题, arraySize2是constexpr

} // namespace name


// const 和 constexpr变量的区别在于，const变量的值不一定在编译期可知，

namespace Item15_02
{
    int sz;                            //和之前一样

    const auto arraySize = sz;         //没问题，arraySize是sz的const复制
    std::array<int, arraySize> data;   //错误，arraySize值在编译期不可知


} // namespace name


// 三、 constexpr修饰函数

// * 如果实参是编译期常量，这些函数将产出编译期常量:

// constexpr函数可以用于需求编译期常量的上下文。在需求编译期常量的上下文中，
// 如果你传给constexpr函数的实参在编译期可知，那么结果将在编译期计算；
// 如果实参的值在编译期不知道，你的代码就会被拒绝。

// * 如果实参是运行时才能知道的值，它们就将产出运行时值:
// 当一个constexpr函数被一个或者多个编译期不可知值调用时，它就像普通函数一样，运行时计算它的结果。


// * 这样设计的好处就是：不需要两个函数，一个用于编译期计算，一个用于运行时计算。constexpr全做了。

// (1) 这里设计一个constexpr int的power函数（std::pow是浮点型，且不是constexpr）:
namespace Item15_03 {

    //pow是绝不抛异常的constexpr函数
    constexpr                                   
    int pow(int base, int exp) noexcept 
    {

        // C++11中，constexpr函数的代码不超过一行语句：一个return。
        // 这里使用了 递归 替代 循环 方式来实现
        // return (exp == 0 ? 1 : base * pow(base, exp - 1)); // C++ 11风格

        // 在C++14中，constexpr函数的限制变得非常宽松了
        auto result = 1;
        for (int i = 0; i < exp; ++i) result *= base;
        
        return result;
    }

    //结果有3^numConds个元素
    constexpr auto numConds = 5;  

    std::array<int, pow(3, numConds)> results;  
}

// (2) 类也可以是constexpr:
namespace Item15_04 {

class Point {
    public:
        constexpr Point(double xVal = 0, double yVal = 0) noexcept
        : x(xVal), y(yVal)
        {}

        constexpr double xValue() const noexcept { return x; } 
        constexpr double yValue() const noexcept { return y; }

        // constexpr函数限制条件是【只能获取和返回字面值类型】，这基本上意味着有了值的类型能在编译期决定。
        // 在C++11中，除了void外的所有内置类型，以及一些用户定义类型都可以是字面值类型

        // 在C++11中，一下setter函数无法为constexpr，原因有二:
        // 第一，C++11中，constexpr成员函数是隐式的const（它们修改了成员）， 
        // 第二，它们有void返回类型，void类型不是C++11中的字面值类型(上述）。

        // void setX(double newX) noexcept { x = newX; } // C++11
        // void setY(double newY) noexcept { y = newY; } // C++11

        // C++14中，constexpr函数的限制放开：
        constexpr void setX(double newX) noexcept { x = newX; } //C++14
        constexpr void setY(double newY) noexcept { y = newY; } //C++14

    private:
        double x, y;
};

// Point的构造函数可被声明为constexpr，因为如果传入的参数在编译期可知，
// Point的数据成员也能在编译器可知
constexpr Point p1(9.4, 27.7);  //没问题，constexpr构造函数会在编译期“运行”
constexpr Point p2(28.8, 5.3);  //也没问题


// xValue和yValue的getter（取值器）函数也能是constexpr，
//因为如果对一个编译期已知的Point对象（如 p1）调用getter，
// 数据成员x和y的值也能在编译期知道。
constexpr
Point midpoint(const Point& p1, const Point& p2) noexcept
{
    return { (p1.xValue() + p2.xValue()) / 2,   //调用constexpr
             (p1.yValue() + p2.yValue()) / 2 }; //成员函数
}

 //使用constexpr函数的结果初始化constexpr对象

constexpr auto mid = midpoint(p1, p2);

// 在C++14的constexpr setter加持下：可以写出以下函数
//返回p相对于原点的镜像
constexpr Point reflection(const Point& p) noexcept
{
    Point result;                   //创建non-const Point
    result.setX(-p.xValue());       //设定它的x和y值
    result.setY(-p.yValue());
    return result;                  //返回它的副本
}

//reflectedMid的值(-19.1, -16.5)在编译期可知
constexpr auto reflectedMid =  reflection(mid);
}


// * constexpr函数的这种机能造就了：
// * 以前相对严格的编译期完成的工作和运行时完成的工作的界限变得模糊，
//   一些传统上在运行时的计算过程能并入编译时。
// * 这种代码越多，程序的运行时性能就越好，因为编译期计算的结果直接
//   存储在可执行文件中（但编译时间会变长）。


// （3） 小结：
// * constexpr对象和constexpr函数可以使用的范围比non-constexpr对象和函数大得多。
// * 使用constexpr关键字可以最大化你的对象和函数可以使用的场景



// 四、实践问题

// * constexpr是对象和函数接口的一部分，加上constexpr相当于宣称“我能被用在C++要求常量表达式的地方”

// 如果一个函数一开始被声明为constexpr，后来去掉constexpr使其不能在编译期求值，
// 那么会造成大量引用它的代码编译错误，因为这些代码可能依赖于该函数在编译期求值的能力。
// * 因此，“尽可能”的使用constexpr表示你需要【长期坚持】对某个对象或者函数施加这种限制



// 五、总结
// * constexpr对象是const，它被在编译期可知的值初始化
// * 当传递编译期可知的值时，constexpr函数可以产出编译期可知的结果
// * constexpr对象和函数可以使用的范围比non-constexpr对象和函数要大
// * constexpr是对象和函数接口的一部分