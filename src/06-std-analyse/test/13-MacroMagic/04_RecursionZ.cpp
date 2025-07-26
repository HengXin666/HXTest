#define EMPTY()
#define DEFER(id) id EMPTY()
#define OBSTRUCT(...) __VA_ARGS__ DEFER(EMPTY)()
#define EXPAND(...) __VA_ARGS__

#define CALL(x) x
#define A() 123

// A()                 // 展开为 123
// CALL(A)()           // 展开为 123
// DEFER(A)()          // 展开为 A (), 因为它需要再扫描一次才能完全展开

/*
    下面是解析过程, ! 表示发现的宏, ~ 表示没有识别此为宏, ^ 表示解析器光标

    EXPAND(DEFER(A)())
    !      !         ^ 全部解析, 栈内有两个({EXPAND, DEFER})待展开的宏, 出栈一个:

    出栈, 展开
    EXPAND(A EMPTY()())
           ~ !        ^ 全部解析, 栈内有两个({EXPAND, EMPTY})待展开的宏, 出栈一个
    
    出栈, 展开, 注意: 此时光标在 A 后面, 没有包含 A, 因此没有展开 A
    EXPAND(A ())
             ^ 括号都不认识, 回退(栈内有一个({EXPAND})待展开的宏, 出栈一个)
    
    出栈, 展开, 此时光标已经回退到 开头 (EXPAND处)
    A()
    !  ^ 全部解析, 栈内有一个({A})待展开的宏, 出栈一个

    123 解析宏完毕
*/
// EXPAND(DEFER(A)())  // 展开为 123, 因为 EXPAND 宏会强制多一次扫描

#pragma message("字符串? 我滴天啊")

#define STR(x) #x
#define CALL_STR(x) STR(x)
#define LOG_MACRO(x) _Pragma(STR(message("当前宏: " CALL_STR(x))))

LOG_MACRO(EXPAND(DEFER(A)()))

int main() {
    return 0;
}

/* 证明 宏是从内部开始展开的
#include <stdio.h>

// 定义宏：f(a,b) 将参数 a 和 b 拼接为一个标记（token）
//         g(a) 将参数 a 转换为字符串（stringify）
//         h(a) 间接调用 g(a)，用于说明先展开内层宏的效果
#define f(a,b) a##b    // 拼接参数 a 和 b
#define g(a) #a        // 将参数 a 字符串化（注意：# 运算符不会展开 a）
#define h(a) g(a)      // 间接调用 g 宏

void _main() {
    // 第一个 printf：h(f(1,2))
    //   先展开内层宏 f(1,2) -> 12，再将 12 作为参数传给 g -> "12"
    printf("%s\n", h(f(1,2)));
    // 第二个 printf：g(f(1,2))
    //   直接对 f(1,2) 做字符串化操作，不先展开 f -> "f(1,2)"
    printf("%s\n", g(f(1,2)));
}

// 定义INNER宏，展开时输出消息
#define INNER _Pragma("message \"Inner macro expanded\"")

// 定义OUTER宏，展开时输出消息并返回参数x
#define OUTER(x) _Pragma("message \"Outer macro expanded\"") x

OUTER(INNER);

*/

// #define foo foo a bar b bar baz c
// #define bar foo 12
// #define baz bar 13
// foo