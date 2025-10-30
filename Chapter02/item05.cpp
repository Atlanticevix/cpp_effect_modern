/* 条款5 - 优先选用auto， 而非显示类别声明 */

#include <memory>
#include <functional>
#include <unordered_map>

// 一、简化声明

namespace 
{
    template<typename It>           //对从b到e的所有元素使用
    void dwim(It b, It e)           //dwim（“do what I mean”）算法
    {
        while (b != e) {
            typename std::iterator_traits<It>::value_type currValue = *b;
            // …
        }
    }

}

// C++ 11以后
namespace 
{
    template<typename It>           //对从b到e的所有元素使用
    void dwim(It b, It e)           //dwim（“do what I mean”）算法
    {
        while (b != e) {
            auto currValue = *b;
            // …
        }
    }

}



namespace 
{
    class Widget
    {
    public:
        bool operator<(const Widget &rhs) const {
            return false;
        }

    };

    

    void test() {
        //用于std::unique_ptr<Widget>的比较函数
        auto derefUPLess =  [](const std::unique_ptr<Widget> &p1, const std::unique_ptr<Widget> &p2) { 
                return *p1 < *p2;
            }; 

         // C++14版本 lambda表达式中的auto参数
         // 被任何像指针一样的东西指向的值的比较函数
        auto derefLess = [](const auto& p1, const auto& p2) { 
                return *p1 < *p2; 
            };
    }
}


// 二、 auto 在Lambda声明上的 比 std：function 的优势

namespace {
    
    void test() {
        
        // 使用std::function声明比较函数
        // std::function 声明的是一个对象，对象需要开辟内存来保存lambda表达式
        std::function<bool(const std::unique_ptr<Widget> &, const std::unique_ptr<Widget> &)> derefUPLess = 
            [](const std::unique_ptr<Widget> &p1, const std::unique_ptr<Widget> &p2) { 
                return *p1 < *p2;
            };

        // 使用auto声明比较函数
        // auto 声明的变量保存一个和闭包一样类型的（新）lambda，不需要额外的内存开销
        auto derefUPLess2 = 
            [](const std::unique_ptr<Widget> &p1, const std::unique_ptr<Widget> &p2) { 
                return *p1 < *p2;
            };
    }
}


// 三、 正确声明变量

namespace 
{
    void test() {

        // 在Windows 32-bit上std::vector<int>::size_type和unsigned是一样的大小，
        // 但是在Windows 64-bit上std::vector<int>::size_type是64位，unsigned是32位
        std::vector<int> v;
        unsigned sz = v.size(); // 丢失精度
    }

    void test2() {

        std::unordered_map<std::string, int> m;

        // 性能问 - 每次循环会产生一个pair的复制
        // 1、std::unordered_map的key是const的，所以hash table中的std::pair的
        // 类型不是std::pair<std::string, int>，而是std::pair<const std::string, int>
        // 2、编译器会尽努力实现该循环（编译成功），但问题是通过拷贝m中的每个元素给p。
        
        for(const std::pair<std::string, int>& p : m)
        {
        }

        // 使用auto避免上述问题（通过编译器提示查看p类型）
        for(const auto& p : m)
        {
        }
    }
}


// 四、 auto的陷阱
// 如Item02中提到的初始化列表、代码可读性等问题

