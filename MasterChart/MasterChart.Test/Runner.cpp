#include "Runner.h"
#include <iostream>
#include <set>
#include "SwapChainSupportDetails.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) {
    std::cerr << "validation layer: " << msg << std::endl;

    return VK_FALSE;
}

const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

void Runner::InitializeVulkan()
{
    CreateInstance();
    SetupDebugCallback();
    CreateRenderSurface();
    ChoosePhysicialDevice();
    InitializeLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
}

void Runner::CreateImageViews()
{
    _SwapChainImageViews.resize(_SwapChainImages.size());

    for (size_t i = 0; i < _SwapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = _SwapChainImages[i];

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = _SwapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(_Device, &createInfo, nullptr, &_SwapChainImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create image view");
    }

}

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(instance, callback, pAllocator);
    }
}

void Runner::InitializeWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    _Window = glfwCreateWindow(WindowWidth, WindowHeight, "Master Chart Test", nullptr, nullptr);
}

void Runner::InitializeLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(_PhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo>  queueCreateInfos = GetQueueCreateInfo(indices);
    VkPhysicalDeviceFeatures deviceFeatures = GetPhysicalDeviceFeatures();

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(_RequiredDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = _RequiredDeviceExtensions.data();

    if (enableValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
        deviceCreateInfo.enabledLayerCount = 0;

    if (vkCreateDevice(_PhysicalDevice, &deviceCreateInfo, nullptr, &_Device) != VK_SUCCESS)
        throw std::runtime_error("Could not create logical device");

    vkGetDeviceQueue(_Device, indices.GraphFamily, 0, &_GraphicsQueue);
    vkGetDeviceQueue(_Device, indices.PresentFamily, 0, &_PresentQueue);
}

void Runner::CreateSwapChain()
{
    SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::QuerySwapChainSupport(_PhysicalDevice, _RenderSurface);

    VkSurfaceFormatKHR surfaceFormat = SwapChainSupportDetails::ChooseSwapSurfaceFormat(swapChainSupport.SurfaceFormats);
    VkPresentModeKHR presentMode = SwapChainSupportDetails::ChoosePresentMode(swapChainSupport.PresentModes);
    VkExtent2D extent = SwapChainSupportDetails::ChooseSwapExtent(swapChainSupport.Capabilities, WindowWidth, WindowHeight);

    uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
    if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount)
        imageCount = swapChainSupport.Capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _RenderSurface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(_PhysicalDevice);
    uint32_t queueFamilyIndices[] = { static_cast<uint32_t>(indices.GraphFamily), static_cast<uint32_t>(indices.PresentFamily) };

    if (indices.GraphFamily != indices.PresentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_Device, &createInfo, nullptr, &_SwapChain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain");

    vkGetSwapchainImagesKHR(_Device, _SwapChain, &imageCount, nullptr);
    _SwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_Device, _SwapChain, &imageCount, _SwapChainImages.data());

    _SwapChainImageFormat = surfaceFormat.format;
    _SwapChainExtent = extent;
}

std::vector<VkDeviceQueueCreateInfo> Runner::GetQueueCreateInfo(const QueueFamilyIndices& indices)
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQeueFamilies = { indices.GraphFamily, indices.PresentFamily };

    for (int queueFamily : uniqueQeueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;

        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    return queueCreateInfos;
}

VkPhysicalDeviceFeatures Runner::GetPhysicalDeviceFeatures()
{
    return {};
}

void Runner::MainLoop() const
{
    while (!glfwWindowShouldClose(_Window)) {
        glfwPollEvents();
    }
}

#pragma region Physical Device

void Runner::ChoosePhysicialDevice()
{
    uint32_t deviceCount = 0;

    vkEnumeratePhysicalDevices(_Instance, &deviceCount, nullptr);

    if (deviceCount == 0)
        throw std::runtime_error("No GPUs with Vulkan support found, make sure you have installed the Vulkan runtime.");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_Instance, &deviceCount, devices.data());


    for (const auto& device : devices)
    {
        if (IsDeviceSuitable(device))
        {
            _PhysicalDevice = device;
            break;
        }
    }

    if (_PhysicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find a suitable GPU");

}

bool Runner::IsDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);


    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        && deviceFeatures.geometryShader
        && DeviceHasRequiredExensionSupport(device)
        && DeviceHasSwapChainSupport(device, _RenderSurface)
        && DeviceHasRequiredQueueFamilies(device);
}


bool Runner::DeviceHasSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails swapChainSupportDetails = SwapChainSupportDetails::QuerySwapChainSupport(device, surface);

    return !swapChainSupportDetails.SurfaceFormats.empty() && !swapChainSupportDetails.PresentModes.empty();
}

bool Runner::DeviceHasRequiredExensionSupport(VkPhysicalDevice device)
{
    uint32_t availableExtensionsCount;

    vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionsCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(availableExtensionsCount);

    vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionsCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(_RequiredDeviceExtensions.begin(), _RequiredDeviceExtensions.end());

    for (const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}


bool Runner::DeviceHasRequiredQueueFamilies(VkPhysicalDevice device)
{
    return FindQueueFamilies(device).IsComplete();
}

QueueFamilyIndices Runner::FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilyProperties)
    {

#pragma region Render Surface 
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _RenderSurface, &presentSupport);

        if (queueFamily.queueCount > 0 && presentSupport)
            indices.PresentFamily = i;
#pragma endregion 

        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & GetRequiredQueues())
            indices.GraphFamily = i;

        if (indices.IsComplete())
            break;

        i++;
    }

    return indices;
}

VkQueueFlagBits Runner::GetRequiredQueues()
{
    return VK_QUEUE_GRAPHICS_BIT;
}

#pragma endregion Physical Device

void Runner::CreateRenderSurface()
{
    if (glfwCreateWindowSurface(_Instance, _Window, nullptr, &_RenderSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render surface");
    }
}

void Runner::CleanUp() {
    DestroyDebugReportCallbackEXT(_Instance, _Callback, nullptr);

    for (size_t i = 0; i < _SwapChainImageViews.size(); i++)
        vkDestroyImageView(_Device, _SwapChainImageViews[i], nullptr);

    vkDestroySwapchainKHR(_Device, _SwapChain, nullptr);
    vkDestroyDevice(_Device, nullptr);
    vkDestroySurfaceKHR(_Instance, _RenderSurface, nullptr);
    vkDestroyInstance(_Instance, nullptr);

    glfwDestroyWindow(_Window);

    glfwTerminate();
}

void Runner::CreateInstance() {
    if (enableValidationLayers && !CheckValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInstanceInfo = {};
    createInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInstanceInfo.pApplicationInfo = &appInfo;

    auto extensions = GetRequiredExtensions();
    createInstanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInstanceInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidationLayers) {
        createInstanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInstanceInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInstanceInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInstanceInfo, nullptr, &_Instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void Runner::SetupDebugCallback() {
    if (!enableValidationLayers) return;

    VkDebugReportCallbackCreateInfoEXT createReportInfo = {};
    createReportInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createReportInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createReportInfo.pfnCallback = DebugCallback;

    if (CreateDebugReportCallbackEXT(_Instance, &createReportInfo, nullptr, &_Callback) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug _Callback!");
    }
}

std::vector<const char*> Runner::GetRequiredExtensions() {
    std::vector<const char*> extensions;

    unsigned int glfwExtensionCount = 0;
    const char ** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (unsigned int i = 0; i < glfwExtensionCount; i++) {
        extensions.push_back(glfwExtensions[i]);
    }

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return extensions;
}

bool Runner::CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

