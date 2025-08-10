#include "head.h"

struct C {
    int v;
    std::string s;

    C() = default;
};

inline void myShow() {
    printf("在main文件\n");
}

int main() {
    fun1();
    printf("main -> i: %d\n", ++i);
    printf("main看到的 %p\n", (void *)&look);
    look("main");
    look("xxx");
    myShow();

    std::cout << C{}.s.size() << '\n';

    std::cout << "main: " << &abcBool << '\n';
    return 0;
}
