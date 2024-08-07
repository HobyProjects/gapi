#pragma once

#include <memory>
#include <type_traits>
#include <string>
#include <initializer_list>
#include <vector>
#include <algorithm>
#include <cassert>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

#if !defined(__glew_h__) || !defined(__GLEW_H__)
#include <GL/glew.h>
#endif

#ifndef _glfw3_h_
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif
#endif

#ifdef APIENTRY
#undef APIENTRY
#endif

#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
#define GAPI_PLATFORM_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
// Check for Apple platforms
#if TARGET_IPHONE_SIMULATOR == 1
#error "IOS simulator is not supported!"
#elif TARGET_OS_IPHONE == 1
#define GAPI_PLATFORM_IOS
#error "IOS is not supported!"
#elif TARGET_OS_MAC == 1
#define GAPI_PLATFORM_MACOS
#error "MacOS is not supported!"
#else
#error "Unknown Apple platform!"
#endif
#elif defined(__ANDROID__)
#define GAPI_PLATFORM_ANDROID
#elif defined(__linux__)
#define GAPI_PLATFORM_LINUX
__attribute__((visibility("default")))
#else
#error "Unknown platform!"
#endif

#ifdef _DEBUG

// Debugging
#if defined(GAPI_PLATFORM_WINDOWS)
#define gapi_debugbreak() __debugbreak()
#define gapi_debug_msg(cap, msg) std::cerr << cap << msg << "\n"
#elif defined(GAPI_PLATFORM_LINUX)
#include <signal.h>
#define gapi_debugbreak() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif

#define gapi_asserts(exp, msg) if(!(exp)) { gapi_debug_msg("Assertion Failed: ", msg); gapi_debugbreak(); }

#else

#define gapi_asserts(exp, msg)
#define gapi_debug_msg(cap, msg)

#endif

namespace gapi{
    
    enum draw : size_t{
        static_draw     = 0, 
        dynamic_draw    = 1, 
    };

    enum component : int32_t{
        xyz     = 3,    xyzw    = 4,
        rgb     = 3,    rgba    = 4,
        xy      = 2,    uv      = 2,    none    = 0,
    };

    enum data_types : size_t{
        null    = 0,    f1      = 4,    f2      = 8,    f3      = 12,  
        f4      = 16,   i1      = 4,    i2      = 8,    i3      = 12,
        i4      = 16,   ui1     = 4,    ui2     = 8,    ui3     = 12, 
        ui4     = 16,   mat2    = 16,   mat3    = 36,   mat4    = 64,   boolean  = 1
    };

    enum class _gapi {
        none = 0, opengl = 1, directx = 2, vulkan = 3, metal = 4
    };


    class api_info {

        public:
            api_info() = default;
            virtual ~api_info() = default;

            virtual const std::string& vendor() const = 0;
            virtual const std::string& renderer() const = 0;
            virtual const std::string& version() const = 0;
            virtual const std::string& language() const = 0;
    };

    class context{

        public:
            context() = default;
            context(const context&) = delete;
            context& operator=(const context&) = delete;
            context(context&&) = delete;
            context& operator=(context&&) = delete;
            virtual ~context() = default;

            virtual bool init() = 0;
            virtual void swap() = 0;
            virtual void interval(uint32_t interval) = 0;
    };

    struct buffer_elements{
        buffer_elements() {}
        buffer_elements(const std::string& name, component comp, size_t size, bool normalized = false) noexcept
            : name(name), component(comp), size(size), normalized(normalized) {}
        ~buffer_elements() = default;

        std::string name{};
        component component{component::none};
        size_t size{0};
        size_t offset{0}; 
        bool normalized{false};
    };

    class buffer_layout{
        public:
            buffer_layout(){}
            buffer_layout(std::initializer_list<buffer_elements> elements)
                : m_elements(elements) { _stride(); }
            ~buffer_layout() = default;

            [[nodiscard]] inline size_t stride() const { return m_stride; }
            [[nodiscard]] inline const std::vector<buffer_elements>& elements() const { return m_elements; }

            inline std::vector<buffer_elements>::iterator begin() { return m_elements.begin(); }
            inline std::vector<buffer_elements>::iterator end() { return m_elements.end(); }
            inline std::vector<buffer_elements>::const_iterator begin() const { return m_elements.begin(); }
            inline std::vector<buffer_elements>::const_iterator end() const { return m_elements.end(); }

        private:
            inline void _stride(){
                m_stride = 0;
                size_t offset = 0;
                for (auto &element : m_elements)
                {
                    element.offset = offset;
                    offset += element.size;
                    m_stride += element.size; 
                }
            }

        private:
            std::vector<buffer_elements> m_elements{};
            size_t m_stride{0};
    };

    class vertex_buffer{
        public:
            constexpr vertex_buffer() = default;
            virtual ~vertex_buffer() = default;

            virtual void bind() const = 0;
            [[maybe_unused]] virtual void unbind() const = 0;

