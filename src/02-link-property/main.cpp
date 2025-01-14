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
    fun();
    printf("main -> i: %d\n", ++i);
    printf("main看到的 %p\n", &look);
    look("main");
    look("xxx");
    myShow();

    std::cout << C{}.s.size() << '\n';

    return 0;
}
