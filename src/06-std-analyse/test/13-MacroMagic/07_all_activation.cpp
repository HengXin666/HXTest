int main() {}

#define STR(x) #x
#define LOG_MACRO(x) _Pragma(STR(message("当前宏: " STR(x))))

///

#define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL5(...) __VA_ARGS__

///

#define EMPTY()
#define DELAY(code) code EMPTY()
#define NEXT_TIME(...) __VA_ARGS__ DELAY(EMPTY)()
#define SCAN_AGAIN(...) __VA_ARGS__

// 转发宏
#define FORWARD_IMPL(cb, ...) cb(__VA_ARGS__)
#define FORWARD(cb, ...) FORWARD_IMPL(cb, __VA_ARGS__)

// 拼接宏
#define ACTIVATION_IMPL_1(x, y) x##y
#define ACTIVATION(x, y) ACTIVATION_IMPL_1(x, y)

// 判断是否为空
#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0, )
#define IS_EMPTY() ~, 1
#define IF_EMPTY(x) CHECK(ACTIVATION(IS_EMPTY, x)())

LOG_MACRO(IF_EMPTY(2233))
LOG_MACRO(IF_EMPTY())

// 判断宏
#define IF_BOOL_0(t, f) f
#define IF_BOOL_1(t, f) t
#define IF_BOOL(val, t, f) ACTIVATION(IF_BOOL_, val)(t, f)

// 实现一个宏, 让 __VA_ARGS__ 全部连接
#define ACTIVATION_ALL_CALL_THIS() ACTIVATION_ALL_IMPL
#define ACTIVATION_ALL_0(x, ...) ACTIVATION(x,) DELAY(ACTIVATION_ALL_CALL_THIS)()(__VA_ARGS__)
#define ACTIVATION_ALL_1(...)
#define ACTIVATION_ALL_IMPL(x, ...) \
        ACTIVATION(ACTIVATION_ALL_, IF_EMPTY(x))(x, __VA_ARGS__)
#define ACTIVATION_ALL(...) EVAL(ACTIVATION_ALL_IMPL(__VA_ARGS__))

LOG_MACRO(ACTIVATION_ALL(awa, qwq, 0, 0))

#define GET_2(a, b) b

LOG_MACRO(GET_2(1, 2))                    // 2
LOG_MACRO(GET_2(ACTIVATION_ALL(1, 2), 3)) // 3

// 无法实现
// auto ACTIVATION_ALL(awa, qwq) = 1;