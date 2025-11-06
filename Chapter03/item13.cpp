// 条款13 - 优先考虑const_iterator而非iterator

#include <iterator>
#include <algorithm>
#include <vector>

// STL const_iterator等价于指向常量的指针（pointer-to-const）。它们都指向不能被修改的值。
// 因此，除非确实需要修改容器中的元素，否则应优先使用const_iterator。

// 一、问题示例

// C++98的const_iterator的支持有限。
namespace Item13_01 {

    // 不用using是为了更好展示C++98代码风格
    typedef std::vector<int>::iterator IterT;
    typedef std::vector<int>::const_iterator ConstIterT;


    void test() {
        std::vector<int> values = {1, 2, 3, 4, 5};

        // 假设我们想查找1983这个数，并在它前面插入1998

        // values是non-const， 从其获取const_iterator的做法都有点别扭
        // 这里的cast是无法通过编译的，这里只是举例才这么处理，
        ConstIterT ci =
            std::find(static_cast<ConstIterT>(values.begin()),  //cast
                      static_cast<ConstIterT>(values.end()),    //cast
                      1983);

        // insert不允许插入const_iterator，因此下面这行代码无法通过编译
        values.insert(static_cast<IterT>(ci), 1998);
    }

}


// 二、C++11改进

//const_iterator变得既容易获取又容易使用：
// * 容器的成员函数cbegin和cend产出const_iterator，甚至对于non-const容器也可用
// * 使用iterator指示位置（如insert和erase）的STL成员函数也可以使用const_iterator
namespace Item13_02 {
    void test() {
        std::vector<int> values = {1, 2, 3, 4, 5};

        // 查找1983这个数，并在它前面插入1998
        auto ci = std::find(values.cbegin(), values.cend(), 1983);

        // insert允许使用const_iterator，因此下面这行代码可以通过编译
        values.insert(ci, 1998);
    }   
}


// 三、C++14改进
// C++11只添加了非成员函数begin和end，但是没有添加cbegin，cend，rbegin，rend，crbegin，crend。
// C++ 14补充了这些非成员函数。
namespace Item13_03 {

    template<typename C, typename V>
    void findAndInsert(C& container,            //在容器中查找第一次
                    const V& targetVal,      //出现targetVal的位置，
                    const V& insertVal)      //然后在那插入insertVal
    {
        using std::cbegin;
        using std::cend;

        auto it = std::find(cbegin(container),  //非成员函数cbegin
                            cend(container),    //非成员函数cend
                            targetVal);
        container.insert(it, insertVal);
    }
 
}

// 如果需要弥补C++11的遗漏，可以自己实现这些非成员函数，如：
namespace Item13_04 {

    // 1、 这个cbegin模板接受任何代表类似容器的数据结构的实参类型C
    // 并且通过reference-to-const形参container访问这个实参

    // 2、 如果C是一个普通的容器类型（如std::vector<int>），container将
    // 会引用一个const版本的容器（如const std::vector<int>&）

    // 3、 对const容器调用非成员函数begin（由C++11提供）将产出const_iterator，
    // 这个迭代器也是模板要返回的【->decltype(std::begin(container))】

    // 最终可以返回这个const_iterator
    template <class C>
    auto cbegin(const C& container)->decltype(std::begin(container))
    {
        return std::begin(container);   //解释见下
    }


    // 如果C是原生数组，这个模板也能工作。这时，container成为一个const数组的引用。
    // C++11为数组提供特化版本的非成员函数begin，它返回指向数组第一个元素的指针。
    // 一个const数组的元素也是const，所以对于const数组，非成员函数begin返回
    // 指向const的指针（pointer-to-const）。在数组的上下文中，
    // 所谓指向const的指针（pointer-to-const），也就是const_iterator了。

}


// 四、总结
// * 优先考虑const_iterator而非iterator
// * 在最大程度通用的代码中，优先考虑非成员函数版本的begin，end，rbegin等，而非同名成员函数