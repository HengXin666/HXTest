#include <check/OpenGL.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <stdexcept>
#include <vector>

#include <HXTest.hpp>

auto __init__ = []{
    setlocale(LC_ALL, "zh_CN.UTF-8");
    try {
        auto cwd = std::filesystem::current_path();
        log::hxLog.debug("当前工作路径是:", cwd);
        std::filesystem::current_path("../../../src/08-OpenGL");
        log::hxLog.debug("切换到路径:", std::filesystem::current_path());
    } catch (const std::filesystem::filesystem_error& e) {
        log::hxLog.error("Error:", e.what());
    }
    return 0;
}();

namespace HX {

// 摄像机类
struct CameraState {
    glm::vec3 eye = {0, 0, 5};        // 相机坐标
    glm::vec3 lookat = {0, 0, 0};     // 相机看向的目标点
    glm::vec3 upVector = {0, 1, 0};   // 上方向 (相机本地坐标系的y正方向(相机屏幕的上方向))
    glm::vec3 keepUpAxis = {0, 1, 0};
    float focalLen = 40.0f;
    float filmHeight = 24.0f;
    float filmWidth = 32.0f;
    int width = 1920;
    int height = 1080;

    // 环绕模式: 环绕着目标物体旋转, 所以 orbit 的旋转枢轴是 lookat;
    void orbit(glm::vec2 delta) {
        // 获取鼠标的水平和竖直分量
        auto angle_X_inc = delta.x;
        auto angle_Y_inc = delta.y;

        // 旋转原点
        auto rotation_pivot = lookat; // 环绕着目标物体
        // 指向目标点的单位方向向量
        auto front_vector = glm::normalize(lookat - eye);
        // 右向量 (指向目标点的单位方向向量 和 上方向 的叉积)
        auto right_vector = glm::normalize(
            glm::cross(front_vector, upVector));

        // 新的上方向是 右向量 和 指向目标点的单位方向向量 的叉积
        upVector = glm::normalize(glm::cross(right_vector, front_vector));
/*
        front_vector(指向目标点方向向量) 和 upVector(相机本地坐标系的y正方向向量) 是共面的
        对其叉积, 得到 右向量
        然后 右向量 再和 front_vector(指向目标点方向向量) 叉积
        得到新的 upVector(相机本地坐标系的y正方向向量)

        此时 front_vector(x)、upVector(y)、右向量(z) 互相垂直, 形成空间直角坐标系
*/
        // 第一次旋转: 鼠标左右 对应是 照相机本地坐标系 以 y 轴 "旋转"
        glm::mat4x4 rotation_matrixX = glm::rotate(glm::mat4x4(1), -angle_X_inc, upVector);
        // 第二次旋转: 鼠标上下 对应是 照相机本地坐标系 以 z 轴 "旋转" (俯仰)
        glm::mat4x4 rotation_matrixY = glm::rotate(glm::mat4x4(1), angle_Y_inc, right_vector);

        // 先平移到 rotation_pivot 为中心的坐标系, 绕中心旋转, 然后平移回来(经典 T R T^{-1})
        auto transformation
            = glm::translate(glm::mat4x4(1), rotation_pivot)
            * rotation_matrixY * rotation_matrixX
            * glm::translate(glm::mat4x4(1), -rotation_pivot);

        // 更新眼睛和物体的位置坐标(其实只需要更新眼睛就行, lookat 作为枢轴是不会变的)
        eye = glm::vec3(transformation * glm::vec4(eye, 1));
        lookat = glm::vec3(transformation * glm::vec4(lookat, 1));

        // 计算出新的上矢量和最初的上矢量偏移了多少(通过点积求重合程度)
        float right_o_up = glm::dot(right_vector, keepUpAxis); // "右"点积"旧上"
        float right_handness = glm::dot(glm::cross(keepUpAxis, right_vector), front_vector);
        // "右"和"旧上"点积的 acos 是"右"和"旧上"的夹角, asin 是"新上"和"旧上"的夹角
        float angle_Z_err = glm::asin(right_o_up);
        // 如果摄像头整个都已经颠倒了, 那么修正也往颠倒的下方向去修正而不是试图180度大转弯
        angle_Z_err *= glm::atan(right_handness);
        // 把上矢量(up)绕着前矢量(front)旋转误差的度数, 就可以一直保持摄像头朝上, 不歪脖子
        glm::mat4x4 rotation_matrixZ = glm::rotate(glm::mat4x4(1), angle_Z_err, front_vector);
        upVector = glm::mat3x3(rotation_matrixZ) * upVector;
    }

