#include <HXprint/print.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // 设置 OpenGL 版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口和上下文
    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Test", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    };

    glfwMakeContextCurrent(window);

    // 初始化 GLAD (加载 OpenGL 函数指针)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }

    // 现在可以安全调用 OpenGL 函数
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}