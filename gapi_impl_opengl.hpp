#pragma once

#include <GL/glew.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef APIENTRY
#undef APIENTRY
#endif

#include <stb_image.h>
#include "gapi.hpp"

namespace gapi::opengl{

    enum DRAW : GLenum {

        DRAW_STATIC     = GL_STATIC_DRAW,
        DRAW_DYNAMIC    = GL_DYNAMIC_DRAW,
        DRAW_STREAM     = GL_STREAM_DRAW
    };

    enum DATA_TYPE : GLenum {

        BYTE            = GL_BYTE,
        UBYTE           = GL_UNSIGNED_BYTE,
        SHORT           = GL_SHORT,
        USHORT          = GL_UNSIGNED_SHORT,
        INT             = GL_INT,
        UINT            = GL_UNSIGNED_INT,
        FLOAT           = GL_FLOAT,
        DOUBLE          = GL_DOUBLE
    };

    enum DRAW_MODE : GLenum {

        DRAW_NONE            = GL_NONE,
        DRAW_POINTS          = GL_POINTS,
        DRAW_LINES           = GL_LINES,
        DRAW_LINE_STRIP      = GL_LINE_STRIP,
        DRAW_LINE_LOOP       = GL_LINE_LOOP,
        DRAW_TRIANGLES       = GL_TRIANGLES,
        DRAW_TRIANGLE_STRIP  = GL_TRIANGLE_STRIP,
        DRAW_TRIANGLE_FAN    = GL_TRIANGLE_FAN
    };

    enum SHADER_TYPE : GLenum {

        SHADER_NONE            = GL_NONE,
        SHADER_VERTEX          = GL_VERTEX_SHADER,
        SHADER_FRAGMENT        = GL_FRAGMENT_SHADER,
        SHADER_GEOMETRY        = GL_GEOMETRY_SHADER
    };

    enum TEXTURE_TYPE : GLenum {

        TEXTURE_NONE            = GL_NONE,
        TEXTURE_2D              = GL_TEXTURE_2D,
        TEXTURE_2D_ARRAY        = GL_TEXTURE_2D_ARRAY,
        TEXTURE_3D              = GL_TEXTURE_3D,
        TEXTURE_CUBE_MAP        = GL_TEXTURE_CUBE_MAP
    };

    enum TEXTURE_FILTER : GLenum {

        TEX_FILTER_NEAREST         = GL_NEAREST,
        TEX_FILTER_LINEAR          = GL_LINEAR,
        TEX_FILTER_NEAREST_MIPMAP  = GL_NEAREST_MIPMAP_NEAREST,
        TEX_FILTER_LINEAR_MIPMAP   = GL_LINEAR_MIPMAP_LINEAR
    };

    enum TEXTURE_WRAP : GLenum {

        TEX_WRAP_CLAMP           = GL_CLAMP_TO_EDGE,
        TEX_WRAP_REPEAT          = GL_REPEAT,
        TEX_WRAP_MIRRORED_REPEAT = GL_MIRRORED_REPEAT
    };


    class info final : public gapi::info{

        public:
            info();
            virtual ~info() = default;

            inline virtual const std::string& vendor() const override   { return m_vendor;          }
            inline virtual const std::string& renderer() const override { return m_renderer;        }
            inline virtual const std::string& version() const override  { return m_version;         }
            inline virtual const std::string& language() const override { return m_glsl_version;    }
        
        private:
            std::string m_vendor;
            std::string m_renderer;
            std::string m_version;
            std::string m_glsl_version;
    };

    class context final : public gapi::context{

        public:
            context() {}
            context(GLFWwindow* window): m_window(window) { }
            virtual ~context() = default;

            virtual bool init() override;
            virtual void swap() override;
            virtual void interval(uint32_t interval) override;
            inline const std::shared_ptr<gapi::opengl::info>& info() const { return m_info; }

        private:
            GLFWwindow* m_window{nullptr};
            std::shared_ptr<gapi::opengl::info> m_info{nullptr};
    };

    class vertex_buffer final : public gapi::vertex_buffer {

        public:
            vertex_buffer(float* v, uint32_t s, DRAW t);
            virtual ~vertex_buffer();

            virtual void bind() const override;
            virtual void unbind() const override;
            virtual void configure_layout(const gapi::buffer_layout& layout) override { m_layout = layout; };
            virtual const gapi::buffer_layout& layout() const override { return m_layout; };

        private:
            uint32_t m_id{0};
            gapi::buffer_layout m_layout{};
    };

    class index_buffer final : public gapi::index_buffer {

        public:
            index_buffer(uint32_t* i, size_t c, DRAW t);
            virtual ~index_buffer();

            void bind() const override;
            void unbind() const override;
            inline size_t count() const override { return m_count; }

        private:
            uint32_t m_id{0};
            size_t m_count{0};
    };

    class vertex_array final : public gapi::vertex_array {

