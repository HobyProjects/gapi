#pragma once

#include <GL/glew.h>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#ifdef APIENTRY
#undef APIENTRY
#endif

#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>

#ifdef _DEBUG

#include <iostream>
struct gl_error_message{

    gl_error_message() {}
    gl_error_message(size_t error, const char* str, const char* file, int line): 
        error(error), enum_str(str), file(file), line(line){}
    ~gl_error_message() = default;

    inline const char* str() const noexcept { 

        const char* error_str{nullptr};
        switch(error){
            case GL_INVALID_ENUM:                       error_str = "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
            case GL_INVALID_VALUE:                      error_str = "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
            case GL_INVALID_OPERATION:                  error_str = "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
            case GL_STACK_OVERFLOW:                     error_str = "An attempt has been made to perform an operation that would cause an internal stack to overflow.";
            case GL_STACK_UNDERFLOW:                    error_str = "An attempt has been made to perform an operation that would cause an internal stack to underflow.";
            case GL_OUT_OF_MEMORY:                      error_str = "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
            case GL_INVALID_FRAMEBUFFER_OPERATION:      error_str = "The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.";
            default:                                    error_str = "UNKNOWN";
        }

        std::stringstream ss;
        ss << "[GL-ERROR]:[" << enum_str << "]: " << error_str << " at (" << file << ":" << line << ")";
        return ss.str().c_str();
    }

    size_t error{0};
    const char* enum_str{nullptr};
    const char* file{nullptr};
    int line{0};
};

