#include <HXprint/print.h>

template <typename T>
auto funA1(T t) {       // 有 const 也不会保留, 仅返回 T
                        // 忽略所有的 & 与 &&
    return t;
}

template <typename T>
auto& funA2(T& t) {     // 有 const 会保留 const
    return t;
}

template <typename T>
auto&& funA3(T&& t) {   // 有 const 会保留 const
    if constexpr (0) {
        return t; // 返回 & 类型, 和 auto&& + & -> auto&&& 触发引用折叠 [(&&)&] -> auto&
    } else if constexpr (0) {
        return std::move(t); // 返回 && 类型, 和 auto&& + && -> auto&&&& 触发引用折叠 [(&&)&&] -> auto&&
    } else {
        return std::forward<T>(t);
    }
}

// #define __TEST__
#ifdef __TEST__
template <typename T>
auto&& funA3_nr(T t) {
    return t;
}
#endif // !__TEST__

template <typename T>
decltype(auto) funA4(T&& t) {   // decltype(auto) 是返回值的完美转发
    if constexpr (0) {          // 如果返回值是引用, 则返回引用
                                // 如果返回值是右值, 则返回右值
                                // 如果返回值无&, 仅返回该类型
                                // 一切操作保留原有的 const
        return t;
    } else if constexpr (0) {
        return std::move(t);
    } else {
        return std::forward<T>(t);
    }
}

template <typename T>
decltype(auto) funA4_nr(T t) {
    return t;
}

template <typename T>
std::string_view getType() {
#if !defined(_MSC_VER)
    auto str = __PRETTY_FUNCTION__;
    std::string_view res{str};
    res = res.substr(res.find('[') + 1);
    res = res.substr(0, res.size() - 1);
    return res;
#else
    auto str = __FUNCSIG__; // MSVC 的 __FUNCSIG__
    std::string_view res{str};
    res = res.substr(res.find("getType<") + 8);
    res = res.substr(0, res.find(">"));
    return res;
#endif
}

#define infoType(__TYPE__) \
HX::print::println(#__TYPE__": ", getType<__TYPE__>())

int main() {
    int x = 0;
    const int c_x = x;
    int& r_x = x;
    const int& cr_x = x;
    
    infoType(decltype(funA1(x)));
    infoType(decltype(funA1(c_x)));
    infoType(decltype(funA1(r_x)));
    infoType(decltype(funA1(cr_x)));
    infoType(decltype(funA1(std::move(x))));

    HX::print::println("");

    infoType(decltype(funA2(x)));
    infoType(decltype(funA2(c_x)));
    infoType(decltype(funA2(r_x)));
    infoType(decltype(funA2(cr_x)));
    // using AutoType2_move_x = decltype(funA2(std::move(x))); // 没有匹配的函数

    HX::print::println("");

    infoType(decltype(funA3(x)));
    infoType(decltype(funA3(c_x)));
    infoType(decltype(funA3(r_x)));
    infoType(decltype(funA3(cr_x)));
    infoType(decltype(funA3(std::move(x))));

#ifdef __TEST__
    infoType(decltype(funA3_nr(x)));
#endif // ！__TEST__
    // infoType(decltype(funA3_nr(c_x)));
    // infoType(decltype(funA3_nr(r_x)));
    // infoType(decltype(funA3_nr(cr_x)));
    // infoType(decltype(funA3_nr(std::move(x))));
    
    HX::print::println("");

    infoType(decltype(funA4(x)));
    infoType(decltype(funA4(c_x)));
    infoType(decltype(funA4(r_x)));
    infoType(decltype(funA4(cr_x)));
    infoType(decltype(funA4(std::move(x))));

    infoType(decltype(funA4_nr(x)));
    // infoType(decltype(funA4_nr(c_x)));
    // infoType(decltype(funA4_nr(r_x)));
    // infoType(decltype(funA4_nr(cr_x)));
    // infoType(decltype(funA4_nr(std::move(x))));

/*
    decltype(auto) 和 auto&& 在返回值时候唯一不同的是: 
    - 后者永远是带引用的, 
    - 而前者是真正意义的完美转发, 还可以被RVO(消除复制)优化
*/

    return 0;
}