#define EMPTY()
#define DEFER(id) id EMPTY()
#define OBSTRUCT(...) __VA_ARGS__ DEFER(EMPTY)()

#define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL5(...) __VA_ARGS__

#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,)
#define PROBE(x) x, 1,
#define ACTIVATION(x, y) x##y
#define NOT_0 PROBE(~)
#define NOT(x) CHECK(ACTIVATION(NOT_, x))
#define BOOL(x) NOT(NOT(x))
#define DO(...) __VA_ARGS__
#define NOT_DO(...)
#define IF_THEN(val) ACTIVATION(IF_THEN_, val)
#define IF_THEN_0(t, f) f
#define IF_THEN_1(t, f) t
#define WHEN(x) IF_THEN(BOOL(x))(DO, NOT_DO)

#define DEC(x) ACTIVATION(DEC_, x)
#define DEC_0 0
#define DEC_1 0
#define DEC_2 1
#define DEC_3 2
#define DEC_4 3
#define DEC_5 4
#define DEC_6 5
#define DEC_7 6
#define DEC_8 7

#define REPEAT_INDIRECT() REPEAT_IMPL
#define REPEAT(count, macro, ...) EVAL(REPEAT_IMPL(count, macro, __VA_ARGS__))

#define REPEAT_IMPL(count, macro, ...) \
    WHEN(count) \
    ( \
        OBSTRUCT(REPEAT_INDIRECT) () \
        ( \
            DEC(count), macro, __VA_ARGS__ \
        ) \
        OBSTRUCT(macro) \
        ( \
            DEC(count), __VA_ARGS__ \
        ) \
    )

#define M(i, _) i,

int res[] = {
    REPEAT(8, M, ~) // 0, 1, 2, 3, 4, 5, 6, 7,
};

#define WHILE(pred, op, ...) \
    IF(pred(__VA_ARGS__)) \
    ( \
        OBSTRUCT(WHILE_INDIRECT) () \
        ( \
            pred, op, op(__VA_ARGS__) \
        ), \
        __VA_ARGS__ \
    )
#define WHILE_INDIRECT() WHILE

int main() {}