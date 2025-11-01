// 条款08 - 优先考虑nullptr而非0和NULL

#include <mutex>

// 一、历史问题
// NULL是可以任意被定义的东西，可以是int类型的0，也可以是long类型的0，甚至是void指针类型。（甚至可能为非0）

// 但这里的问题不是NULL没有一个确定的类型，而是0和NULL都不是指针类型。

// 一般来说C++的解析策略是把0看做int而不是指针。
//  在C++98中，如果给下面的重载函数传递0或NULL，它们绝不会调用指针版本的重载函数
namespace
{
void f(int);        //三个f的重载函数
void f(bool);
void f(void*);

    void test()
    {
    f(0);               //调用f(int)而不是f(void*)


    f(NULL);            //可能不会被编译，一般来说调用f(int)，
                        //绝对不会调用f(void*)
    }
}

// 二、nullptr的优点

// C++11引入了nullptr关键字，表示空指针字面值，类型为std::nullptr_t，可以隐式转换为任何指针类型。

// a、但绝对不会是一个整形类型
namespace
{
    void f(int);        //三个f的重载函数
    void f(bool);
    void f(void*);

    void test()
    {
        int a = static_cast<int>(nullptr);      //错误！nullptr不能转换为int

        f(nullptr);        //调用f(void*)，没有二义性
    }
} // namespace

// b、辅助auto的类型推导
namespace
{
    void test()
    {
        auto result = findRecord( /* arguments */ );

        // findRecord可能返回指针类型或者整形类型，这样的话就无法确定result的类型
        if (result == 0) {
        } 

        // 如下写法就没有二义性了，findRecord返回的是指针类型
        if (result == nullptr) {
        }
    }
} // namespace


// c、模板推导时更安全
// 在模板代码中，使用0或NULL可能会引起二义性，使用nullptr则不会

// 有以下代码，代码重复上锁功能：
namespace
{
    class Widget
    {
    };

    int    f1(std::shared_ptr<Widget> spw);     //只能被合适的
    double f2(std::unique_ptr<Widget> upw);     //已锁互斥量
    bool   f3(Widget* pw);  

    void test()
    {
        std::mutex f1m, f2m, f3m;       //用于f1，f2，f3函数的互斥量

        using MuxGuard =                //C++11的typedef，参见Item9
            std::lock_guard<std::mutex>;

        {  
            MuxGuard g(f1m);            //为f1m上锁
            auto result = f1(0);        //向f1传递0作为空指针
        }                               //解锁 

        {  
            MuxGuard g(f2m);            //为f2m上锁
            auto result = f2(NULL);     //向f2传递NULL作为空指针
        }                               //解锁 

        {
            MuxGuard g(f3m);            //为f3m上锁
            auto result = f3(nullptr);  //向f3传递nullptr作为空指针
        }                               //解锁 
    }
} // namespace

// 简化：
namespace
{
    template <typename Func, typename Mutex, typename Ptr>
    auto callWithLock(Func f, Mutex& mtx, Ptr pw) -> decltype(f(pw))
    {
        std::lock_guard<Mutex> g(mtx);    //上锁
        return f(pw);                     //调用函数
    }                                     //解锁

    void test()
    {
        std::mutex mtx;

        auto result1 = callWithLock(f1, mtx, 0); //错误 调用f1
        auto result2 = callWithLock(f2, mtx, NULL); //错误 调用f2
        auto result3 = callWithLock(f3, mtx, nullptr); //OK 调用f3
    }
} // namespace

// 推导解析：
// 在第一个调用中存在的问题是当0被传递给callWithLock模板，
// 模板类型推导会尝试去推导实参类型，0的类型总是int，所以这就是这次
// 调用callWithLock实例化出的ptr为int的类型。这意味着callWithLock中func
// 会被int类型的实参调用，这与f1期待的std::shared_ptr<Widget>形参不符。

// 传递0给callWithLock本来想表示空指针，但是实际上得到的一个普通的int。
// 把int类型看做std::shared_ptr<Widget>类型给f1自然是一个类型错误。

// 然而，使用nullptr是调用没什么问题。当nullptr传给lockAndCall时，
// ptr被推导为std::nullptr_t。当ptr被传递给f3的时候，隐式转换使std::nullptr_t转换为Widget*，
// 因为std::nullptr_t可以隐式转换为任何指针类型。

// 三、总结
// * 优先考虑nullptr而非0和NULL
// * 避免重载指针和整型