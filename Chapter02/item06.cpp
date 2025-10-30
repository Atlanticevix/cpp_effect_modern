/* 条款6 - auto推导若非己愿，使用显式类型初始化惯用法 */


#include <vector>


// 一、auto的带来的问题
namespace 
{
    class Widget
    {

    };

    std::vector<bool> features(const Widget &w) {
        return std::vector<bool>{true, false, true, false, true, false, true, false};
    }

    void processWidget(const Widget &w, bool b) {
    }

    void test() {

        Widget w;

        bool highPriority = features(w)[5];  // OK

        // auto highPriority = features(w);  // 错误！features1是std::vector<bool>
        // std::vector<bool>::reference之所以存在是因为std做了优化，std::vector<bool>规定了
        // 使用一个打包形式（packed form）表示它的bool，每个bool占一个bit。
       

        bool highPriority = static_cast<bool>(features(w)[5]); // 显式转换，OK

        processWidget(w, highPriority); // 编译通过
    }
}

// 二、小心代理类

// std::vector<bool>::reference 其实是一个代理类，std类型的还有 std::bitset::reference、
// std::shared_ptr和std::unique_ptr

// 代理类的威力，例如：
// Matrix sum = m1 + m2 + m3 + m4;
// 只需要使让operator+返回一个代理类代理结果而不是返回结果本身，可以是用更少的内存和更快的速度完成操作。
// 如operator+将会返回如Sum<Matrix, Matrix>这样的代理类作为结果而不是直接返回一个Matrix对象
// 所以，使用auto时要小心，避免将代理类类型赋给auto变量。


// 三、 auto不是问题，是你的认知和实践太浅

// 当你越熟悉你使用的库的基本设计理念，你的思维就会越活跃，不至于思维僵化认为代理类只能在这些库中使用。
// 当缺少文档的时候，可以去看看头文件。很少会出现源代码全都用代理对象，它们通常用于一些函数的返回类型，
// 所以通常能从函数签名中看出它们的存在。这里有一份std::vector<bool>::operator[]的说明书：
/*
namespace std{                                  //来自于C++标准库
    template<class Allocator>
    class vector<bool, Allocator>{
    public:
        …
        class reference { … };

        reference operator[](size_type n);
        …
    };
}
*/
namespace 
{
    class Widget
    {

    };

    std::vector<bool> features(const Widget &w) {
        return std::vector<bool>{true, false, true, false, true, false, true, false};
    }

    void processWidget(const Widget &w, bool b) {
    }

    void test() {

        Widget w;

        bool highPriority = static_cast<bool>(features(w)[5]); // 显式转换，OK

        processWidget(w, highPriority); // 编译通过
    }
}