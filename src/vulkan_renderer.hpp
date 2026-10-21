#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec2 pos;
    glm::vec2 uv;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributes{};
        attributes[0].binding = 0;
        attributes[0].location = 0;
        attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[0].offset = offsetof(Vertex, pos);

        attributes[1].binding = 0;
        attributes[1].location = 1;
        attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[1].offset = offsetof(Vertex, uv);
        return attributes;
    }
};

struct UniformBufferObject {
    glm::vec2 center;     // Center of view in complex plane
    float zoom;           // Zoom level
    float aspectRatio;    // Window aspect ratio
    int maxIterations;    // Maximum iterations
    float padding[3];     // Padding for alignment
};

class VulkanRenderer {
public:
    VulkanRenderer(GLFWwindow* window, int width, int height);
    ~VulkanRenderer();

    void drawFrame();
    void resize(int width, int height);
    void updateUniforms(const UniformBufferObject& ubo);

    // Camera control
    void setCenter(const glm::vec2& center) { m_ubo.center = center; }
    void setZoom(float zoom) { m_ubo.zoom = zoom; }
    void setMaxIterations(int iterations) { m_ubo.maxIterations = iterations; }

    const UniformBufferObject& getUBO() const { return m_ubo; }

private:
    GLFWwindow* m_window;
    int m_width, m_height;

    // Vulkan core
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    VkFormat m_swapchainImageFormat;
    VkExtent2D m_swapchainExtent;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkFramebuffer> m_swapchainFramebuffers;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;

    // Synchronization
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    size_t m_currentFrame = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;

    // Vertex buffer (fullscreen quad)
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;

    // Uniform buffers
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;
    UniformBufferObject m_ubo{};

    // Descriptor sets
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets;

    // Shader modules
    VkShaderModule m_vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule m_fragShaderModule = VK_NULL_HANDLE;

    // Helper functions
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createVertexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSyncObjects();

    void cleanupSwapchain();
    void recreateSwapchain();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrameImpl();

    // Vulkan helpers
    static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
    static std::vector<char> readFile(const std::string& filename);
    static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    static bool checkValidationLayerSupport();
    static std::vector<const char*> getRequiredExtensions();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    bool isDeviceSuitable(VkPhysicalDevice device);
};