#include <check/OpenGL.hpp> // 包括 glad/glad.h

#include <vector>

struct Vertex {
    float x, y, z;
};

std::vector<Vertex> vertices = {
    {0.5, 0.5, 0},
    {0.5, -0.5, 0},
    {-0.5, 0.5, 0},
    {-0.5, -0.5, 0},
};

struct Triangle {
    std::size_t a, b, c;
};

std::vector<Triangle> triangles = {
    {0, 1, 2},
    {3, 2, 1},
};

void show() {
    glBegin(GL_TRIANGLES);
    for (auto const& v : triangles) {
        // 从三个参数的 glVertex3f 改成一个指针参数的 glVertex3fv. 少写点代码
        // glVertex3fv 接受一个 float * 指针作为参数, 会加载指针所指向的内存中连续的三个 float
        glVertex3fv(&vertices[v.a].x);
        glVertex3fv(&vertices[v.b].x);
        glVertex3fv(&vertices[v.c].x);
    }
    CHECK_GL(glEnd());
}

void mouseBtnCb(GLFWwindow* win, int btn, int action, [[maybe_unused]] int mods) {
    // 判断是鼠标 && 左键按下
    if (btn == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        int width, height;
        glfwGetCursorPos(win, &xpos, &ypos);
        glfwGetWindowSize(win,  &width, &height);

        auto x = static_cast<float>(2 * xpos / width - 1);
        auto y = static_cast<float>(2 * (height - ypos) / height - 1);
        vertices[2] = {x, y, 0};
    }
}

int main() {
    auto* window = initOpenGL();
    log::hxLog.debug("OpenGL version: ", glGetString(GL_VERSION));

    CHECK_GL(glEnable(GL_POINT_SMOOTH));
    CHECK_GL(glEnable(GL_BLEND));
    CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    CHECK_GL(glPointSize(64.0f));

    glfwSetMouseButtonCallback(window, mouseBtnCb); // 设置鼠标事件回调
    while (!glfwWindowShouldClose(window)) {
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT)); // 清空画布
        show();
        glfwSwapBuffers(window); // 双缓冲
        glfwPollEvents();
    }
    return 0;
}