            virtual void configure_layout(const buffer_layout& layout) const = 0;
            virtual const buffer_layout& layout() const = 0;
    };

    class index_buffer{
        public:
            index_buffer() = default;
            virtual ~index_buffer() = default;

            virtual void bind() const = 0;
            [[maybe_unused]] virtual void unbind() const = 0;
            virtual size_t count() const = 0;
    };

    class vertex_array{

        public:
            vertex_array() = default;
            virtual ~vertex_array() = default;

            virtual void bind() const = 0;
            [[maybe_unused]] virtual void unbind() const = 0;

            virtual void emplace_vbuffer(const std::shared_ptr<vertex_buffer>& vertex_buffer)  = 0;
            virtual void emplace_ibuffer(const std::shared_ptr<index_buffer>& index_buffer) = 0;
            inline virtual const std::vector<std::shared_ptr<vertex_buffer>>& vertexs() const = 0;
            inline virtual const std::shared_ptr<index_buffer>& index() const = 0;
    };

    class shader{
        public:
            shader() = default;
            virtual ~shader() = default;

            virtual void bind() const = 0;
            [[maybe_unused]] virtual void unbind() const = 0;

            virtual const std::string& name() const = 0;
            virtual bool uniform(const std::string& n, uint32_t v) const = 0;
            virtual bool uniform(const std::string& n, float v) const = 0;
            virtual bool uniform(const std::string& n, float x, float y) const = 0;
            virtual bool uniform(const std::string& n, float x, float y, float z) const = 0;
            virtual bool uniform(const std::string& n, float x, float y, float z, float w) const = 0;
            virtual bool uniform(const std::string& n, const glm::vec2& v) const = 0;
            virtual bool uniform(const std::string& n, const glm::vec3& v) const = 0;
            virtual bool uniform(const std::string& n, const glm::vec4& v) const = 0;
            virtual bool uniform(const std::string& n, const glm::mat2& v) const = 0;
            virtual bool uniform(const std::string& n, const glm::mat3& v) const = 0;
            virtual bool uniform(const std::string& n, const glm::mat4& v) const = 0;
    };

    class texture{
        public:
            texture() = default;
            virtual ~texture() = default;

            virtual void bind(uint32_t slot = 0) const = 0;
            [[maybe_unused]] virtual void unbind() const = 0;
            [[maybe_unused]] virtual void* data() const = 0;

            virtual uint32_t id() const = 0;
            virtual int32_t width() const = 0;
            virtual int32_t height() const = 0;
            virtual int32_t channels() const = 0;
    };

    class base_api{

        public:
            base_api() = default;
            virtual ~base_api() = default;

            virtual void init() = 0;
            virtual void draw(const std::shared_ptr<vertex_array>& va) = 0;
            virtual void clear()  = 0;
            virtual void clear_color(float r, float g, float b, float a) = 0;   
            virtual _gapi api() const  = 0;   
    };


    template<typename Ty>
    requires std::is_base_of<context, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_context() noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty>
    requires std::is_base_of<context, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_context(GLFWwindow*) noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty, typename Args>
    requires std::is_base_of<vertex_buffer, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_context(Args args) noexcept{
        return std::make_shared<Ty>(std::forward<Args>(args)...);
    }

    template<typename Ty, typename... TArgs>
    requires std::is_base_of<vertex_buffer, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_context(TArgs... args) noexcept{
        return std::make_shared<Ty>(std::forward<TArgs>(args)...);
    }

    template<typename Ty>
    requires std::is_base_of<vertex_buffer, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_vertex() noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty, typename... TArgs>
    requires std::is_base_of<vertex_buffer, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_vertex(TArgs... args) noexcept{
        return std::make_shared<Ty>(std::forward<TArgs>(args)...);
    }

    template<typename Ty>
    requires std::is_base_of<index_buffer, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_index() noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty, typename... TArgs>
    requires std::is_base_of<index_buffer, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_index(TArgs... args) noexcept{
        return std::make_shared<Ty>(std::forward<TArgs>(args)...);
    }

    template<typename Ty>
    requires std::is_base_of<vertex_array, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_varray() noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty, typename... TArgs>
    requires std::is_base_of<vertex_array, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_varray(TArgs... args) noexcept{
        return std::make_shared<Ty>(std::forward<TArgs>(args)...);
    }

    template<typename Ty>
    requires std::is_base_of<texture, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_texture() noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty, typename... TArgs>
    requires std::is_base_of<texture, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_texture(TArgs... args) noexcept{
        return std::make_shared<Ty>(std::forward<TArgs>(args)...);
    }

    template<typename Ty>
    requires std::is_base_of<shader, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_shader() noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty, typename... TArgs>
    requires std::is_base_of<shader, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_shader(TArgs... args) noexcept{
        return std::make_shared<Ty>(std::forward<TArgs>(args)...);
    }
}
