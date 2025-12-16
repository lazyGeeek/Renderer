#pragma once
#ifndef RENDERER_VULKAN_HPP_
#define RENDERER_VULKAN_HPP_

#include <filesystem>
#include <memory>
#include <map>

#include "renderer/shader.hpp"

namespace Renderer
{
    class Vulkan
    {
    public:
        Vulkan();
        ~Vulkan() = default;

        Vulkan(const Vulkan& other)             = delete;
        Vulkan(Vulkan&& other)                  = delete;
        Vulkan& operator=(const Vulkan& other)  = delete;
        Vulkan& operator=(const Vulkan&& other) = delete;

    //     void AddShader(std::string name, const std::filesystem::path& file);

    // private:
    //     std::map<std::string, std::unique_ptr<Shader>> m_shaders;
    };
}

#endif // RENDERER_VULKAN_HPP_