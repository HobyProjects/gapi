#pragma once

#include <memory>
#include <type_traits>
#include <string>
#include <initializer_list>
#include <vector>
#include <algorithm>
#include <cassert>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

#if defined(_WIN32) || defined(_WIN64)
#define GAPI_PLATFORM_WINDOWS
#if defined(GAPI_STATIC)
#define GAPI /* no exports or imports, library building static */        
#elif defined(GAPI_SHARED)
#ifdef GAPI_SHARED
#define GAPI __declspec(dllexport)
#else
#define GAPI __declspec(dllimport)
#endif
#endif  
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
#if defined(GAPI_PLATFORM_WINDOWS)
#define gapi_debugbreak() __debugbreak()
#elif defined(GAPI_PLATFORM_LINUX)
#include <signal.h>
#define gapi_debugbreak() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define gapi_assert(exp, msg) cassert(exp, msg)
#else
#define gapi_asserts(exp, msg)
#endif

namespace gapi{

    class GAPI context{
        public:
            context() = default;
            context(const context&) = delete;
            context& operator=(const context&) = delete;
            context(context&&) = delete;
            context& operator=(context&&) = delete;
            virtual ~context() = default;

            virtual bool init() const noexcept = 0;
            virtual void swap() const noexcept = 0;
            virtual void interval(uint32_t interval) const noexcept = 0;
    };

    template<typename Ty>
    requires std::is_base_of<context, Ty>::value
    [[nodiscard]] GAPI std::shared_ptr<Ty> make_context() noexcept{
        return std::make_shared<Ty>();
    }

    enum class draw : size_t{
        static_draw     = 0, 
        dynamic_draw    = 1, 
    };

    enum class component : size_t{
        xyz     = 3,    xyzw    = 4,
        rgb     = 3,    rgba    = 4,
        xy      = 2,    uv      = 2,    none    = 0,
    };

    enum class data_types : size_t{
        null    = 0,    f1      = 4,    f2      = 8,    f3      = 12,  
        f4      = 16,   i1      = 4,    i2      = 8,    i3      = 12,
        i4      = 16,   ui1     = 4,    ui2     = 8,    ui3     = 12, 
        ui4     = 16,   mat2    = 16,   mat3    = 36,   mat4    = 64,   boolean  = 1
    };

    struct GAPI buffer_elements{
        buffer_elements() {}
        buffer_elements(const std::string& name, component comp, data_types type, size_t size, bool normalized = false) noexcept
            : name(name), component(comp), type(type), size(size), normalized(normalized) {}
        ~buffer_elements() = default;

        std::string name{};
        component component{component::none};
        data_types type{data_types::null};
        size_t size{0};
        size_t offset{0}; 
        bool normalized{false};
    };

    class GAPI buffer_layout{
        public:
            buffer_layout(){}
            buffer_layout(std::initializer_list<buffer_elements> elements) noexcept
                : m_elements(elements) { _stride(); }
            ~buffer_layout() = default;

            [[nodiscard]] inline size_t stride() const noexcept { return m_stride; }
            [[nodiscard]] inline const std::vector<buffer_elements>& elements() const noexcept { return m_elements; }

            inline std::vector<buffer_elements>::iterator begin() noexcept { return m_elements.begin(); }
            inline std::vector<buffer_elements>::iterator end() noexcept { return m_elements.end(); }
            inline std::vector<buffer_elements>::const_iterator begin() const noexcept { return m_elements.begin(); }
            inline std::vector<buffer_elements>::const_iterator end() const noexcept { return m_elements.end(); }