    // drift(转头模式): 摄像头自身在旋转, 所以 drift 的旋转枢轴是 eye
    void drift(glm::vec2 delta) {
        // 而且由于鼠标右移是用户要把物体向右移动, 需要反方向向左旋转摄像头, 所以 delta 要翻转方向(乘以 -1)
        delta *= -1.0f;
        delta *= std::atan(filmHeight / (2 * focalLen));

        // 获取鼠标的水平和竖直分量
        auto angle_X_inc = delta.x;
        auto angle_Y_inc = delta.y;

        // 旋转原点
        auto rotation_pivot = eye; // 环绕着摄像机
        // 指向目标点的单位方向向量
        auto front_vector = glm::normalize(lookat - eye);
        // 右向量 (指向目标点的单位方向向量 和 上方向 的叉积)
        auto right_vector = glm::normalize(
            glm::cross(front_vector, upVector));

        // 新的上方向是 右向量 和 指向目标点的单位方向向量 的叉积
        upVector = glm::normalize(glm::cross(right_vector, front_vector));

        // 第一次旋转: 基于鼠标横轴
        glm::mat4x4 rotation_matrixX = glm::rotate(glm::mat4x4(1), -angle_X_inc, upVector);
        // 第二次旋转: 基于鼠标纵轴
        glm::mat4x4 rotation_matrixY = glm::rotate(glm::mat4x4(1), angle_Y_inc, right_vector);

        // 先平移到 rotation_pivot 为中心的坐标系, 绕中心旋转, 然后平移回来(经典 T R T^{-1})
        auto transformation = glm::translate(glm::mat4x4(1), rotation_pivot)
            * rotation_matrixY * rotation_matrixX
            * glm::translate(glm::mat4x4(1), -rotation_pivot);

        // 更新眼睛和物体的位置坐标(其实只需要更新眼睛就行, lookat 作为枢轴是不会变的)
        eye = glm::vec3(transformation * glm::vec4(eye, 1));
        lookat = glm::vec3(transformation * glm::vec4(lookat, 1));

        // 计算出新的上矢量和最初的上矢量偏移了多少(通过点积求重合程度)
        float right_o_up = glm::dot(right_vector, keepUpAxis); // "右"点积"旧上"
        float right_handness = glm::dot(glm::cross(keepUpAxis, right_vector), front_vector);
        // "右"和"旧上"点积的 acos 是"右"和"旧上"的夹角, asin 是"新上"和"旧上"的夹角
        float angle_Z_err = glm::asin(right_o_up);
        // 如果摄像头整个都已经颠倒了, 那么修正也往颠倒的下方向去修正而不是试图180度大转弯
        angle_Z_err *= glm::atan(right_handness);
        // 把上矢量(up)绕着前矢量(front)旋转误差的度数, 就可以一直保持摄像头朝上, 不歪脖子
        glm::mat4x4 rotation_matrixZ = glm::rotate(glm::mat4x4(1), angle_Z_err, front_vector);
        upVector = glm::mat3x3(rotation_matrixZ) * upVector;
    }

    // pan(平移模式)
    void pan(glm::vec2 delta) {
        delta *= -2.f;

        // 从摄像头到目标的方向向量 (屏幕正方向)
        auto front_vector = glm::normalize(lookat - eye);
        auto right_vector = glm::normalize(
            glm::cross(front_vector, upVector));
        auto fixed_up_vector = glm::normalize(
            glm::cross(right_vector, front_vector));

        auto delta3d = delta.x * right_vector + delta.y * fixed_up_vector;

        eye += delta3d;
        lookat += delta3d;
    }

    // zoom(缩放模式)
    void zoom(float delta) {
        float inv_zoom_factor = glm::exp(-0.2f * delta);
        eye = (eye - lookat) * inv_zoom_factor + lookat;
    }

    // hitchcock(变焦模式)
    void hitchcock(float delta) {
        float inv_zoom_factor = glm::exp(-0.2f * delta);
        eye = (eye - lookat) * inv_zoom_factor + lookat;
        focalLen *= inv_zoom_factor;
    }

    // 相机 视角矩阵
    glm::mat4x4 view_matrix() const {
        return glm::lookAt(
            eye,            // 相机坐标点
            lookat, // 相机"看向"的目标点(视线穿过的点)
            upVector    // "上"方向的参考向量, 用来确定相机的旋转朝向(通常是世界坐标里的近似全局向上, 比如 (0,1,0) )
        );
    }

    // 投影矩阵
    glm::mat4x4 projection_matrix() {
        auto fov = 2 * std::atan(filmHeight / (2 * focalLen));
        auto aspect = (float)width / (float)height;
        return glm::perspective(fov, aspect, 0.01f, 100.0f);
    }
};

// 按键状态
struct MouseState {
    bool leftClickPress = false;
    bool rightClickPress = false;
};

// 获取鼠标当前位置
glm::vec2 get_cursor_pos(GLFWwindow* window) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float x = (float)(2 * xpos / width - 1);
    float y = (float)(2 * (height - ypos) / height - 1);
    return glm::vec2(x, y);
}

