#include "vulkan_renderer.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <sstream>

class MandelbrotApp {
public:
    MandelbrotApp(int width, int height, const char* title)
        : m_width(width), m_height(height) {
        initWindow(title);
        m_renderer = std::make_unique<VulkanRenderer>(m_window, m_width, m_height);
        setupCallbacks();
    }

    ~MandelbrotApp() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void run() {
        auto lastTime = std::chrono::high_resolution_clock::now();
        int frameCount = 0;
        double accumulatedTime = 0.0;

        while (!glfwWindowShouldClose(m_window)) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            frameCount++;
            accumulatedTime += deltaTime;

            // Update FPS counter every second
            if (accumulatedTime >= 1.0) {
                std::stringstream ss;
                ss << "Mandelbrot Vulkan - FPS: " << frameCount
                   << " | Center: (" << std::fixed << std::setprecision(6)
                   << m_renderer->getUBO().center.x << ", " << m_renderer->getUBO().center.y
                   << ") Zoom: " << std::setprecision(2) << m_renderer->getUBO().zoom
                   << " | Iterations: " << m_renderer->getUBO().maxIterations;
                glfwSetWindowTitle(m_window, ss.str().c_str());
                frameCount = 0;
                accumulatedTime = 0.0;
            }

            handleInput(deltaTime);
            glfwPollEvents();
            m_renderer->drawFrame();
        }

        vkDeviceWaitIdle(m_renderer->getDevice());
    }

private:
    int m_width, m_height;
    GLFWwindow* m_window;
    std::unique_ptr<VulkanRenderer> m_renderer;

    // Camera state
    bool m_mouseDragging = false;
    double m_lastMouseX = 0, m_lastMouseY = 0;
    bool m_keys[1024] = {false};

    void initWindow(const char* title) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_window = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);
        if (!m_window) {
            throw std::runtime_error("Failed to create GLFW window!");
        }

        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
        glfwSetKeyCallback(m_window, keyCallback);
        glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
        glfwSetCursorPosCallback(m_window, cursorPosCallback);
        glfwSetScrollCallback(m_window, scrollCallback);
    }

    void setupCallbacks() {
        // Callbacks are static, use user pointer to access instance
    }

    void handleInput(float deltaTime) {
        auto& ubo = const_cast<VulkanRenderer::UniformBufferObject&>(m_renderer->getUBO());
        bool updated = false;

        // Movement (WASD / Arrow keys)
        float panSpeed = deltaTime * 2.0f / ubo.zoom;
        if (m_keys[GLFW_KEY_W] || m_keys[GLFW_KEY_UP]) {
            ubo.center.y -= panSpeed;
            updated = true;
        }
        if (m_keys[GLFW_KEY_S] || m_keys[GLFW_KEY_DOWN]) {
            ubo.center.y += panSpeed;
            updated = true;
        }
        if (m_keys[GLFW_KEY_A] || m_keys[GLFW_KEY_LEFT]) {
            ubo.center.x -= panSpeed;
            updated = true;
        }
        if (m_keys[GLFW_KEY_D] || m_keys[GLFW_KEY_RIGHT]) {
            ubo.center.x += panSpeed;
            updated = true;
        }

        // Zoom with keyboard (Q/E)
        float zoomSpeed = deltaTime * 2.0f;
        if (m_keys[GLFW_KEY_Q]) {
            ubo.zoom *= exp(zoomSpeed);
            updated = true;
        }
        if (m_keys[GLFW_KEY_E]) {
            ubo.zoom *= exp(-zoomSpeed);
            updated = true;
        }

        // Iteration adjustment (I/K)
        if (m_keys[GLFW_KEY_I]) {
            ubo.maxIterations = std::min(10000, ubo.maxIterations + 10);
            updated = true;
        }
        if (m_keys[GLFW_KEY_K]) {
            ubo.maxIterations = std::max(10, ubo.maxIterations - 10);
            updated = true;
        }

        // Reset view (R)
        if (m_keys[GLFW_KEY_R]) {
            ubo.center = glm::vec2(-0.5f, 0.0f);
            ubo.zoom = 1.0f;
            ubo.maxIterations = 256;
            updated = true;
        }

        if (updated) {
            m_renderer->updateUniforms(ubo);
        }
    }

    // Static callbacks
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = static_cast<MandelbrotApp*>(glfwGetWindowUserPointer(window));
        app->m_renderer->resize(width, height);
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = static_cast<MandelbrotApp*>(glfwGetWindowUserPointer(window));
        if (key >= 0 && key < 1024) {
            app->m_keys[key] = (action != GLFW_RELEASE);
        }

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        auto app = static_cast<MandelbrotApp*>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                app->m_mouseDragging = true;
                glfwGetCursorPos(window, &app->m_lastMouseX, &app->m_lastMouseY);
            } else if (action == GLFW_RELEASE) {
                app->m_mouseDragging = false;
            }
        }
        // Middle mouse button for zoom to point
        if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            app->zoomToPoint(x, y, 2.0f);
        }
    }

    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        auto app = static_cast<MandelbrotApp*>(glfwGetWindowUserPointer(window));
        if (app->m_mouseDragging) {
            auto& ubo = const_cast<VulkanRenderer::UniformBufferObject&>(app->m_renderer->getUBO());

            float dx = float(xpos - app->m_lastMouseX) / app->m_width * 2.0f / ubo.zoom * ubo.aspectRatio;
            float dy = float(ypos - app->m_lastMouseY) / app->m_height * 2.0f / ubo.zoom;

            ubo.center.x -= dx;
            ubo.center.y += dy;  // Y is inverted in screen coordinates

            app->m_renderer->updateUniforms(ubo);
            app->m_lastMouseX = xpos;
            app->m_lastMouseY = ypos;
        }
    }

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        auto app = static_cast<MandelbrotApp*>(glfwGetWindowUserPointer(window));
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        float zoomFactor = (yoffset > 0) ? 1.5f : 1.0f / 1.5f;
        app->zoomToPoint(x, y, zoomFactor);
    }

    void zoomToPoint(double screenX, double screenY, float zoomFactor) {
        auto& ubo = const_cast<VulkanRenderer::UniformBufferObject&>(m_renderer->getUBO());

        // Convert screen coordinates to complex plane coordinates before zoom
        float ndcX = (float(screenX) / m_width) * 2.0f - 1.0f;
        float ndcY = 1.0f - (float(screenY) / m_height) * 2.0f;  // Flip Y
        ndcX *= ubo.aspectRatio;

        vec2 worldPos = ubo.center + vec2(ndcX, ndcY) / ubo.zoom;

        // Apply zoom
        ubo.zoom *= zoomFactor;

        // Adjust center so the point under cursor stays fixed
        ubo.center = worldPos - vec2(ndcX, ndcY) / ubo.zoom;

        m_renderer->updateUniforms(ubo);
    }
};

int main() {
    try {
        MandelbrotApp app(1280, 720, "Mandelbrot Vulkan");
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}