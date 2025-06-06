#include <check/OpenGL.hpp> // 包括 glad/glad.h
#include <GLFW/glfw3.h>     // 必须放在 glad/glad.h 后面

void show() {
    glBegin(GL_TRIANGLES);
    constexpr int n = 100;
    constexpr float pi = 3.1415926535897f;
    float r = 0.5f, dr = 0.3f;
    static int x = 0;
    if (++x > n)
        x %= n;
    for (int i = 0; i < x; ++i) {
        float angle = (float)i / (float)n * pi * 2;
        float angleNext = (float)(i + 1) / (float)n * pi * 2;

        glVertex3f(dr * std::sin(angle), dr * std::cos(angle), 0.f);
        glVertex3f(r * std::sin(angle), r * std::cos(angle), 0.f);
        glVertex3f(r * std::sin(angleNext), r * std::cos(angleNext), 0.f);

        glVertex3f(dr * std::sin(angle), dr * std::cos(angle), 0.f);
        glVertex3f(dr * std::sin(angleNext), dr * std::cos(angleNext), 0.f);
        glVertex3f(r * std::sin(angleNext), r * std::cos(angleNext), 0.f);
    }
    CHECK_GL(glEnd());
}

int main() {
    if (!glfwInit()) {
        throw std::runtime_error("failed to initialize GLFW");
    }
    auto* window = glfwCreateWindow(640, 640, "Example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("GLFW failed to create window");
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL()) {
        glfwTerminate();  // 由于 glfwInit 在前, 理论上是需要配套的 glfwTerminate 防止泄漏
        throw std::runtime_error("GLAD failed to load GL functions");
    }
    HX::print::println("OpenGL version: ", glGetString(GL_VERSION)); // 初始化完毕, 打印一下版本号

    CHECK_GL(glEnable(GL_POINT_SMOOTH));
    CHECK_GL(glEnable(GL_BLEND)); // BLEND 的原理我们之后的课程讲解！
    CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    CHECK_GL(glPointSize(3.0f));

    while (!glfwWindowShouldClose(window)) {
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
        show();
        glfwSwapBuffers(window); // 双缓冲
        glfwPollEvents();
    }
    return 0;
}