        private:
            inline void _stride() noexcept{
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

    class GAPI vertex_buffer{
        public:
            constexpr vertex_buffer() = default;
            virtual ~vertex_buffer() = default;

            virtual void bind() const noexcept = 0;
            virtual void unbind() const noexcept = 0;
            virtual void configure_layout(const buffer_layout& layout) const noexcept = 0;
            virtual const buffer_layout& layout() const noexcept = 0;
    };

    class GAPI index_buffer{
        public:
            index_buffer() = default;
            virtual ~index_buffer() = default;

            virtual void bind() const noexcept = 0;
            virtual void unbind() const noexcept = 0;
            virtual size_t count() const noexcept = 0;

        protected:
            size_t m_count{0};
    };

    template<typename Ty>
    requires std::is_base_of<vertex_buffer, Ty>::value
    [[nodiscard]] GAPI std::shared_ptr<Ty> make_vertex() noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty, typename... TArgs>
    requires std::is_base_of<vertex_buffer, Ty>::value
    [[nodiscard]] GAPI std::shared_ptr<Ty> make_vertex(TArgs... args) noexcept{
        return std::make_shared<Ty>(std::forward<TArgs>(args)...);
    }

    template<typename Ty>
    requires std::is_base_of<index_buffer, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_index() noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty, typename... TArgs>
    requires std::is_base_of<index_buffer, Ty>::value
    [[nodiscard]] GAPI std::shared_ptr<Ty> make_index(TArgs... args) noexcept{
        return std::make_shared<Ty>(std::forward<TArgs>(args)...);
    }

    class GAPI vertex_array{
        public:
            vertex_array() = default;
            virtual ~vertex_array() = default;

            virtual void bind() const noexcept = 0;
            virtual void unbind() const noexcept = 0;

            virtual void emplace_vbuffer(const std::shared_ptr<vertex_buffer>& vertex_buffer) noexcept = 0;
            virtual void emplace_ibuffer(const std::shared_ptr<index_buffer>& index_buffer) noexcept = 0;

        private:
            std::vector<std::shared_ptr<vertex_buffer>> m_vertex_buffers{};
            std::vector<std::shared_ptr<index_buffer>> m_index_buffer{};
    };

    template<typename Ty>
    requires std::is_base_of<vertex_array, Ty>::value
    [[nodiscard]] GAPI std::shared_ptr<Ty> make_varray() noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty, typename... TArgs>
    requires std::is_base_of<vertex_array, Ty>::value
    [[nodiscard]] GAPI std::shared_ptr<Ty> make_varray(TArgs... args) noexcept{
        return std::make_shared<Ty>(std::forward<TArgs>(args)...);
    }

    class GAPI shader{
        public:
            shader() = default;
            virtual ~shader() = default;

            virtual void bind() const noexcept = 0;
            virtual void unbind() const noexcept = 0;

            virtual bool uniform(const std::string& n, uint32_t v) const noexcept = 0;
            virtual bool uniform(const std::string& n, float v) const noexcept = 0;
            virtual bool uniform(const std::string& n, float x, float y) const noexcept = 0;
            virtual bool uniform(const std::string& n, float x, float y, float z) const noexcept = 0;
            virtual bool uniform(const std::string* n, float x, float y, float z, float w) const noexcept = 0;
            virtual bool uniform(const std::string& n, const glm::vec2& v) const noexcept = 0;
            virtual bool uniform(const std::string& n, const glm::vec3& v) const noexcept = 0;
            virtual bool uniform(const std::string& n, const glm::vec4& v) const noexcept = 0;
            virtual bool uniform(const std::string& n, const glm::mat2& v) const noexcept = 0;
            virtual bool uniform(const std::string& n, const glm::mat3& v) const noexcept = 0;
            virtual bool uniform(const std::string& n, const glm::mat4& v) const noexcept = 0;
    };

    template<typename Ty>
    requires std::is_base_of<shader, Ty>::value
    [[nodiscard]] GAPI std::shared_ptr<Ty> make_shader() noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty, typename... TArgs>
    requires std::is_base_of<shader, Ty>::value
    [[nodiscard]] GAPI std::shared_ptr<Ty> make_shader(TArgs... args) noexcept{
        return std::make_shared<Ty>(std::forward<TArgs>(args)...);
    }

    enum class texture_type : size_t{
        none = 0,    diffuse = 1,    specular = 2,    normal = 3,    height = 4,   ambient = 5
    };

    class GAPI texture{
        public:
            texture() = default;
            virtual ~texture() = default;

            virtual void bind(uint32_t slot = 0) const noexcept = 0;
            virtual void unbind() const noexcept = 0;

            virtual void* data() const noexcept = 0;
            virtual void slot() const noexcept = 0;
            virtual void width() const noexcept = 0;
            virtual void height() const noexcept = 0;
            virtual void channels() const noexcept = 0;
            virtual texture_type type() const noexcept = 0;
    };

    template<typename Ty>
    requires std::is_base_of<texture, Ty>::value
    [[nodiscard]] GAPI std::shared_ptr<Ty> make_texture() noexcept{
        return std::make_shared<Ty>();
    }

    template<typename Ty, typename... TArgs>
    requires std::is_base_of<texture, Ty>::value
    [[nodiscard]] GAPI std::shared_ptr<Ty> make_texture(TArgs... args) noexcept{
        return std::make_shared<Ty>(std::forward<TArgs>(args)...);
    }
}