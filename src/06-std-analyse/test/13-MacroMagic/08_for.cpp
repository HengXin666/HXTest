int main() {}

#define STR(x) #x
#define LOG_MACRO(x) _Pragma(STR(message("当前宏: " STR(x))))

// 展开宏
#define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL5(...) __VA_ARGS__

// 延迟展开宏
#define EMPTY()
#define DELAY(code) code EMPTY()

// 拼接宏
#define ACTIVATION_IMPL_1(x, y) x##y
#define ACTIVATION(x, y) ACTIVATION_IMPL_1(x, y)

// 判断是否为空
#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0, )
#define IS_EMPTY() ~, 1
#define IF_EMPTY(x) CHECK(ACTIVATION(IS_EMPTY, x)())

// for
#define FOR_IMPL_CALL_THIS() FOR_IMPL
#define FOR_IMPL_0(macro, x, ...) macro(x) DELAY(FOR_IMPL_CALL_THIS)()(macro, __VA_ARGS__)
#define FOR_IMPL_1(...)
#define FOR_IMPL(macro, x, ...) ACTIVATION(FOR_IMPL_, IF_EMPTY(x))(macro, x, __VA_ARGS__)
#define FOR(macro, x, ...) EVAL(FOR_IMPL(macro, x, __VA_ARGS__))

// test
#define DECLARED_MEMBER(name) decltype(name) name;

struct A {
    void func() {
        struct __my_a__ {
            FOR(DECLARED_MEMBER, op, str)
            LOG_MACRO(FOR(DECLARED_MEMBER, op, str))
        };
    }

    int op;
    char* str;
};