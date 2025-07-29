int main() {}

#define STR(x) #x
#define LOG_MACRO(x) _Pragma(STR(message("当前宏: " STR(x))))

#define JOIN_IMPL(x, y) x##y
#define JOIN(x, y) JOIN_IMPL(x, y)

// 判断是否为空
#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0, )
#define IS_EMPTY_(...) ~, 1
#define IF_EMPTY(...) CHECK(JOIN(IS_EMPTY, _ __VA_ARGS__))

#define VA_OPT(...) IS_EMPTY(__VA_ARGS__)

// 好像不行...

LOG_MACRO(IF_EMPTY())