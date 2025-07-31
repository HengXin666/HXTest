#include <check/OpenGL.hpp> // 包括 glad/glad.h
#include <GLFW/glfw3.h>     // 必须放在 glad/glad.h 后面

#include <vector>

struct Vertex {
    float x, y, z;
};

std::vector<Vertex> vertices;

void show() {
    glBegin(GL_TRIANGLES);
    for (auto const& v : vertices)
        glVertex3f(v.x, v.y, v.z);
    CHECK_GL(glEnd());
}

void mouseBtnCb(GLFWwindow* win, int btn, int action, [[maybe_unused]] int mods) {
    // 判断是鼠标 && 左键按下
    if (btn == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        int width, height;
        // 获取鼠标坐标
        glfwGetCursorPos(win, &xpos, &ypos);
        // 获取窗口大小
        glfwGetWindowSize(win,  &width, &height);

        // 映射到 [-1, 1], 除以对应的宽、高, 以保证宽高比
        auto x = static_cast<float>(2 * xpos / width - 1);
        // 注意因为窗口坐标是左上原点, y 正轴向下, 所以 转换需要 height - ypos
        auto y = static_cast<float>(2 * (height - ypos) / height - 1);
        // 比如 xpos / width 是把 [0, width] -> [0, 1]
        // *2 - 1 是把 [0, 1] -> [-1, 1]
        vertices.push_back({x, y, 0});
    }
}

int main() {
    if (!glfwInit()) {
        throw std::runtime_error("failed to initialize GLFW");
    }
    auto* window = glfwCreateWindow(960, 720, "Example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("GLFW failed to create window");
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL()) {
        glfwTerminate();  // 由于 glfwInit 在前, 理论上是需要配套的 glfwTerminate 防止泄漏
        throw std::runtime_error("GLAD failed to load GL functions");
    }
    log::hxLog.debug("OpenGL version: ", glGetString(GL_VERSION)); // 初始化完毕, 打印一下版本号

    CHECK_GL(glEnable(GL_POINT_SMOOTH));
    CHECK_GL(glEnable(GL_BLEND)); // BLEND 的原理我们之后的课程讲解！
    CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    CHECK_GL(glPointSize(64.0f));

    glfwSetMouseButtonCallback(window, mouseBtnCb); // 设置鼠标事件回调
    while (!glfwWindowShouldClose(window)) {
        show();
        glfwSwapBuffers(window); // 双缓冲
        glfwPollEvents();
    }
    return 0;
}