#define THE_IF_THEN(cb) THE_IF_THEN_ ## cb
#define THE_IF_THEN_0(t, f) f
#define THE_IF_THEN_1(t, f) t

#define FUNC() 0
// 这正确地扩展为 true
auto res1 = THE_IF_THEN(1)(true, false);

// 但是，这将扩展为 IF_THEN_A()(true， false)
// 这是因为 A() 不会扩展到 1,
// 因为它被 ## 运算符抑制
// auto res2 = IF_THEN(FUNC())(true, false);

#define ACTIVATION(x, y) x##y

#define IF_THEN(cb) ACTIVATION(IF_THEN_, cb)
#define IF_THEN_0(t, f) f
#define IF_THEN_1(t, f) t

#define LAZY_CODE(x) IF_THEN(FUNC_(x))(true, false)

auto res_1 = IF_THEN(1)(true, false);
auto res_2 = IF_THEN(FUNC())(true, false);

#define FUNC_(x) 0
auto res_3 = IF_THEN(FUNC_(0))(true, false);
auto res_4 = LAZY_CODE(2233);

#define WDF(x) ACTIVATION(IF_THEN_, FUNC_(x))

// auto res5 = WDF(2233);

int main() { return 0; }