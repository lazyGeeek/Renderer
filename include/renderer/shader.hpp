#pragma once
#ifndef RENDERER_SHADER_HPP_
#define RENDERER_SHADER_HPP_

#include <filesystem>
#include <vector>

namespace Renderer
{
    class Shader
    {
    public:
        Shader()  = default;
        ~Shader() = default;

        std::vector<char> ReadFile(const std::filesystem::path& file);
    };
}

#endif // RENDERER_SHADER_HPP_