struct ObjParser {
    void parser(std::string_view path) {
        std::ifstream file{{path.data(), path.size()}};
        if (!file.is_open()) [[unlikely]] {
            log::hxLog.error("打开文件:", path, "失败!");
            throw std::runtime_error{path.data()};
        }
        std::string line;
        while (std::getline(file, line)) {
            auto head = line.substr(0, 2);
            if (head == "v ") {
                // 解析顶点
                std::istringstream s{line.substr(2)};
                glm::vec3 v;
                s >> v.x >> v.y >> v.z;
                _vertices.push_back(std::move(v));
            } else if (head == "f ") {
                // 解析面
                std::istringstream s{line.substr(2)};
                std::vector<std::size_t> idx;
                while (std::getline(s, head, ' ')) {
                    std::size_t i;
                    std::istringstream{head} >> i;
                    idx.push_back(i - 1);
                }
                for (std::size_t i = 2; i < idx.size(); ++i)
                    _trinales.push_back({idx[0], idx[i], idx[i - 1]});
            }
        }

        file.close();
        log::hxLog.info("加载:", path, "完成!");
    }

    auto& getVertices() const noexcept {
        return _vertices;
    }

    auto& getFaces() const noexcept {
        return _trinales;
    }

private:
    std::vector<glm::vec3> _vertices;
    std::vector<glm::uvec3> _trinales;
};

} // namespace HX


CameraState camera;
MouseState mouse;
glm::vec2 lastpos; // 当前鼠标位置

// 手动计算法线
glm::vec3 compute_normal(glm::vec3 a, glm::vec3 b, glm::vec3 c) noexcept {
    auto ab = b - a;
    auto ac = c - a;
    return glm::normalize(glm::cross(ac, ab));
}

// 手动计算法线 asin 系数
glm::vec3 compute_normal_biased(glm::vec3 a, glm::vec3 b, glm::vec3 c) noexcept {
    auto ab = b - a;
    auto ac = c - a;
    auto n = glm::cross(ac, ab);
    auto nLen = glm::length(n);
    if (nLen != 0) {
        n *= glm::asin(nLen / (glm::length(ab) * glm::length(ac))) / nLen;
    }
    return n;
}

template <bool IsSmooth = false> // 是否为平滑模式
void show(GLFWwindow* window, glm::mat4x4 model) {
    static auto obj = [] {
        ObjParser res;
        res.parser("./obj/opencvpart.obj");
        return res;
    }();
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    glm::mat4x4 perspective = camera.projection_matrix();

    auto& vertices = obj.getVertices();
    static auto normals = [&] {
        std::vector<glm::vec3> res;
        if constexpr (IsSmooth) {
            auto& faces = obj.getFaces();
            res.resize(faces.size());
            for (auto const& v : faces) {
                auto a = vertices[v[0]],
                     b = vertices[v[1]],
                     c = vertices[v[2]];
                HX_NO_WARNINGS_BEGIN
                for (std::size_t i = 0; i < 3; ++i)
                    res[v[i]] += compute_normal_biased(a, b, c);
                HX_NO_WARNINGS_END
            }
            for (auto& it : res)
                it = glm::normalize(it);
        } else {
            for (auto const& v : obj.getFaces()) {
                auto a = vertices[v.x],
                     b = vertices[v.y],
                     c = vertices[v.z];
                res.push_back(compute_normal(a, b, c));
            }
        }
        return res;
    }();

    // 视角
    glm::mat4x4 view = camera.view_matrix();
    glm::mat4x4 viewModel = view * model; // ModelView

    // 加载投影矩阵
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(perspective));

    // 加载模型视图矩阵
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(viewModel));

    glBegin(GL_TRIANGLES);
    for (std::size_t i = 0; auto const& v : obj.getFaces()) {
        auto a = vertices[v.x],
             b = vertices[v.y],
             c = vertices[v.z];
        if constexpr (IsSmooth) {
            glNormal3fv(glm::value_ptr(normals[v[0]]));
            glVertex3fv(glm::value_ptr(a));
            glNormal3fv(glm::value_ptr(normals[v[1]]));
            glVertex3fv(glm::value_ptr(b));
            glNormal3fv(glm::value_ptr(normals[v[2]]));
            glVertex3fv(glm::value_ptr(c));
        } else {
            glNormal3fv(glm::value_ptr(normals[i]));
            glVertex3fv(glm::value_ptr(a));
            glVertex3fv(glm::value_ptr(b));
            glVertex3fv(glm::value_ptr(c));
        }
        ++i;
    }
    CHECK_GL(glEnd());
}

