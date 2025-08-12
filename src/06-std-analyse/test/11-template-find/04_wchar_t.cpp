#include <HXLibs/log/Log.hpp>

// 但是如果都去掉 typename S, 则不会冲突
struct Test01 {
    template <typename T, std::size_t N> // 完全匹配
    void todo(T const (&arr)[N]) {
        (void)arr;
    }

    void todo(wchar_t const* arr) {
        (void)arr;
    }

    template <std::size_t N>
    void todo(wchar_t (&arr)[N]) {      // 完全匹配, 并且更具体
        (void)arr;
    }
};

template <typename T>
void func01(T const& t) {
    Test01{}.todo(t);
}

struct Test02 {
    template <typename T, std::size_t N, typename S>
#if 1
        requires (!std::is_same_v<T, wchar_t> // 0) 加上这个则没问题
               && !std::is_same_v<T, char>)
#endif
    void todo(T const (&arr)[N], S&) {
        (void)arr;
        HX::log::hxLog.debug(1);
    }

    template <typename S>
    void todo(wchar_t const* arr, S&) {      // 1) 去掉 const 则没问题
        (void)arr;                           // 2) 删除掉这个方法也没问题
        HX::log::hxLog.debug(2);
    }

    template <std::size_t N, typename S>
    void todo(wchar_t const (&arr)[N], S&) { // 3) 删除 const 则不参与冲突, 上面两个还是冲突
        (void)arr;                           // 4) 统一去掉 typename S 则没问题
        HX::log::hxLog.debug(3);
    }                                        // 5) 单纯删掉这个方法则不行, 上面两个还是冲突
};

template <typename T, typename S>
void func02(T const& t, S& s) {
    Test02{}.todo(t, s);
}

int main() {
    std::string s;
    func01(L"xxx");
    func02(L"xxx", s); // 传入 wchar_t const (&)[4], S = string
                            // func02 推导: T = wchar_t (&)[4], S = string
    /*
    没事, 得看 接下来的推导:
    对照)
    void todo(T const (&arr)[N], S&)        -> T = wchar_t, N = 4, S = string
    void todo(wchar_t const* arr, S&)       ->                     S = string (但, 隐式转换)
    void todo(wchar_t const (&arr)[N], S&)  ->              N = 4, S = string
    因为 S 都是可变参数, 所以 等价于 func(A) x 3, 故匹配不了
    // ps: 即使 #3 看起来更特化, 但标准要求模板参数推导后签名相同时视为等价

    0) 加上这个则没问题
    void todo(T const (&arr)[N], S&)        -> 不合适: 不满足约束
    void todo(wchar_t const* arr, S&)       ->                     S = string (但, 隐式转换)
    void todo(wchar_t const (&arr)[N], S&)  ->              N = 4, S = string (更具体: OK)

    1) 去掉 const 则没问题
    void todo(T const (&arr)[N], S&)        -> T = wchar_t, N = 4, S = string
    void todo(wchar_t* arr, S&)             ->                     S = string (匹配失败: 不可隐式转换)
    void todo(wchar_t const (&arr)[N], S&)  ->              N = 4, S = string
    因为传入的是 T = wchar_t const (&)[4], 无法隐式转换为 非const, 匹配失败

    2) 删除掉这个方法也没问题
    void todo(T const (&arr)[N], S&)        -> T = wchar_t, N = 4, S = string
    void todo(wchar_t const (&arr)[N], S&)  ->              N = 4, S = string (更具体: OK)

    3) 删除 const 则不参与冲突, 上面两个还是冲突
    void todo(T const (&arr)[N], S&)        -> T = wchar_t, N = 4, S = string
    void todo(wchar_t const* arr, S&)       ->                     S = string (但, 隐式转换)
    void todo(wchar_t (&arr)[N], S&)        ->              N = 4, S = string (匹配失败: 不可隐式转换)
    实际上是还有别的冲突, 因为 wchar_t const (&)[4] 无法隐式转换为 非const, 所以第三个匹配失败

    4) 统一去掉 typename S 则没问题
    void todo(T const (&arr)[N])            -> T = wchar_t, N = 4
    void todo(wchar_t const* arr)           ->                                (但, 隐式转换)
    void todo(wchar_t const (&arr)[N])      ->              N = 4             (更具体: OK)
    因为: 编译器根据匹配优先级 (数组引用 > 指针)

    5) 单纯删掉这个方法则不行, 上面两个还是冲突
    void todo(T const (&arr)[N], S&)        -> T = wchar_t, N = 4, S = string
    void todo(wchar_t const* arr, S&)       ->                     S = string (但, 隐式转换)
    因为多维推导, 1. 按照具体, wchar_t const* arr 更具体, 但是有隐式转换
                2. 按照匹配 T const (&arr)[N] 更匹配
                3. 所以谁也说服不了, 看看 S, 都是一样(标准未明确定义优先级(互不特化)) => 二义性

    体感核心: 字符串数组 和 字符指针 可以隐式转换, 并且是模板, 触发重载决议, 发现都ok, 所以二义性
    */
    return 0;
}