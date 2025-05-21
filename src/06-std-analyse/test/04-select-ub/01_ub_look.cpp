#include <HXprint/print.h>

/**
 * @brief 技术指导: https://142857.red/book/undef/
 * 完整列表:

 未定义行为完整列表
    建议开启标准库的调试模式
        msvc: Debug 配置
        gcc: 定义 _GLIBCXX_DEBUG 宏
    空指针类
        不能解引用空指针（通常会产生崩溃，但也可能被优化产生奇怪的现象）
        不能解引用 end 迭代器
        this 指针不能为空
        空指针不能调用成员函数
    指针别名类
        reinterpret_cast 后以不兼容的类型访问
        union 访问不是激活的成员
        T 类型指针必须对齐到 alignof(T)
        从父类 static_cast 到不符合的子类后访问
        bool 类型不得出现 0 和 1 以外的值
    算数类
        有符号整数的加减乘除模不能溢出
        左移或右移的位数，不得超过整数类型上限，不得为负
        除数不能为 0
        求值顺序类
        同一表达式内，对同一个变量有多个自增/自减运算
        内建类型的二元运算符，其左右两个参数求值的顺序是不确定的
        函数参数求值的顺序是不确定的
    函数类
        返回类型不为 void 的函数，必须有 return 语句
        函数指针被调用时，不能为空
        函数指针被调用时，参数列表或返回值必须匹配
        普通函数指针与成员函数指针不能互转
    生命周期类
        不能读取未初始化的变量
        指针的加减法不能超越数组边界
        可以有指向数组尾部的指针（类似 end 迭代器），但不能解引用
        不能访问未初始化的指针
        不能访问已释放的内存
        new / new[] / malloc 和 delete / delete[] / free 必须匹配
        不要访问已经析构的对象
        不能把函数指针转换为普通类型指针解引用
    库函数类
        ctype.h 中一系列函数的字符参数，必须在 0~127 范围内（即只支持 ASCII 字符）
        memcpy 函数的 src 和 dst 不能为空指针
        memcpy 不能接受带有重叠的 src 和 dst
        v.back() 当 v 为空时是未定义行为
        vector 的 operator[] 当 i 越界时，是未定义行为
        容器迭代器失效
        容器元素引用失效
    多线程类
        多个线程同时访问同一个对象，其中至少一个线程的访问为写访问，是未定义行为（俗称数据竞争）
        多个线程同时对两个 mutex 上锁，但顺序相反，会产生未定义行为（俗称死锁）
        对于非 recursive_mutex，同一个线程对同一个 mutex 重复上锁，会产生未定义行为（俗称递归死锁）
    总结
        CppCon 相关视频: https://www.youtube.com/watch?v=ehyHyAIa5so
 */


// #define __LOOK_UB_ERROR__ // 开启示例
#ifdef __LOOK_UB_ERROR__
#include <cstdint>

/**
 * @brief 有符号数不能溢出
 * @return constexpr int 
 */
constexpr int ub_01() {
    for (int8_t i = 1; i > 0; ++i) // ub! 会被优化为 while (1)
        ;                          // 原因: 有符号数 不能溢出
    return 0;                      // 修改为 uint8_t 则没问题
}

/**
 * @brief 位运算不能溢出
 * @return constexpr int 
 */
constexpr int ub_02() {
    constexpr auto _01 = 1 << 32;
    constexpr auto _02 = 1 >> 32;
    static_cast<void>(_01);
    static_cast<void>(_02);
    return 1U << 32U
        |  1U >> 32U;
}

/**
 * @brief ub: swap | 不能对: 同一表达式内, 对同一个变量有多个自增/自减运算
 * @param a 
 * @param b 
 * @return constexpr int 
 */
constexpr int ub_03(int& a, int& b) {
    // oi爷的写法
    // https://oi-wiki.org/math/bit/#%E4%BA%A4%E6%8D%A2%E4%B8%A4%E4%B8%AA%E6%95%B0
    a ^= b ^= a ^= b;
    return 0;
}

#endif // !__LOOK_UB_ERROR__

int main() {
#ifdef __LOOK_UB_ERROR__
    // 可以使用 c++17 的 constexpr 常量函数, 这样可以保证结果在编译期计算,
    // 而且C++规定了编译期是不能出现未定义行为的, 因此如果出现了未定义行为, 就会直接编译错误了~
    constexpr auto _01 = ub_01();
    constexpr auto _02 = ub_02();
    static_cast<void>(_01);
    static_cast<void>(_02);

    int a = 2233, b = 666;
    HX::print::println("begin: ", a, " ", b);
    constexpr auto _03 = ub_03(a, b);
    HX::print::println("end: ", a, " ", b);
    static_cast<void>(_03);
#endif // !__LOOK_UB_ERROR__
    return 0;
}