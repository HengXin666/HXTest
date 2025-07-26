#define COMPARE_foo(x) x
#define COMPARE_bar(x) x

#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,) // 要求只能传入一个参数
#define PROBE(x) x, 1,

#define IS_PAREN_PROBE(...) PROBE(~)
#define IS_PAREN(x) CHECK(IS_PAREN_PROBE x)

#define PRIMITIVE_COMPARE(x, y) IS_PAREN \
( \
COMPARE_ ## x ( COMPARE_ ## y) (())  \
)

auto res01 = PRIMITIVE_COMPARE(foo, bar); // Expands to 1
auto res02 = PRIMITIVE_COMPARE(foo, foo); // Expands to 0

// 需要定义 COMPARE_???, 否则实际上是报错的:
// PRIMITIVE_COMPARE(foo, unfoo)

int main() {}

#define ACTIVATION(x, y) x##y
#define CALL_ACTIVATION(x, y) ACTIVATION(x, y)

#define IF_THEN(cb) ACTIVATION(IF_THEN_, cb)
#define IF_THEN_0(t, f) f
#define IF_THEN_1(t, f) t

#define IS_COMPARABLE(x) IS_PAREN( CALL_ACTIVATION(COMPARE_, x) (()) )

#define BITAND_0(y) 0
#define BITAND_1(y) y
#define BITAND(x) ACTIVATION(BITAND_, x)

#define EAT(...)
#define NOT_EQUAL(x, y) \
IF_THEN(BITAND(IS_COMPARABLE(x))(IS_COMPARABLE(y)) ) \
( \
   PRIMITIVE_COMPARE, \
   1 EAT \
)(x, y)

#define COMPL_0 1
#define COMPL_1 0
#define COMPL(b) ACTIVATION(COMPL_, b)

#define EQUAL(x, y) COMPL(NOT_EQUAL(x, y))

auto res03 = EQUAL(foo, bar); // 0
auto res04 = EQUAL(foo, foo); // 1
auto res05 = EQUAL(foo, abc); // 0