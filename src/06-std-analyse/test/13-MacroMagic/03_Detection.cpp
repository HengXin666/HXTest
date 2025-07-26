#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,) // 要求只能传入一个参数
#define PROBE(x) x, 1,

/*
    CHECK(PROBE(~)) -> CHECK(~, 1,)
                    -> CHECK_N(~, 1, 0,)
                    -> 1
*/
auto res01 = CHECK(PROBE(~));  // Expands to 1

/*
    CHECK(xxx) -> CHECK(xxx)
               -> CHECK_N(xxx, 0,)
               -> 0
*/
auto res02 = CHECK(xxx);       // Expands to 0
auto hk_01 = CHECK(xxx, 2333); // Expands to 233

#define IS_PAREN_PROBE(...) PROBE(~)
#define IS_PAREN(x) CHECK(IS_PAREN_PROBE x)

/*
    IS_PAREN(()) -> CHECK(IS_PAREN_PROBE())
                 -> CHECK(PROBE(~)) -> 1
    否则, 等价于
                 -> CHECK(xxx) -> 0
*/
auto res03 = IS_PAREN(());      // Expands to 1
auto res04 = IS_PAREN(xxx);     // Expands to 0
auto hk_02 = IS_PAREN(((())));  // Expands to 1

#define ACTIVATION(x, y) x##y
#define NOT_0 PROBE(~)
#define NOT(x) CHECK(ACTIVATION(NOT_, x))
#define BOOL(x) NOT(NOT(x))

#define DO(...) __VA_ARGS__
#define NOT_DO(...)
#define IF_THEN(val) ACTIVATION(IF_THEN_, val)
#define IF_THEN_0(t, f) f
#define IF_THEN_1(t, f) t
#define IF(x) IF_THEN(BOOL(x))(DO, NOT_DO)

auto _ = IF_THEN(0)(1, 0);
int res05[1000] = { IF(0)(2233, 2322) };
int res06[1000] = { IF(1)(2233, 2322) };
int res07[1000] = { IF()(2233, 2322) }; // u

int main() { return 0; }