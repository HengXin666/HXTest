#include <check/OpenGL.hpp> // 包括 glad/glad.h
#include <GLFW/glfw3.h>     // 必须放在 glad/glad.h 后面

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
 * @tparam IsClockwise 是否是顺时针
 * @param x [-1, 1]
 * @param y [-1, 1]
 * @param r [-1, 1]
 * @param dr [-1, 1]
 * @param step 渲染步数
 * @param begin 渲染起始位置
 * @param end 渲染结束位置 [0, Fineness]
 */
 template <bool IsClockwise>
void showRound(
    float x, float y,
    float r, float dr,
    int step,
    int begin, int end
) {
    constexpr auto mod = Fineness + 1;
    for (int i = begin; step && i != end; i = (i + (2 * IsClockwise - 1) + mod) % mod, --step) {
        float angle = static_cast<float>(i) / Fineness * pi * 2;
        float angleNext = static_cast<float>(i + 1) / Fineness * pi * 2;

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

    static int cntR = 0, cntG = 0, cntB = 0, tmp = 0;

    glColor3f(1.f, 0.f, 0.f);
    showRound<false>(0.f, 0.5f, r, dr, cntR,
        getShowRoundPercentage(150), getShowRoundPercentage(210));

    glColor3f(0.f, 1.f, 0.f);
    showRound<false>(-xDown, -0.25f, r, dr, cntG,
        getShowRoundPercentage(30), getShowRoundPercentage(90));

    glColor3f(0.f, 0.f, 1.f);
    showRound<false>(xDown, -0.25f, r, dr, cntB,
        getShowRoundPercentage(330), getShowRoundPercentage(30));

    constexpr int MaxStep = static_cast<int>(Fineness / 360.f * (360 - 60));

    if (++cntR > MaxStep) {
        if (++cntG > MaxStep) {
            if (++cntB > MaxStep) {
                if (++tmp > MaxStep) {
                    cntR = cntG = cntB = tmp = 0;
                }
            }
        }
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