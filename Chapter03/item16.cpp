// Item 16 - 让const成员函数线程安全

#include <atomic>
#include <cmath>
#include <mutex> 

// 一、const并非然线程安全
// const成员函数虽然不能修改对象的可变状态，但它们可能并非线程安全的，
// 因为它们可能修改对象的不可变状态，例如通过mutable数据成员或者
// 调用非const函数来修改对象的状态。

namespace Item16_01 {

    class Point {                                   //2D点
    public:
        double distanceFromOrigin() const noexcept  //noexcept的使用
        {                                           //参考条款14
            ++callCount;                            //atomic的递增
            
            return std::sqrt((x * x) + (y * y));
        }

    private:
        // mutable允许在const成员函数中修改它，但在多线程环境下需要确保安全
        // 使用atomic类型确保对callCount的修改是线程安全的且有更小的开销
        mutable std::atomic<unsigned> callCount{ 0 };
        double x, y;
    };
}


// 但atomic类型的使用也有缺点：
namespace Item16_02 {

    class Widget {
        int expensiveComputation1() const { return 0; } //占位符
        int expensiveComputation2() const { return 0; } //占位符

    public:
        

        int magicValue1() const
        {
            if (cacheValid) return cachedValue;
            else {
                auto val1 = expensiveComputation1();
                auto val2 = expensiveComputation2();

                cachedValue = val1 + val2; 

                // 在没有将cacheValid设为true之前，其他线程的cacheValid都是false
                // 所以每条线程都有可能执行一遍昂贵开销的计算函数expensiveComputation1和2
                cacheValid = true; 
                return cachedValue;
            }
        }


        // 如果换个位置，情况会更糟糕
        int magicValue2() const
        {
            if (cacheValid) return cachedValue;
            else {
                auto val1 = expensiveComputation1();
                auto val2 = expensiveComputation2();

                // 第一条线程已经将cacheValid置为true,
                // 其他线程如果在第一个判断条件【 if (cacheValid)】之前读取cachedValue，
                // 就会得到一个未知的值
                cacheValid = true; 
                
                return cachedValue = val1 + val2;
            }
        }
        
    private:
        mutable std::atomic<bool> cacheValid{ false };
        mutable std::atomic<int> cachedValue;
    };
}


// 改进：
// 使用互斥锁保护对缓存的访问
namespace Item16_03
{
    class Widget {

        int expensiveComputation1() const { return 0; } //占位符
        int expensiveComputation2() const { return 0; } //占位符
    public:

        int magicValue() const
        {
            std::lock_guard<std::mutex> guard(m);   //锁定m
            
            if (cacheValid) return cachedValue;
            else {
                auto val1 = expensiveComputation1();
                auto val2 = expensiveComputation2();
                cachedValue = val1 + val2;
                cacheValid = true;
                return cachedValue;
            }
        }                                           //解锁m


    private:
        mutable std::mutex m;
        mutable int cachedValue;                    //不再用atomic
        mutable bool cacheValid{ false };           //不再用atomic
    };
} // namespace name


// 二、总结
// * 确保const成员函数线程安全，除非你确定它们永远不会在并发上下文（concurrent context）中使用。
// * 使用std::atomic变量可能比互斥量提供更好的性能，但是它只适合操作单个变量或内存位置。