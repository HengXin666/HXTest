// 增加宏展开扫描次数
#define E4(...) E3(E3(E3(E3(E3(E3(E3(E3(E3(E3(__VA_ARGS__))))))))))
#define E3(...) E2(E2(E2(E2(E2(E2(E2(E2(E2(E2(__VA_ARGS__))))))))))
#define E2(...) E1(E1(E1(E1(E1(E1(E1(E1(E1(E1(__VA_ARGS__))))))))))
#define E1(...) __VA_ARGS__

#define EMPTY()
#define TUPLE_AT_2(x, y, ...) y
#define TUPLE_TAIL(x, ...) __VA_ARGS__

#define CHECK(...) TUPLE_AT_2(__VA_ARGS__, 0, )
#define EQ_END_END , 1

#define SCAN(...) __VA_ARGS__

// 定义拼接宏
#define CAT_(a, b) a##b
#define CAT(a, b) CAT_(a, b)

#define LOOP_() LOOP
#define LOOP(x, y, ...) CAT(LOOP, CHECK(EQ_END_##y))(x, y, __VA_ARGS__)
#define LOOP1(x, ...) TUPLE_TAIL x
#define LOOP0(x, y, ...) LOOP_ EMPTY()()((SCAN x, y), __VA_ARGS__)

#define DTC(...) E4(LOOP((), __VA_ARGS__ END))

int arr[] = {DTC(1, 2, 3, 4, 5, 6, 7, 8, 9, )};
// expands to: 1, 2, 3, 4, 5, 6, 7, 8, 9
