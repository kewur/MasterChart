
#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>

class Runner
{

private:
    const int WindowWidth = 800;
    const int WindowHeight = 600;

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
    void CreateInstance();
    static std::vector<const char*> GetRequiredExtensions();
    void SetupValidationLayer();
    static bool CheckValidationLayerSupport();
    void SetupDebugCallback();
    void InitializeWindow();

    void MainLoop() const;
    void ChoosePhysicialDevice();
    bool IsDeviceSuitable(VkPhysicalDevice device);

    void CleanUp();

private:
    VkPhysicalDevice _PhysicalDevice = VK_NULL_HANDLE;
    GLFWwindow* _Window;
    VkInstance _Instance;
    VkDebugReportCallbackEXT _Callback;
};
