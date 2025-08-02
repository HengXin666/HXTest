#include <check/OpenGL.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <stdexcept>
#include <vector>

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

    auto& getTriangles() const noexcept {
        return _trinales;
    }

private:
    std::vector<glm::vec3> _vertices;
    std::vector<glm::uvec3> _trinales;
    
};

} // namespace HX

glm::vec3 perspective_divide(glm::vec4 pos) {
    return {pos.x / pos.w, pos.y / pos.w, pos.z / pos.w};
}

void show(GLFWwindow* window) {
    glBegin(GL_TRIANGLES);
    static auto obj = [] {
        ObjParser res;
        res.parser("./obj/monkey.obj");
        return res;
    }();
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    glm::mat4x4 perspective = glm::perspective(glm::radians(40.0f), (float)w / (float)h, 0.01f, 100.f);
    
    // 视角
    glm::vec3 eye{0, 0, 5};
    glm::vec3 center{0, 0, 0};
    glm::vec3 up{0, 1, 0};
    glm::mat4x4 view = glm::lookAt(eye, center, up);

    glm::mat4x4 model{1};

    auto& vertices = obj.getVertices();
    for (auto const& v : obj.getTriangles()) {
        auto a = vertices[v.x],
             b = vertices[v.y],
             c = vertices[v.z];
        glVertex3fv(glm::value_ptr(
            perspective_divide(perspective * view * model * glm::vec4(a, 1))));
        glVertex3fv(glm::value_ptr(
            perspective_divide(perspective * view * model * glm::vec4(b, 1))));
        glVertex3fv(glm::value_ptr(
            perspective_divide(perspective * view * model * glm::vec4(c, 1))));
    }
    CHECK_GL(glEnd());
}

int main() {
    auto* window = initOpenGL();
    log::hxLog.debug("OpenGL version: ", glGetString(GL_VERSION));

    CHECK_GL(glEnable(GL_POINT_SMOOTH));
    CHECK_GL(glEnable(GL_BLEND));
    // CHECK_GL(glEnable(GL_DEPTH_TEST)); // 开启深度测试
    CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    CHECK_GL(glPointSize(64.0f));

    while (!glfwWindowShouldClose(window)) {
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT)); // 清空画布
        show(window);
        glfwSwapBuffers(window); // 双缓冲
        glfwPollEvents();
    }
    return 0;
}