        public:
            vertex_array();
            virtual ~vertex_array();

            void bind() const override;
            void unbind() const override;
            void emplace_vertex(const std::shared_ptr<gapi::vertex_buffer>& vb) override;
            void emplace_index(const std::shared_ptr<gapi::index_buffer>& ib) override;
            inline const std::vector<std::shared_ptr<gapi::vertex_buffer>>& vertexs() const override { return m_vertex_buffers; }
            inline const std::shared_ptr<gapi::index_buffer>& index() const override { return m_index_buffer; }

        private:
            uint32_t m_id{0};
            std::vector<std::shared_ptr<gapi::vertex_buffer>> m_vertex_buffers{};
            std::shared_ptr<gapi::index_buffer> m_index_buffer{};
    };

    class shader final : public gapi::shader {

        private:
            uint32_t validator(const std::string& n) const;
            void compile(std::unordered_map<SHADER_TYPE, std::string> sources);
            std::string read_file(const std::filesystem::path& file_path) const;
            std::unordered_map<SHADER_TYPE, std::string> pre_process(const std::string& src) const;

        public:
            shader(const std::string& sname, const std::filesystem::path& path);
            shader(const std::string& sname, const std::filesystem::path& vertex, const std::filesystem::path& fragment);
            virtual ~shader();

            void bind() const override;
            void unbind() const override;
            inline virtual const std::string& name() const override { return m_name; }

            virtual bool uniform(const std::string& n, uint32_t v) const override;
            virtual bool uniform(const std::string& n, float v) const override;
            virtual bool uniform(const std::string& n, float x, float y) const override;
            virtual bool uniform(const std::string& n, float x, float y, float z) const override;
            virtual bool uniform(const std::string& n, float x, float y, float z, float w) const override;
            virtual bool uniform(const std::string& n, const glm::vec2& v) const override;
            virtual bool uniform(const std::string& n, const glm::vec3& v) const override;
            virtual bool uniform(const std::string& n, const glm::vec4& v) const override;
            virtual bool uniform(const std::string& n, const glm::mat2& v) const override;
            virtual bool uniform(const std::string& n, const glm::mat3& v) const override;
            virtual bool uniform(const std::string& n, const glm::mat4& v) const override;
            inline uint32_t id() const { return m_id; }
            inline uint32_t uniformloc(const std::string& n) const { return glGetUniformLocation(m_id, n.c_str()); }

        private:
            uint32_t m_id{0};
            std::string m_name{};
    };

    class texture_2d final : public gapi::texture {

        public:
            texture_2d(std::filesystem::path path, TEXTURE_FILTER filter, TEXTURE_WRAP wrap,  bool flip = true);
            virtual ~texture_2d();

            virtual void bind(uint32_t slot = 0) const override;
            [[maybe_unused]] virtual void unbind() const override;

            [[maybe_unused]] virtual uint32_t id() const override { return m_id; }
            [[maybe_unused]] virtual int32_t width() const override { return m_width; }
            [[maybe_unused]] virtual int32_t height() const override { return m_height; }
            [[maybe_unused]] virtual int32_t channels() const override { return m_channels; }
            [[maybe_unused]] virtual uint8_t* data() const override { return m_data; }
        
        private:
            int32_t m_width{0};
            int32_t m_height{0};
            int32_t m_channels{0};
            uint32_t m_id{0};
            std::string m_path{};
            uint8_t* m_data{nullptr};
            TEXTURE_TYPE m_type{TEXTURE_2D};
    };

    class api final : public gapi::base_api {

        public:
            api() = default;
            virtual ~api() = default;

            virtual void init() override;
            virtual void draw(const std::shared_ptr<gapi::vertex_array>& va) override;
            virtual void clear() override;
            virtual void clear_color(float r, float g, float b, float a) override;
            virtual GAPI xapi() const override { return gapi::GAPI::OPENGL; }
    };

    [[nodiscard]] std::shared_ptr<context> make_context(GLFWwindow* window) noexcept;
    [[nodiscard]] std::shared_ptr<vertex_buffer> make_vertex(float* v, uint32_t s, DRAW t) noexcept;
    [[nodiscard]] std::shared_ptr<index_buffer> make_index(uint32_t* i, size_t c, DRAW t) noexcept;
    [[nodiscard]] std::shared_ptr<vertex_array> make_array() noexcept;
    [[nodiscard]] std::shared_ptr<texture_2d> make_texture2d(std::filesystem::path path, TEXTURE_FILTER filter, TEXTURE_WRAP wrap,  bool flip = true) noexcept;
    [[nodiscard]] std::shared_ptr<shader> make_shader(const std::string& sname, const std::filesystem::path& path) noexcept;
    [[nodiscard]] std::shared_ptr<shader> make_shader(const std::string& sname, const std::filesystem::path& vertex, const std::filesystem::path& fragment) noexcept;
    
}

namespace ggl = gapi::opengl;