inline gl_error_message get_error_message(size_t error){
    switch(error){
        case GL_INVALID_ENUM:                       return {GL_INVALID_ENUM, "GL_INVALID_ENUM", __FILE__, __LINE__};
        case GL_INVALID_VALUE:                      return {GL_INVALID_VALUE, "GL_INVALID_VALUE", __FILE__, __LINE__};
        case GL_INVALID_OPERATION:                  return {GL_INVALID_OPERATION, "GL_INVALID_OPERATION", __FILE__, __LINE__};
        case GL_STACK_OVERFLOW:                     return {GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW", __FILE__, __LINE__};
        case GL_STACK_UNDERFLOW:                    return {GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW", __FILE__, __LINE__};
        case GL_OUT_OF_MEMORY:                      return {GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY", __FILE__, __LINE__};
        case GL_INVALID_FRAMEBUFFER_OPERATION:      return {GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION", __FILE__, __LINE__};
        default:                                    return {error, "UNKNOWN", __FILE__, __LINE__};
    }
}

inline bool gl_check_errors(const char* file, int line){
    GLenum error{0};
    gl_error_message msg;
    while((error = glGetError()) != GL_NO_ERROR){
        msg = get_error_message(error);
    }

    if(msg.error != GL_NO_ERROR){
        std::cerr << msg.str() << '\n';
        return true;
    }

    return false;
}

#define gl(gl_func) gl_func; if(gl_check_errors(__FILE__, __LINE__)) { gapi_debugbreak(); }
#else
#define gl(gl_func) gl_func
#endif

#include "gapi.hpp"
namespace gapi::opengl{

    class GAPI gl_info final : public api_info{

        public:
            gl_info();
            virtual ~gl_info() = default;

            inline virtual const std::string& vendor() const noexcept override { return m_vendor; }
            inline virtual const std::string& renderer() const noexcept override { return m_renderer; }
            inline virtual const std::string& version() const noexcept override { return m_version; }
            inline virtual const std::string& language() const noexcept override { return m_glsl_version; }
        
        private:
            std::string m_vendor;
            std::string m_renderer;
            std::string m_version;
            std::string m_glsl_version;
    };

    class GAPI gl_context final : public context{

        public:
            gl_context(GLFWwindow* window): m_window(window) {}
            virtual ~gl_context() = default;

            virtual bool init() noexcept override;
            virtual void swap() noexcept override;
            virtual void interval(uint32_t interval) noexcept override;

            inline std::shared_ptr<gl_info> info() const { return m_info; }

        private:
            GLFWwindow* m_window{nullptr};
            std::shared_ptr<gl_info> m_info{nullptr};
    };

    enum gl_draw_type : GLenum {
        static_daw      = GL_STATIC_DRAW,
        dynamic_draw    = GL_DYNAMIC_DRAW,
        stream_draw     = GL_STREAM_DRAW
    };

    enum gl_data_type : GLenum {
        byte            = GL_BYTE,
        ubyte           = GL_UNSIGNED_BYTE,
        short_          = GL_SHORT,
        ushort          = GL_UNSIGNED_SHORT,
        int_            = GL_INT,
        uint            = GL_UNSIGNED_INT,
        float_          = GL_FLOAT,
        double_         = GL_DOUBLE
    };

    enum gl_draw_mode : GLenum {
        points          = GL_POINTS,
        lines           = GL_LINES,
        line_strip      = GL_LINE_STRIP,
        line_loop       = GL_LINE_LOOP,
        triangles       = GL_TRIANGLES,
        triangle_strip  = GL_TRIANGLE_STRIP,
        triangle_fan    = GL_TRIANGLE_FAN
    };

    class GAPI gl_vertex_buffer final : public vertex_buffer {

        public:
            gl_vertex_buffer(float* v, size_t s, gl_draw_type t);
            virtual ~gl_vertex_buffer();

            void bind() const noexcept;
            void unbind() const noexcept;

            inline virtual const buffer_layout& get_layout() const noexcept { return m_layout; }
            inline virtual void set_layout(const buffer_layout& layout) noexcept { m_layout = layout; }

        private:
            uint32_t m_id{0};
            buffer_layout m_layout;
    };

    class GAPI gl_index_buffer final : public index_buffer {

        public:
            gl_index_buffer(uint32_t* i, size_t c, gl_draw_type t);
            virtual ~gl_index_buffer();

            void bind() const noexcept;
            void unbind() const noexcept;

            inline size_t count() const noexcept override { return m_count; }

        private:
            uint32_t m_id{0};
            size_t m_count{0};
    };

    class GAPI gl_vertex_array final : public vertex_array {

        public:
            gl_vertex_array();
            virtual ~gl_vertex_array();

            void bind() const noexcept;
            void unbind() const noexcept;

            void emplace_vbuffer(const std::shared_ptr<vertex_buffer>& vb) noexcept override;
            void emplace_ibuffer(const std::shared_ptr<index_buffer>& ib) noexcept override;

            inline const std::vector<std::shared_ptr<vertex_buffer>>& vertexs() const noexcept { return m_vertex_buffers; }
            inline const std::shared_ptr<index_buffer>& index() const noexcept { return m_index_buffer; }

        private:
            uint32_t m_id{0};
            std::vector<std::shared_ptr<vertex_buffer>> m_vertex_buffers{};
            std::shared_ptr<index_buffer> m_index_buffer{};
    };

    enum class gl_shader_type : GLenum {
        none            = 0,
        vertex          = GL_VERTEX_SHADER,
        fragment        = GL_FRAGMENT_SHADER,
        geometry        = GL_GEOMETRY_SHADER
    };

    class GAPI gl_shader final : public shader {

        public:
            gl_shader(const std::string& sname, const std::filesystem::path& path);
            gl_shader(const std::string& sname, const std::filesystem::path& vertex, const std::filesystem::path& fragment);
            virtual ~gl_shader();

            void bind() const noexcept;
            void unbind() const noexcept;

            virtual bool uniform(const std::string& n, uint32_t v) const noexcept override;
            virtual bool uniform(const std::string& n, float v) const noexcept override;
            virtual bool uniform(const std::string& n, float x, float y) const noexcept override;
            virtual bool uniform(const std::string& n, float x, float y, float z) const noexcept override;
            virtual bool uniform(const std::string& n, float x, float y, float z, float w) const noexcept override;
            virtual bool uniform(const std::string& n, const glm::vec2& v) const noexcept override;
            virtual bool uniform(const std::string& n, const glm::vec3& v) const noexcept override;
            virtual bool uniform(const std::string& n, const glm::vec4& v) const noexcept override;
            virtual bool uniform(const std::string& n, const glm::mat2& v) const noexcept override;
            virtual bool uniform(const std::string& n, const glm::mat3& v) const noexcept override;
            virtual bool uniform(const std::string& n, const glm::mat4& v) const noexcept override;

            inline uint32_t id() const { return m_id; }
            inline uint32_t uniformloc(const std::string& n) const noexcept { return glGetUniformLocation(m_id, n.c_str()); }

        private:
            uint32_t validator(const std::string& n) const noexcept;
            void compile(std::unordered_map<gl_shader_type, std::string> sources);
            std::string read_file(const std::filesystem::path& file_path) const;
            std::unordered_map<gl_shader_type, std::string> pre_process(const std::string& src) const;

        private:
            uint32_t m_id{0};
            std::string m_name{};
    };
}