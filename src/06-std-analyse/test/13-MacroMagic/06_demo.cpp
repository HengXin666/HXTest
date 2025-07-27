#define STR(x) #x
#define LOG_MACRO(x) _Pragma(STR(message("当前宏: " STR(x))))

///

#define EMPTY()
#define DELAY(code) code EMPTY()
#define NEXT_TIME(...) __VA_ARGS__ DELAY(EMPTY)()
#define SCAN_AGAIN(...) __VA_ARGS__

#define A() 123

LOG_MACRO(A())
LOG_MACRO(DELAY(A)())
LOG_MACRO(NEXT_TIME(A()))
LOG_MACRO(SCAN_AGAIN(NEXT_TIME(A())))
LOG_MACRO(SCAN_AGAIN(DELAY(A())))

#define SCAN_AGAIN_ALL(...) SCAN_AGAIN_ALL_IMPL(__VA_ARGS__)
#define SCAN_AGAIN_ALL_IMPL(...) SCAN_1(SCAN_1(SCAN_1(__VA_ARGS__)))
#define SCAN_1(...) SCAN_2(SCAN_2(SCAN_2(SCAN_2(SCAN_2(__VA_ARGS__)))))
#define SCAN_2(...) SCAN_3(SCAN_3(SCAN_3(SCAN_3(SCAN_3(__VA_ARGS__)))))
#define SCAN_3(...) SCAN_4(SCAN_4(SCAN_4(SCAN_4(SCAN_4(__VA_ARGS__)))))
#define SCAN_4(...) __VA_ARGS__

#define MY_NEXT_CALL_THIS() MY_NEXT
#define MY_NEXT(code, x, ...) code(x) DELAY(MY_NEXT_CALL_THIS)()(code, __VA_ARGS__)

#define ECHO_NUM(v) v
LOG_MACRO(MY_NEXT(ECHO_NUM, 1, 2, 3, 4))
LOG_MACRO(SCAN_AGAIN(MY_NEXT(ECHO_NUM, 1, 2, 3, 4)))
LOG_MACRO(SCAN_AGAIN(SCAN_AGAIN(MY_NEXT(ECHO_NUM, 1, 2, 3, 4))))
LOG_MACRO(SCAN_AGAIN(SCAN_AGAIN(SCAN_AGAIN(MY_NEXT(ECHO_NUM, 1, 2, 3, 4)))))

#define FORWARD_IMPL(cb, x, y) cb(x, y)
#define FORWARD(cb, x, y) FORWARD_IMPL(cb, x, y)
#define ACTIVATION(x, ...) x##__VA_ARGS__
#define ACTIVATION_ALL_CALL_THIS ACTIVATION_ALL
#define ACTIVATION_ALL(x, ...)                                                 \
    SCAN_AGAIN_ALL(                                                            \
        FORWARD(ACTIVATION, x, DELAY(FOR_IMPL_CALL_THIS)()(__VA_ARGS__)))

#define TO_NULL(...)

// 如果参数为空, 就不展开
#define CHECK_N(x, n, ...) n 
#define CHECK(...) CHECK_N(__VA_ARGS__, 0, )

#define IF_EMPTY_TRUE() ~, 1
#define IF_EMPTY(...) CHECK(ACTIVATION_ALL(IF_EMPTY_, __VA_ARGS__ TRUE))

LOG_MACRO(ACTIVATION_ALL(IF_EMPTY_, 1, 2, 3, TRUE))

// 判断
#define IF_0(t, f) f
#define IF_1(t, f) t
#define IF(val, t, f) ACTIVATION(IF_, val)(t, f)

#define FOR_IMPL_CALL_THIS() FOR_IMPL
#define FOR_IMPL(code, x, ...) IF(IF_EMPTY(x), TO_NULL(), code(x) DELAY(FOR_IMPL_CALL_THIS)()(code, __VA_ARGS__))
#define FOR(code, v, ...) SCAN_AGAIN_ALL(FOR_IMPL(code, v, __VA_ARGS__))

LOG_MACRO(IF_EMPTY())

LOG_MACRO(IF(IF_EMPTY(), TO_NULL(), 非空))

LOG_MACRO(FOR(ECHO_NUM, 1, 2, 3, 4))

int main() {}