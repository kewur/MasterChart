
#pragma once
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include "QueueFamilyIndices.h"


class Runner
{

private:
    const int WindowWidth = 800;
    const int WindowHeight = 600;
    const std::vector<const char*> _RequiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

private:
    VkPhysicalDevice _PhysicalDevice = VK_NULL_HANDLE;
    GLFWwindow* _Window;
    VkSurfaceKHR _RenderSurface;
    VkSwapchainKHR _SwapChain;

    VkDevice _Device;
    VkQueue _GraphicsQueue;
    VkQueue _PresentQueue;
    VkInstance _Instance;
    VkDebugReportCallbackEXT _Callback;

    std::vector<VkImage> _SwapChainImages;
    VkFormat _SwapChainImageFormat;
    VkExtent2D _SwapChainExtent;
    std::vector<VkImageView> _SwapChainImageViews;

public:
    void Run()
    {
        InitializeWindow();
        InitializeVulkan();
        MainLoop();
        CleanUp();
    }
    
private:
    void InitializeVulkan();

    void CreateImageViews();
    void InitializeLogicalDevice();
    void CreateSwapChain();
    static std::vector<VkDeviceQueueCreateInfo> GetQueueCreateInfo(const QueueFamilyIndices& indices);
    VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures();
    void CreateInstance();
    static std::vector<const char*> GetRequiredExtensions();
    static bool CheckValidationLayerSupport();
    void SetupDebugCallback();
    void InitializeWindow();

    void MainLoop() const;

    void ChoosePhysicialDevice();
    bool IsDeviceSuitable(VkPhysicalDevice device);
    bool DeviceHasSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    bool DeviceHasRequiredExensionSupport(VkPhysicalDevice device);
    static VkQueueFlagBits GetRequiredQueues();
    bool DeviceHasRequiredQueueFamilies(VkPhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    void CreateRenderSurface();

    void CleanUp();
};