int main() {
    auto* window = initOpenGL();
    log::hxLog.debug("OpenGL version: ", glGetString(GL_VERSION));

    CHECK_GL(glEnable(GL_POINT_SMOOTH));
    CHECK_GL(glPointSize(64.0f));

    CHECK_GL(glEnable(GL_DEPTH_TEST));       // 深度测试, 防止前后物体不分
    CHECK_GL(glEnable(GL_MULTISAMPLE));      // 多重采样抗锯齿 (MSAA)
    CHECK_GL(glEnable(GL_BLEND));            // 启用 Alpha 通道 (透明度)
    CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // 标准 Alpha 混合: src*alpha + dst*(1-alpha)
    CHECK_GL(glEnable(GL_LIGHTING));         // 启用固定管线光照 (古代特性)
    CHECK_GL(glEnable(GL_LIGHT0));           // 启用 0 号光源 (古代特性)
    CHECK_GL(glEnable(GL_COLOR_MATERIAL));   // 启用材质颜色追踪 (古代特性)
    CHECK_GL(glEnable(GL_NORMALIZE));        // 法线归一化

    // 绑定鼠标移动回调
    glfwSetCursorPosCallback(window, [](
        [[maybe_unused]] GLFWwindow* window, double xpos, double ypos
    ) {
        int width, height;
        // 获取窗口大小
        glfwGetWindowSize(window,  &width, &height);

        // 映射到 [-1, 1], 除以对应的宽、高, 以保证宽高比
        auto x = static_cast<float>(2 * xpos / width - 1);
        // 注意因为窗口坐标是左上原点, y 正轴向下, 所以 转换需要 height - ypos
        auto y = static_cast<float>(2 * (height - ypos) / height - 1);
        glm::vec2 pos = {x, y}; // @todo 设置光源

        // 计算变化量: 是本次鼠标移动和上一次移动的变化量, 而不是和按下鼠标那一刻相比!
        auto delta = glm::fract((pos - lastpos) * 0.5f + 0.5f) * 2.f - 1.f;
        // 按键状态
        if (mouse.rightClickPress) {
            camera.drift(delta);
        } else if (mouse.leftClickPress) {
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS 
             || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS
            ) {
                camera.pan(delta);
            } else {
                camera.orbit(delta);
            }
        }
        lastpos = pos;
    });

    // 绑定鼠标点击事件
    glfwSetMouseButtonCallback(window, [](
        [[maybe_unused]] GLFWwindow* window, int btn, int action, [[maybe_unused]] int mods
    ) {
        switch (btn) {
        case GLFW_MOUSE_BUTTON_LEFT:
            mouse.leftClickPress = action == GLFW_PRESS;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            mouse.rightClickPress = action == GLFW_PRESS;
            break;
        default:
            break;
        }
    });

    // 绑定鼠标滚轮事件
    glfwSetScrollCallback(window, [](
        [[maybe_unused]] GLFWwindow* window, double xoffset, double yoffset
    ) {
        float deltax = xoffset < 0 ? -1 : xoffset > 0 ? 1 : 0;
        float deltay = yoffset < 0 ? -1 : yoffset > 0 ? 1 : 0;
        glm::vec2 delta(deltax, deltay);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS 
         || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS
        ) {
            camera.hitchcock(delta[1]);
        } else {
            camera.zoom(delta[1]);
        }
    });
    using namespace std::chrono;
    float k = 0.618f;
    float xDown = k * std::sqrt(3.f);
    float hOpencv = k * 1.f;

    while (!glfwWindowShouldClose(window)) {
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // 清空画布
        glColor3f(1.f, 0.f, 0.f);
        glm::mat4x4 model{1.f};
        model = glm::translate(model, glm::vec3{hOpencv * 2, 0, 0});
        model = glm::rotate(model, glm::radians(60.f), glm::vec3{0, 1, 0});
        show<true>(window, model);

        glColor3f(0.f, 1.f, 0.f);
        model = glm::mat4x4{1.f};
        model = glm::translate(model, glm::vec3{-hOpencv, 0, -xDown});
        model = glm::rotate(model, glm::radians(180.f), glm::vec3{0, 1, 0});
        show<true>(window, model);

        glColor3f(0.f, 0.f, 1.f);
        model = glm::mat4x4{1.f};
        model = glm::translate(model, glm::vec3{-hOpencv, 0, xDown});
        model = glm::rotate(model, glm::radians(240.f), glm::vec3{0, 1, 0});
        show<true>(window, model);

        glfwSwapBuffers(window); // 双缓冲
        glfwPollEvents();

        std::this_thread::sleep_for(0.01s);
    }
    return 0;
}