#include "Runner.h"
#include <stdexcept>
#include <vector>
#include <iostream>


void Runner::InitializeVulkan()
{
    SetupValidationLayer();

    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "MasterChart Test";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "No Engine";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;

    const std::vector<const char*> extensionNames = GetRequiredExtensions();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &_Instance) != VK_SUCCESS)
        throw std::runtime_error("Can't create Vulkan instance");

}

std::vector<const char*> Runner::GetRequiredExtensions()
{
    std::vector<const char*> extensionNamesVector;

    unsigned int extensionsCount = 0;
    const char ** extensionNames = glfwGetRequiredInstanceExtensions(&extensionsCount);

    for (unsigned int i = 0; i < extensionsCount; i++)
        extensionNamesVector.push_back(extensionNames[i]);

    if (_EnableValidationLayers)
        extensionNamesVector.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    return extensionNamesVector;
}

void Runner::SetupValidationLayer()
{
#ifdef NDEBUG
    return;
#endif
    _EnableValidationLayers = true;
    const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };

    if (!CheckValidationLayerSupport(validationLayers))
        throw std::runtime_error("Requested validation layers not supported");
}

bool Runner::CheckValidationLayerSupport(std::vector<const char*> validationLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}


void Runner::InitializeWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _Window = glfwCreateWindow(WindowWidth, WindowHeight, "MasterChart.Test", nullptr, nullptr);
}

void Runner::MainLoop()
{
    while (!glfwWindowShouldClose(_Window))
    {
        glfwPollEvents();
    }
}

void Runner::CleanUp()
{
    vkDestroyInstance(_Instance, nullptr);
    glfwDestroyWindow(_Window);
    glfwTerminate();
}
