#include <check/OpenGL.hpp> // 包括 glad/glad.h

#include <vector>

struct Vertex {
    float x, y, z;
};

struct Triangle {
    Vertex a, b, c;
};

std::vector<Triangle> triangles = {
    {
        {0.5, 0.5, 0},
        {0.5, -0.5, 0},
        {-0.5, 0.5, 0},
    },
    {
        {-0.5, -0.5, 0},
        {-0.5, 0.5, 0},
        {0.5, -0.5, 0},
    },
};

void show() {
    glBegin(GL_TRIANGLES);
    for (auto const& v : triangles) {
        glVertex3f(v.a.x, v.a.y, v.a.z);
        glVertex3f(v.b.x, v.b.y, v.b.z);
        glVertex3f(v.c.x, v.c.y, v.c.z);
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
        triangles[0].c = {x, y, 0};
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