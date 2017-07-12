
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
    std::vector<const char*> GetRequiredExtensions();
    void SetupValidationLayer();
    static bool CheckValidationLayerSupport(std::vector<const char*> validationLayers);
    void SetupDebugCallback();
    void InitializeWindow();

    void MainLoop();
    
    void CleanUp();

private:
    GLFWwindow* _Window;
    VkInstance _Instance;
    bool _EnableValidationLayers = false;
    VkDebugReportCallbackEXT _DebugReportCallback;
};
