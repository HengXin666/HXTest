#include <HXprint/print.h>

#define THE_JJ(x, y) x##y
#define THE_J(x, y) x#y
#define THE_(x, y) x y

// THE_JJ(1e, 2);      // 1e2  语法拼接 (预处理阶段)
// THE_J("1", "2");    // "1""\"2\"", 等价于 "1\"2\"" 字符串拼接
// THE_(1, 2);         // 1 2  正常转发文本内容

#define THE_ARG_JJ(x, ...) x##__VA_ARGS__
#define THE_ARG_J(x,  ...) x#__VA_ARGS__
#define THE_ARG_(x, ...)   x __VA_ARGS__

// THE_ARG_(1, 2, 3)       // -> 1 2, 3 朴素转发可变参数, 注意可变参数是转发为 arg1, arg2, ... 的, 带逗号的
// THE_ARG_J("1", 2, 3)    // -> "12, 3" 字符串
// THE_ARG_JJ(1, 2, 3)     // -> 12, 3, 第一个是拼接token, 剩下的是 arg 展开

// #define THE_ARG_JJ_CALL(...) 0, ##__VA_ARGS__

// THE_ARG_JJ_CALL(1, 2, 3) // 0, 1, 2, 3
// THE_ARG_JJ_CALL()        // 0

#define THE_OPT(...) \
int arr[] = {2233 __VA_OPT__(, __VA_ARGS__)}

// THE_OPT();

#define SX_1() SX_0()
#define SX_0() "1 -> 0"

// SX_1();

#define SX_ERR() SX_ERR_ON()

// SX_ERR(); // A type specifier is required for all declaration -> SX_ERR_ON

#define SX_ERR_ON() "Err"

int main() {
    return 0;
}