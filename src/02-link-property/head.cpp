#include "head.h"

struct C {
    int s;
};

inline void myShow() {
    printf("在.cpp文件\n");
}

void fun1() {
    printf("fun -> i: %d\n", ++i);
    printf("fun看到的: %p\n", (void *)&look);
    look("fun");
    look("yyy");
    myShow();

    std::cout << "func: " <<  &abcBool << '\n';
}
