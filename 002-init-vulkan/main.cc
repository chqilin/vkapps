
#include "header.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


int main(int argc, char** argv) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(_WINDOW_WIDTH, _WINDOW_HEIGHT, _WINDOW_TITLE, NULL, NULL);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
