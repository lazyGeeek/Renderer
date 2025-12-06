#include "renderer/vulkan.hpp"
#include "renderer/shader.hpp"

#include <stdexcept>

namespace Renderer
{
    Vulkan::Vulkan()
    {
    }

    void Vulkan::AddShader(std::string name, const std::filesystem::path& file)
    {
        std::unique_ptr<Shader> shader = std::make_unique<Shader>();
        
        if (!shader)
            throw std::runtime_error("Can't initialize shader");

        if (m_shaders.contains(name))
            m_shaders[name] = nullptr;

        m_shaders.insert(std::pair<std::string, std::unique_ptr<Shader>>(name, std::move(shader)));
    }
}
