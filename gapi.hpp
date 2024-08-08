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

#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstddef>

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
#include <iostream>
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

    enum COMPOENENT : int32_t{
        XYZ     = 3,    XYZW    = 4,
        RGB     = 3,    RGBA    = 4,
        XY      = 2,    UV      = 2,    NONE    = 0,
    };

    enum DATA : uint32_t{
        BOOL    = 1,    F1      = 4,    F2      = 8,    F3      = 12,  
        F4      = 16,   I1      = 4,    I2      = 8,    I3      = 12,
        I4      = 16,   UI1     = 4,    UI2     = 8,    UI3     = 12, 
        UI4     = 16,   MAT2    = 16,   MAT3    = 36,   MAT4    = 64  
    };

    enum class GAPI : uint32_t{
        SYSTEM = 0, OPENGL = 1, DIRECTX = 2, VULKAN = 3, METAL = 4
    };


    class info {

        public:
            info() = default;
            virtual ~info() = default;

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
        buffer_elements(const std::string& name, COMPOENENT comp, uint32_t size, bool normalized = false) noexcept
            : name(name), component(comp), size(size), normalized(normalized) {}
        ~buffer_elements() = default;

        std::string name{};
        COMPOENENT component{COMPOENENT::NONE};
        uint32_t size{0};
        uint32_t offset{0}; 
        bool normalized{false};
    };

    class buffer_layout{
        public:
            buffer_layout(){}
            buffer_layout(std::initializer_list<buffer_elements> elements)
                : m_elements(elements) { _stride(); }
            ~buffer_layout() = default;

            [[nodiscard]] inline uint32_t stride() const { return m_stride; }
            [[nodiscard]] inline const std::vector<buffer_elements>& elements() const { return m_elements; }

            inline std::vector<buffer_elements>::iterator begin() { return m_elements.begin(); }
            inline std::vector<buffer_elements>::iterator end() { return m_elements.end(); }
            inline std::vector<buffer_elements>::const_iterator begin() const { return m_elements.begin(); }
            inline std::vector<buffer_elements>::const_iterator end() const { return m_elements.end(); }

        private:
            inline void _stride(){
                m_stride = 0;
                uint32_t offset = 0;
                for (auto &element : m_elements)
                {
                    element.offset = offset;
                    offset += element.size;
                    m_stride += element.size; 
                }
            }

        private:
            std::vector<buffer_elements> m_elements{};
            uint32_t m_stride{0};
    };

    class vertex_buffer{
        public:
            constexpr vertex_buffer() = default;
            virtual ~vertex_buffer() = default;

            virtual void bind() const = 0;
            [[maybe_unused]] virtual void unbind() const = 0;

            virtual void configure_layout(const buffer_layout& layout) = 0;
            virtual const buffer_layout& layout() const = 0;
    };

    class index_buffer{
        public:
            index_buffer() = default;
            virtual ~index_buffer() = default;

            virtual void bind() const = 0;
            [[maybe_unused]] virtual void unbind() const = 0;
            virtual uint32_t count() const = 0;
    };

    class vertex_array{

        public:
            vertex_array() = default;
            virtual ~vertex_array() = default;

            virtual void bind() const = 0;
            [[maybe_unused]] virtual void unbind() const = 0;

            virtual void emplace_vertex(const std::shared_ptr<vertex_buffer>& vertex_buffer)  = 0;
            virtual void emplace_index(const std::shared_ptr<index_buffer>& index_buffer) = 0;
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
            [[maybe_unused]] virtual uint8_t* data() const = 0;

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
            virtual GAPI xapi() const  = 0;   
    };

    class shader_container{

        public:
            shader_container() = default;
            ~shader_container() = default;

            void emplace(const std::shared_ptr<shader>& shader){
                auto& shader_name = shader->name();
                gapi_asserts(m_shaders.find(shader_name) == m_shaders.end(), "Shader already exists");
                m_shaders[shader_name] = shader;
            }

            template<typename Ty, typename... TArgs>
            [[nodiscard]] std::shared_ptr<shader> load(TArgs... args) const {
                auto shader = make_shader<Ty>(std::forward<TArgs>(args)...);
                emplace(shader);
                return shader;
            }

            [[nodiscard]] std::shared_ptr<shader> get(const std::string& name) const{
                auto it = m_shaders.find(name);
                gapi_asserts(it != m_shaders.end(), "Shader not found");
                return it->second;
            }

        private:
            std::unordered_map<std::string, std::shared_ptr<shader>> m_shaders{};
    };
}
