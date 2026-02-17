#pragma once
#ifndef RENDERER_UTILS_NON_COPYABLE_HPP_
#define RENDERER_UTILS_NON_COPYABLE_HPP_

#define NON_COPYABLE(ClassName) \
        ClassName(const ClassName& other)            = delete; \
        ClassName(ClassName&& other)                 = delete; \
        ClassName& operator=(const ClassName& other) = delete; \
        ClassName& operator=(ClassName&& other)      = delete;

namespace Renderer::Interfaces
{
    class NonCopyable
    {
    public:
        NonCopyable() = default;

        NonCopyable(const NonCopyable& other)            = delete;
        NonCopyable(NonCopyable&& other)                 = delete;
        NonCopyable& operator=(const NonCopyable& other) = delete;
        NonCopyable& operator=(NonCopyable&& other)      = delete;
    };
}

#endif // RENDERER_UTILS_NON_COPYABLE_HPP_
