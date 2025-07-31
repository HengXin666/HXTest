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

struct Vertex {
    float x, y, z;
};

struct Triangle {
    std::size_t a, b, c;
};

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

void show() {
    glBegin(GL_TRIANGLES);
    static auto obj = [] {
        ObjParser res;
        res.parser("./obj/monkey.obj");
        return res;
    }();
    auto& vertices = obj.getVertices();
    for (auto const& v : obj.getTriangles()) {
        // glVertex3fv(&vertices[v.x].x);
        // glVertex3fv(&vertices[v.y].x);
        // glVertex3fv(&vertices[v.z].x);
        glVertex3fv(glm::value_ptr(vertices[v.x]));
        glVertex3fv(glm::value_ptr(vertices[v.y]));
        glVertex3fv(glm::value_ptr(vertices[v.x]));
    }
    CHECK_GL(glEnd());
}

int main() {
    auto* window = initOpenGL();
    log::hxLog.debug("OpenGL version: ", glGetString(GL_VERSION));

    CHECK_GL(glEnable(GL_POINT_SMOOTH));
    CHECK_GL(glEnable(GL_BLEND));
    CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    CHECK_GL(glPointSize(64.0f));

    while (!glfwWindowShouldClose(window)) {
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT)); // 清空画布
        show();
        glfwSwapBuffers(window); // 双缓冲
        glfwPollEvents();
    }
    return 0;
}