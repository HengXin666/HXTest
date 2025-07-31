#include <check/OpenGL.hpp> // 包括 glad/glad.h
#include <GLFW/glfw3.h>     // 必须放在 glad/glad.h 后面
#include <HXprint/print.h> // 兼容

constexpr int Fineness = 120;
constexpr float pi = 3.1415926535897f;

/**
 * @brief 通过度数范围计算为对应百分比
 * @param du [0, 360]
 * @return constexpr int 
 */
constexpr int getShowRoundPercentage(float du) {
    return static_cast<int>(Fineness * (du / 360.f));
}

/**
 * @brief 画一个圆, 位于 (x, y), 半径是 r, 环半径是 r - dr
 * @tparam IsCross 是否越过 360 -> 0 的边界
 * @param x [-1, 1]
 * @param y [-1, 1]
 * @param r [-1, 1]
 * @param dr [-1, 1]
 * @param noShowL 不渲染的范围起点 [0, Fineness]
 * @param noShowR 不渲染的范围终点 [0, Fineness]
 */
 template <bool IsCross = false>
void showRound(
    float x, float y,
    float r, float dr,
    int noShowL = Fineness, int noShowR = Fineness
) {
    for (int i = 0; i < Fineness; ++i) {
        if constexpr (IsCross) {
            if (i <= noShowL) {
                i = noShowL;
            } else if (i >= noShowR) {
                break;
            }
        } else {
            if (i >= noShowL && i <= noShowR) {
                i = noShowR;
            }
        }

        float angle = (float)i / (float)Fineness * pi * 2;
        float angleNext = (float)(i + 1) / (float)Fineness * pi * 2;

        glVertex3f(dr * std::sin(angle) + x, dr * std::cos(angle) + y, 0.f);
        glVertex3f(r * std::sin(angle) + x, r * std::cos(angle) + y, 0.f);
        glVertex3f(r * std::sin(angleNext) + x, r * std::cos(angleNext) + y, 0.f);

        glVertex3f(dr * std::sin(angle) + x, dr * std::cos(angle) + y, 0.f);
        glVertex3f(dr * std::sin(angleNext) + x, dr * std::cos(angleNext) + y, 0.f);
        glVertex3f(r * std::sin(angleNext) + x, r * std::cos(angleNext) + y, 0.f);
    }
}

void show() {
    glBegin(GL_TRIANGLES);
    constexpr float r = 0.3f, dr = 0.15f;
    float xDown = std::sqrt(3.f) * 0.25f;
    glColor3f(1.f, 0.f, 0.f);
    showRound(0.f, 0.5f, r, dr, 
        getShowRoundPercentage(150), getShowRoundPercentage(210));

    glColor3f(0.f, 1.f, 0.f);
    showRound(-xDown, -0.25f, r, dr, 
        getShowRoundPercentage(30), getShowRoundPercentage(90));

    glColor3f(0.f, 0.f, 1.f);
    showRound<true>(xDown, -0.25f, r, dr,
        getShowRoundPercentage(30), getShowRoundPercentage(330));
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