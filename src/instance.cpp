#include "renderer/instance.hpp"
#include "renderer/debug_messanger.hpp"

#include <cstring>
#include <stdexcept>
#include <unordered_set>

namespace Renderer
{
    Instance::Instance(GLFWwindow* window, bool enableValidationLayer) :
        m_window { window },
        m_enableValidationLayer { enableValidationLayer } { }

    Instance::~Instance()
    {
        Destroy();
    }

    void Instance::Create()
    {
        if (m_enableValidationLayer)
            checkValidationLayerSupport();

        VkApplicationInfo appInfo { };
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Vulkan";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo instanceInfo { };
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &appInfo;

        uint32_t extensionCount = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        std::vector<const char*> requiredExtensions;

        for (uint32_t i = 0; i < extensionCount; ++i)
        {
            requiredExtensions.emplace_back(extensions[i]);
        }

        if (m_enableValidationLayer)
            requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        checkGflwRequiredInstanceExtensions(requiredExtensions);

#ifdef __APPLE__
            requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo { };
        if (m_enableValidationLayer)
        {
            instanceInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
            instanceInfo.ppEnabledLayerNames = m_validationLayers.data();

            DebugMessanger::PopulateDebugMessengerCreateInfo(debugCreateInfo);
            instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            instanceInfo.enabledLayerCount = 0;
            instanceInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&instanceInfo, nullptr, &m_instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance");

        if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create surface");

        m_debugMessanger = std::make_unique<DebugMessanger>(m_instance);
        m_debugMessanger->Create();
    }

    void Instance::Destroy()
    {
        if (m_debugMessanger)
        {
            m_debugMessanger->Destroy();
            m_debugMessanger = nullptr;
        }

        if (m_surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
        }

        if (m_instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(m_instance, nullptr);
            m_instance = VK_NULL_HANDLE;
        }
    }

    const VkInstance& Instance::GetInstance() const
    {
        return m_instance;
    }

    const VkSurfaceKHR& Instance::GetSurface() const
    {
        return m_surface;
    }

    bool Instance::IsValidationLayerEnabled() const
    {
        return m_enableValidationLayer;
    }

    void Instance::checkValidationLayerSupport()
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : m_validationLayers)
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
                throw std::runtime_error("Validation layers requested, but not available");
        }
    }

    void Instance::checkGflwRequiredInstanceExtensions(const std::vector<const char*>& requiredExtensions)
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::unordered_set<std::string> available;
        for (const auto& extension : extensions)
        {
            available.insert(extension.extensionName);
        }

        for (const auto& required : requiredExtensions)
        {
            if (available.find(required) == available.end())
                throw std::runtime_error("Missing required GLFW extension");
        }
    }
}
