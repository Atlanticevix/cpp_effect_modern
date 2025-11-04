// 条款 11 - 优先考虑使用deleted函数而非使用未定义的私有声明

#include <type_traits>

// 一、问题示例
// C++98中，最常见的防止类被拷贝的方法是将拷贝构造函数和拷贝赋值运算符声明为private，
// 但不提供它们的定义。例如 iostream
namespace Item11_01
{
    template <class charT, class traits = char_traits<charT> >
    class basic_ios : public ios_base {
    public:

    private:

        // 故意不定义它们意味着假如还是有代码用它们（比如成员函数或者类的友元friend），
        // 就会在链接时引发缺少函数定义错误

        basic_ios(const basic_ios& );           // not defined
        basic_ios& operator=(const basic_ios&); // not defined
    };
} //

// 二、deleted 做得更彻底
namespace Item11_01
{
    template <class charT, class traits = char_traits<charT> >
    class basic_ios : public ios_base {
    public:

        // 1、deleted函数不能以任何方式被调用，即使你在成员函数或者友元函数里面
        // 调用deleted函数也不能通过编译(const basic_ios& ) = delete;

        // 2、deleted函数在类的接口中是可见（public）的，因此编译器能给出更好的错误信息
        basic_ios& operator=(const basic_ios&) = delete;
    };
} //

// 除了删除类成员函数，还可以删除一般函数，例如：
// 有一个非成员函数，它接受一个整型参数，检查它是否为幸运数：
namespace Item11_02
{
    bool isLucky(int n) {
        // 假设某些实现
        return n == 7;
    }

    // 为了防止误用，可以删除接受double参数的重载版本
    bool isLucky(char) = delete;    //拒绝char
    bool isLucky(bool) = delete;    //拒绝bool
    bool isLucky(double) = delete;  //拒绝float和double

    void test() {
        isLucky(7);    // 正确

        // isLucky(7.0); // 错误！调用被删除的函数
    }
} //


// 三、模板代码中的deleted函数
// 在模板代码中，deleted函数也很有用。例如：假如你要求一个模板仅支持原生指针
// 但又想排除某些指针类型的处理（如char*， void*），可以这样做：
namespace Item11_03
{
    template<typename T>
    void processPointer(T* ptr);

    // 排除char*和void*
    template<>
    void processPointer<void>(void*) = delete;

    template<>
    void processPointer<char>(char*) = delete;

    // 更进一步
    template<>
    void processPointer<const void>(const void*) = delete;

    template<>
    void processPointer<const char>(const char*) = delete;

    // 再更进一步排除const volatile void*和const volatile char*重载版本，
    // 另外还需要一并删除其他标准字符类型的重载版本：std::wchar_t，std::char16_t和std::char32_t

    // C++98 是无法做到这么细致的删除的

} //

// 另外，C++98不能给特化的成员模板函数指定一个不同于主函数模板的访问级别(public和private)
// 例如：
namespace IMtem11_04
{
    class Widget {
    public:
        template<typename T>
        void processPointer(T* ptr) {}

    private:
        // 错误！！
        template<>
        void processPointer<void>(void*);
        
    };

} // namespace

// deleted函数可以做到：
namespace IMtem11_05
{
    class Widget {
    public:
        template<typename T>
        void processPointer(T* ptr) {}

        // 还是public，但被删除了
        template<>
        void processPointer<void>(void*) = delete;
    };

} // namespace

// 四、总结
// * 比起声明函数为private但不定义，使用deleted函数更好
// * 任何函数都能被删除（be deleted），包括非成员函数和模板实例（译注：实例化的函数）