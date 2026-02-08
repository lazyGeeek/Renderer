#include "renderer/instance.hpp"

#include <iostream>
#include <ranges>

namespace Renderer
{
    void Instance::Create(GLFWwindow* window)
    {
        vk::ApplicationInfo appInfo { };
        appInfo.pApplicationName   = "Vulkan Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName        = "Vulkan";
        appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion         = vk::ApiVersion13;

        // Get the required layers
        std::vector<const char*> requiredLayers = getRequiredLayers();
        std::vector<const char*> requredExtensions = getRequiredExtensions();

        vk::InstanceCreateInfo createInfo { };
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size());
        createInfo.ppEnabledLayerNames     = requiredLayers.data();
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(requredExtensions.size());
        createInfo.ppEnabledExtensionNames = requredExtensions.data();
        
        m_instance = vk::raii::Instance(m_context, createInfo);

        setupDebugMessenger();

        if (window)
            createSurface(window);
    }

    const vk::raii::Instance& Instance::Get() const
    {
        return m_instance;
    }

    const vk::raii::SurfaceKHR& Instance::GetSurface() const
    {
        return m_surface;
    }

    std::vector<const char*> Instance::getRequiredLayers()
    {
        std::vector<char const*> requiredLayers;

        if (m_enableValidationLayers)
            requiredLayers.assign(m_validationLayers.begin(), m_validationLayers.end());

        // Check if the required layers are supported by the Vulkan implementation.
        auto layerProperties = m_context.enumerateInstanceLayerProperties();
        if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer)
        {
            return std::ranges::none_of(layerProperties,
                                        [requiredLayer](auto const& layerProperty)
                                        { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
        }))
        {
            throw std::runtime_error("[Vulkan][Instance] One or more required layers are not supported!");
        }

        return requiredLayers;
    }

    std::vector<const char*> Instance::getRequiredExtensions()
    {
        // Get the required instance extensions from GLFW.
        uint32_t glfwExtensionCount = 0;
        auto     glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // Check if the required GLFW extensions are supported by the Vulkan implementation.
        auto extensionProperties = m_context.enumerateInstanceExtensionProperties();

        for (uint32_t i = 0; i < glfwExtensionCount; ++i)
        {
            if (std::ranges::none_of(extensionProperties,
                                     [glfwExtension = glfwExtensions[i]](auto const &extensionProperty)
                                     { return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
            {
                throw std::runtime_error("[Vulkan][Instance] Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
            }
        }

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (m_enableValidationLayers)
            extensions.push_back(vk::EXTDebugUtilsExtensionName);

        return extensions;
    }

    VKAPI_ATTR vk::Bool32 VKAPI_CALL Instance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                             const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                             void* pUserData)
    {
        std::cerr << "[Vulkan] Validation layer: Message: " << callbackData->pMessage << std::endl;
        return vk::False;
    }

    void Instance::setupDebugMessenger()
    {
        if (!m_enableValidationLayers)
            return;

        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
        vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
        vk::DebugUtilsMessengerCreateInfoEXT  debugUtilsMessengerCreateInfoEXT;

        debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags;
        debugUtilsMessengerCreateInfoEXT.messageType     = messageTypeFlags;
        debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &debugCallback;

        m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
    }

    void Instance::createSurface(GLFWwindow* window)
    {
        if (!window)
            return;

        VkSurfaceKHR surface = VK_NULL_HANDLE;
        if (glfwCreateWindowSurface(*m_instance, window, nullptr, &surface) != 0)
            throw std::runtime_error("[Vulkan][Instance] Failed to create window surface!");

        m_surface = vk::raii::SurfaceKHR(m_instance, surface);
    }
}
