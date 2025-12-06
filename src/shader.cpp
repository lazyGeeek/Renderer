#include "renderer/shader.hpp"

#include <format>
#include <fstream>
#include <vector>

namespace Renderer
{
    std::vector<char> Shader::ReadFile(const std::filesystem::path& file)
    {
        if (!std::filesystem::exists(file))
            throw std::runtime_error(std::format("[ERROR] {} file doesn't exist", file.string()));

        std::ifstream shader(file.string(), std::ios::ate | std::ios::binary);

        if (!shader.is_open())
            throw std::runtime_error(std::format("[ERROR] Can't open {}", file.string()));

        size_t shaderSize = static_cast<size_t>(shader.tellg());
        std::vector<char> buffer(shaderSize);

        shader.seekg(0);
        shader.read(buffer.data(), shaderSize);
        shader.close();
        return buffer;
    }
}