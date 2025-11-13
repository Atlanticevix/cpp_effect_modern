// item 14 如果函数不抛出异常请使用noexcept

#include <string>

// 一、概述

// 在C++11中，无条件的noexcept保证函数不会抛出任何异常，一个函数不会抛出异常，
// 最好是显式地用noexcept来标记它。这样做有两个好处：
// 1、提高代码可读性，读者一眼就能看出函数不会抛出异常
// 2、允许编译器进行更多优化，从而生成更高效的代码


namespace Item14_01 {
    int f(int x) throw();   //C++98风格，没有来自f的异常
    int f(int x) noexcept;  //C++11风格，没有来自f的异常

}
// 如果在运行时，f出现一个异常，那么就和f的异常说明冲突了。在C++98的异常说明中，
// 调用栈（the call stack）会展开至f的调用者，在一些与这地方不相关的动作后，程序被终止。
// C++11异常说明的运行时行为有些不同：调用栈只是可能在程序终止前展开。

// 展开调用栈和可能展开调用栈两者对于代码生成（code generation）有非常大的影响。
// 在一个noexcept函数中，当异常可能传播到函数外时，
// 优化器不需要保证运行时栈（the runtime stack）处于可展开状态；
// 也不需要保证当异常离开noexcept函数时，noexcept函数中的对象按照构造的反序析构。


// 二、STL中的noexcept

//

// a、在C++98中，STL容器的移动操作（move operations）是通过拷贝实现的，
// 如vector新增元素时，会分配一个新的更大块的内存用于存放其中元素，
// 然后将元素从老内存区移动到新内存区，然后析构老内存区里的对象。
// 在C++98中，移动是通过复制老内存区的每一个元素到新内存区完成的，
// 然后老内存区的每个元素发生析构。

// 这种方法使得push_back可以提供很强的异常安全保证：如果在复制元素期间抛出异常，
// std::vector状态保持不变，因为老内存元素析构必须建立在它们已经成功复制到新内存的前提下


// C++11引入了移动语义（move semantics），允许STL容器通过移动而非复制来重新安置元素，
// 这通常更高效，因为移动通常比复制更便宜。 但是，移动操作可能抛出异常，
// 这使得STL容器在重新安置元素时无法提供和C++98同样强的异常安全保证。 例如，假设vector在重新安置元素时，
// 第n个元素的移动操作抛出异常，那么前n-1个元素已经被移动到新内存区，而第n个元素仍然在老内存区，
// 这使得vector处于一种不一致的状态。 

// 为了应对这个问题，STL容器在重新安置元素时，首先检查元素的移动操作是否被标记为noexcept。拿
// std::vector举例，push_back之类的函数调用std::move_if_noexcept，这是个std::move的变体，
// 根据其中类型的移动构造函数是否为noexcept的，视情况转换为右值或保持左值。std::move_if_noexcept
// 查阅std::is_nothrow_move_constructible这个type trait，基于移动构造函数是否有noexcept（或者throw()）
// 的设计，编译器设置这个type trait的值



// b、swap，它的广泛使用意味着对其施加不抛异常的优化是非常有价值的。
// 标准库的swap是否noexcept有时依赖于用户定义的swap是否noexcept。
namespace Item14_02
{
    // std在STL中的定义
    //这些函数视情况noexcept：它们是否noexcept依赖于noexcept声明中的表达式是否noexcept
    template <class T, size_t N>
    void swap(T (&a)[N],
            T (&b)[N]) noexcept(noexcept(swap(*a, *b)));  //见下文

    template <class T1, class T2>
    struct pair {

        void swap(pair& p) noexcept(noexcept(swap(first, p.first)) &&
                                    noexcept(swap(second, p.second)));

    };

} // namespace name


// 三、实践问题

// a、实践一：一开始声明为noexcept的函数，后来修改代码使其可能抛出异常。
// 那么会影响使用该函数的代码

// b、看不明白，后面补。。


// c、实践三：C++98允许内存释放（memory deallocation）函数（即operator delete和operator delete[]）和析构函数抛出异常（糟糕设计）

// 在C++11中，这些函数被隐式地标记为noexcept。
// 析构函数非隐式noexcept的情况仅当类的数据成员（包括继承的成员还有继承成员内的数据成员）明确
// 声明它的析构函数可能抛出异常（如声明“noexcept(false)”）。 这种情况不常见，STL中没有这样的类。

// d、实践四：严格契约调用可以视为noexcept

// 接口有宽泛契约（wild contracts）和严格契约（narrow contracts），
// 宽泛契约，没有前置条件（文档约束或口头约束）。这种函数不管程序状态如何都能调用，它对调用者传来的实参不设约束。
// 严格契约，有前置条件。调用者必须保证这些前置条件成立，否则函数行为未定义。在严格定义下，函数执行可以认为时无异常， 例如：
namespace Item14_06
{
    // 设置前置条件：s.length() <= 32 
    // 只要调用者严格遵守前置条件，f就不会抛出异常
    void f(const std::string& s) noexcept;  

} // namespace name


// e、实践五：只要确保函数不会抛出异常，上层调用可以使用noexcept

// 看起来有点矛盾: doWork声明为noexcept，即使它调用了non-noexcept函数setup和cleanup。
// 其实不然，不写noexcept可能有更深层的原因，如：
// * 它们可能是用C写的库函数的一部分（C函数没有异常规范）；
// * 或者它们可能是用98风格编写的旧代码的一部分（C++98没有noexcept）；

// 因为有很多合理原因解释为什么noexcept依赖于缺少noexcept保证的函数, 编译器一般不会给出相应的warning

namespace Item14_07
{
    void setup(); 
    void cleanup();

    
    void doWork() noexcept
    {
        setup();               //设置要做的工作
        // do something ...
        cleanup();             //执行清理动作
    }


} // namespace name

// 四、总结
// * noexcept是函数接口的一部分，这意味着调用者可能会依赖它
// * noexcept函数较之于non-noexcept函数更容易优化
// * noexcept对于移动语义，swap，内存释放函数和析构函数非常有用
// * 大多数函数是异常中立的（译注：可能抛也可能不抛异常）而不是noexcept