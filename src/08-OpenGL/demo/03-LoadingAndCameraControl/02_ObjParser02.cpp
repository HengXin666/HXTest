#include <check/OpenGL.hpp>
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/ext/matrix_uint3x3.hpp>
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
            if (line.substr(0, 2) == "v ") {
                // 解析顶点
                std::istringstream s{line.substr(2)};
                glm::vec3 v;
                s >> v.x >> v.y >> v.z;
                _vertices.push_back(std::move(v));
            } else if (line.substr(0, 3) == "vt ") { // 纹理
                std::istringstream s{line.substr(3)};
                glm::vec2 v;
                s >> v.x >> v.y;
                _uvs.push_back(std::move(v));
            } else if (line.substr(0, 3) == "vn ") { // 法线
                std::istringstream s{line.substr(3)};
                glm::vec3 v;
                s >> v.x >> v.y >> v.z;
                _normals.push_back(glm::normalize(v));
            } else if (line.substr(0, 2) == "f ") {
                // 解析面
                std::istringstream s{line.substr(2)};
                std::vector<glm::uvec3> idxArr;
                std::string tmp;
                while (std::getline(s, tmp, ' ')) {
                    std::string numStream;
                    std::istringstream ss{std::move(tmp)};
                    glm::uvec3 idx{1};
                    int i = 0;
                    while (std::getline(ss, numStream, '/') && i < 3) {
                        std::istringstream{numStream} >> idx[i++];
                    }
                    idxArr.push_back(idx - 1u);
                }
                for (std::size_t i = 2; i < idxArr.size(); ++i)
                    _faces.push_back({idxArr[0], idxArr[i], idxArr[i - 1]});
            }
        }

        file.close();
        log::hxLog.info("加载:", path, "完成!");
    }

    auto& getVertices() const noexcept {
        return _vertices;
    }

    auto& getFaces() const noexcept {
        return _faces;
    }

    auto& getUvs() const noexcept {
        return _uvs;
    }

    auto& getNormals() const noexcept {
        return _normals;
    }

private:
    std::vector<glm::vec3> _vertices; // 点
    std::vector<glm::vec2> _uvs;      // 纹理
    std::vector<glm::vec3> _normals;  // 法线
    std::vector<glm::umat3x3> _faces; // 面
};

} // namespace HX

glm::vec3 perspective_divide(glm::vec4 pos) {
    return {pos.x / pos.w, pos.y / pos.w, pos.z / pos.w};
}

void show(GLFWwindow* window) {
    static auto obj = [] {
        ObjParser res;
        res.parser("./obj/monkey.obj");
        return res;
    }();

    int w, h;
    glfwGetWindowSize(window, &w, &h);

    // 构造矩阵
    glm::mat4x4 perspective = glm::perspective(glm::radians(40.0f), (float)w / (float)h, 0.01f, 100.f);
    glm::vec3 eye{0, 0, 5};
    glm::vec3 center{0, 0, 0};
    glm::vec3 up{0, 1, 0};
    glm::mat4x4 view = glm::lookAt(eye, center, up);
    glm::mat4x4 model{1};
    glm::mat4x4 viewModel = view * model; // ModelView

    // 加载投影矩阵
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(perspective));

    // 加载模型视图矩阵
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(viewModel));

    // 法线变换矩阵(只需算一次)
    [[maybe_unused]] glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3{viewModel}));

    // 开始绘制
    glBegin(GL_TRIANGLES);
    auto& vertices = obj.getVertices();
    auto& uvs = obj.getUvs();
    auto& normals = obj.getNormals();
    for (auto const& v : obj.getFaces()) {
        auto v_x = vertices[v[0][0]];
        auto v_y = vertices[v[1][0]];
        auto v_z = vertices[v[2][0]];

        auto vt_x = uvs[v[0][1]];
        auto vt_y = uvs[v[1][1]];
        auto vt_z = uvs[v[2][1]];

        [[maybe_unused]] auto vn_x = normals[v[0][2]];
        [[maybe_unused]] auto vn_y = normals[v[1][2]];
        [[maybe_unused]] auto vn_z = normals[v[2][2]];

        // 第一个顶点
        glNormal3fv(glm::value_ptr(normalMatrix * vn_x));
        glTexCoord2fv(glm::value_ptr(vt_x));
        glVertex3fv(glm::value_ptr(v_x));

        // 第二个顶点
        // glNormal3fv(glm::value_ptr(normalMatrix * vn_y));
        glTexCoord2fv(glm::value_ptr(vt_y));
        glVertex3fv(glm::value_ptr(v_y));

        // 第三个顶点
        // glNormal3fv(glm::value_ptr(normalMatrix * vn_z));
        glTexCoord2fv(glm::value_ptr(vt_z));
        glVertex3fv(glm::value_ptr(v_z));
    }
    glEnd();
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

    // 开启面剔除
    CHECK_GL(glEnable(GL_CULL_FACE));
    CHECK_GL(glCullFace(GL_BACK));
    CHECK_GL(glFrontFace(GL_CW));

    glColor3f(0.9f, 0.6f, 0.1f);
    while (!glfwWindowShouldClose(window)) {
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // 清空画布
        show(window);
        glfwSwapBuffers(window); // 双缓冲
        glfwPollEvents();
    }
    return 0;
}