// 条款12 - 使用override声明重写函数

#include <vector>

// 一、什么是override（重写）

// a. “重写（overriding）”听起来像“重载（overloading）”，然而两者完全不相关.

// b. overriding的条件：
// * 基类函数必须是virtual
// * 基类和派生类函数名必须完全一样（除非是析构函数)
// * 基类和派生类函数形参类型必须完全一样
// * 基类和派生类函数常量性constness必须完全一样
// * 基类和派生类函数的返回值和异常说明（exception specifications）必须兼容
// * (C++11起) 派生类函数必须有和基类函数一样的ref-qualifier（&或&&)

namespace Item11_01 {
    class Widget {
    public:
        void doWork() &;    //只有*this为左值的时候才能被调用
        void doWork() &&;   //只有*this为右值的时候才能被调用
    }; 

    void test() {

        Widget makeWidget();    //工厂函数（返回右值）
        Widget w;               //普通对象（左值）

        w.doWork();             //调用被左值引用限定修饰的Widget::doWork版本
                                //（即Widget::doWork &）
        makeWidget().doWork();  //调用被右值引用限定修饰的Widget::doWork版本
                                //（即Widget::doWork &&）
    }

}

//  二、问题示例
// 这么多的重写需求意味着哪怕一个小小的错误也会造成巨大的不同。
// 而且这些错误是合法，编译器不会报错。 例如：
namespace Item11_02 {
    class Base {
    public:
        virtual void mf1() const;
        virtual void mf2(int x);
        virtual void mf3() &;
        void mf4() const;
    };

    class Derived: public Base {
    public:
        virtual void mf1();
        virtual void mf2(unsigned int x);
        virtual void mf3() &&;
        void mf4() const;
    };

    // a.mf1在Base基类声明为const，但是Derived派生类没有这个常量限定符
    // b.mf2在Base基类声明为接受一个int参数，但是在Derived派生类声明为接受unsigned int参数
    // c.mf3在Base基类声明为左值引用限定，但是在Derived派生类声明为右值引用限定
    // d. mf4在Base基类没有声明为virtual虚函数(属于覆盖隐藏而非重写)
}

// 三、使用override关键字
// C++11引入了override限定符，专门用于标记重写函数
// 使用override后，编译器会打开所有与重写有关的问题
namespace Item11_03 {
    class Base {
    public:
        virtual void mf1() const;
        virtual void mf2(int x);
        virtual void mf3() &;
        void mf4() const;
    };

    class Derived: public Base {
    public:
        virtual void mf1() const override;
        virtual void mf2(int x) override;
        virtual void mf3() & override;
        void mf4() const override; // 错误！Base::mf4不是virtual
    };

}

// final关键字，用于阻止进一步重写
namespace Item11_04 {
    class Base {
    public:
        virtual void mf1() final; // 不能被重写
        virtual void mf2();
    };

    class Derived: public Base {
    public:
        virtual void mf1() override; // 错误！Base::mf1是final，不能被重写
        virtual void mf2() override;
    };
}


// * 上下文关键字（contextual keywords） final和override并不是保留字（reserved words）
// 这两个关键字的特点是它们是保留的，它们只是位于特定上下文才被视为关键字。
// 对于override，它只在成员函数声明结尾处才被视为关键字。
// 这意味着如果你以前写的代码里面已经用过override这个名字。

namespace Item11_05 {
    class Warning 
    {         //C++98潜在的传统类代码
    public:

        void override();    //C++98和C++11都合法（且含义相同）

    };
}

// * 四、成员函数引用限定（reference qualifiers）
// 例：
namespace Item11_06 {
    class Widget {
    public:
        using DataType = std::vector<double>;   //“using”的信息参见Item9

        // Widget::data函数的返回值是一个左值引用（准确的说是std::vector<double>&）
        DataType& data() { return values; }

    private:
        DataType values;
    };


    void test() {

        Widget w;

        // 左值引用是左值，所以vals1是从左值初始化的。因此vals1由w.values拷贝构造而得
        auto vals1 = w.data(); 
    }
}

// 加入右值引用限定符后的例子：
namespace Item11_06 {
    Widget makeWidget() {
        return Widget(); // 返回的临时对象（即右值）
    }

    void test() {

        // makeWidget返回的临时对象（即右值），所以将其中的std::vector进行拷贝纯属浪费，最好是移动
        // 但是因为data返回左值引用，C++的规则要求编译器不得不生成一个拷贝。
        auto vals1 = makeWidget().data(); 
    }
}


// 改进后的Widget类：
namespace Item11_07 {
    class Widget {
    public:
        using DataType = std::vector<double>;   //“using”的信息参见Item9

        // 左值引用限定版本
        DataType& data() & { return values; }

        // 右值引用限定版本
        DataType data() && { return std::move(values); }

    private:
        DataType values;
    };

    Widget makeWidget() {
        return Widget(); // 返回的临时对象（即右值）
    }

    void test() {

        Widget w;

        //调用左值重载版本的Widget::data，拷贝构造vals1
        auto vals1 = w.data();              

        //调用右值重载版本的Widget::data, 移动构造vals2
        auto vals2 = makeWidget().data();
    }
}

// 五、总结
// * 为重写函数加上override
//* 成员函数引用限定让我们可以区别对待左值对象和右值对象（即*this)