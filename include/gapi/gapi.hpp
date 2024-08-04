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
#if defined(GAPI_PLATFORM_WINDOWS)
#define gapi_debugbreak() __debugbreak()
#define gapi_debug_msg(cap, msg) std::cerr << cap << msg << "\n"
#elif defined(GAPI_PLATFORM_LINUX)
#include <signal.h>
#define gapi_debugbreak() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define gapi_assert(exp, msg) assert(exp && msg)
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
            base_api(_gapi api) : m_api(api) {};
            virtual ~base_api() = default;

            virtual void init() = 0;
            virtual void draw(const std::shared_ptr<vertex_array>& va) = 0;
            virtual void clear()  = 0;
            virtual void clear_color(float r, float g, float b, float a) = 0;
            inline _gapi api() { return m_api; };

        protected:
            _gapi m_api{_gapi::none};       
    };

}

#if !defined(__glew_h__) || !defined(__GLEW_H__)
#include <GL/glew.h>
#endif

#ifndef _glfw3_h_
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif
#endif

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
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

    inline const char* str() const { 

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
namespace gapi::opengl{

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

    enum class gl_shader_type : GLenum {
        none            = 0,
        vertex          = GL_VERTEX_SHADER,
        fragment        = GL_FRAGMENT_SHADER,
        geometry        = GL_GEOMETRY_SHADER
    };


    class gl_info final : public api_info{
        public:
            gl_info(){
                m_version       = reinterpret_cast<const char*>(glGetString(GL_VERSION));
                m_vendor        = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
                m_renderer      = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
                m_glsl_version  = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

                char GL_MAJOR = m_version[0];
                char GL_MINOR = m_version[2];
                char GL_PATCH = m_version[4];

                std::stringstream ss;
                ss << "#version " << GL_MAJOR << GL_MINOR << GL_PATCH << " core";
                m_glsl_version = ss.str();
            }

            virtual ~gl_info() = default;

            inline virtual const std::string& vendor() const override { return m_vendor; }
            inline virtual const std::string& renderer() const override { return m_renderer; }
            inline virtual const std::string& version() const override { return m_version; }
            inline virtual const std::string& language() const override { return m_glsl_version; }
        
        private:
            std::string m_vendor;
            std::string m_renderer;
            std::string m_version;
            std::string m_glsl_version;
    };

    class gl_context final : public context{
        public:
            gl_context(GLFWwindow* window): m_window(window) {}
            virtual ~gl_context() = default;

            virtual bool init() override {
                gapi_assert(m_window != nullptr, "Window is nullptr");
                glfwMakeContextCurrent(m_window);
                glfwSwapInterval(1);

                glewExperimental = GL_TRUE;
                GLenum status = glewInit();
                gapi_assert(status != GLEW_OK, "Failed to initialize GLEW");

                m_info = std::make_shared<gl_info>();
                return true;
            }

            virtual void swap() override{
                gapi_assert(m_window != nullptr, "Window is nullptr");
                glfwSwapBuffers(m_window);
            }

            virtual void interval(uint32_t interval) override{
                gapi_assert(m_window != nullptr, "Window is nullptr");
                glfwSwapInterval(interval);
            }

            inline const std::shared_ptr<gl_info>& info() const { return m_info; }

        private:
            GLFWwindow* m_window{nullptr};
            std::shared_ptr<gl_info> m_info{nullptr};
    };

    class gl_vertex_buffer final : public vertex_buffer {
        public:
            gl_vertex_buffer(float* v, size_t s, gl_draw_type t){
                gl(glGenBuffers(1, &m_id));
                gl(glBindBuffer(GL_ARRAY_BUFFER, m_id));
                gl(glBufferData(GL_ARRAY_BUFFER, s, v, static_cast<GLenum>(t)));
            }

            virtual ~gl_vertex_buffer(){
                gl(glDeleteBuffers(1, &m_id));
            }

            void bind() const override{
                gl(glBindBuffer(GL_ARRAY_BUFFER, m_id));
            }

            void unbind() const override{
                gl(glBindBuffer(GL_ARRAY_BUFFER, 0));
            }

            virtual inline const buffer_layout& get_layout() const { return m_layout; }
            virtual inline void set_layout(const buffer_layout& layout) { m_layout = layout; }

        private:
            uint32_t m_id{0};
            buffer_layout m_layout;
    };

    class gl_index_buffer final : public index_buffer {
        public:
            gl_index_buffer(uint32_t* i, size_t c, gl_draw_type t){
                gl(glGenBuffers(1, &m_id));
                gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
                gl(glBufferData(GL_ELEMENT_ARRAY_BUFFER, c, i, static_cast<GLenum>(t)));
            }

            virtual ~gl_index_buffer(){
                gl(glDeleteBuffers(1, &m_id));
            }

            void bind() const override{
                gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
            }

            void unbind() const override{
                gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            }

            inline size_t count() const override { return m_count; }

        private:
            uint32_t m_id{0};
            size_t m_count{0};
    };

    class gl_vertex_array final : public vertex_array {
        public:
            gl_vertex_array(){
                gl(glGenVertexArrays(1, &m_id));
            }

            virtual ~gl_vertex_array(){
                gl(glDeleteVertexArrays(1, &m_id));
            }

            void bind() const override{
                gl(glBindVertexArray(m_id));
            }

            void unbind() const override{
                gl(glBindVertexArray(0));
            }

            void emplace_vbuffer(const std::shared_ptr<vertex_buffer>& vb) override{
                vb->bind();
                const auto& layout = vb->layout();
                const auto& elements = layout.elements();
                size_t index = 0;
                for(const auto& element : elements)
                {
                    gl(glEnableVertexAttribArray(index));
                    gl(glVertexAttribPointer(index, element.component, gl_data_type::float_, element.normalized, layout.stride(), (const void*)(element.offset)));
                    index++;
                }
                m_vertex_buffers.emplace_back(vb);
            }

            void emplace_ibuffer(const std::shared_ptr<index_buffer>& ib) override{
                ib->bind();
                m_index_buffer = ib;
            }

            inline const std::vector<std::shared_ptr<vertex_buffer>>& vertexs() const override { return m_vertex_buffers; }
            inline const std::shared_ptr<index_buffer>& index() const override { return m_index_buffer; }

        private:
            uint32_t m_id{0};
            std::vector<std::shared_ptr<vertex_buffer>> m_vertex_buffers{};
            std::shared_ptr<index_buffer> m_index_buffer{};
    };

    class gl_shader final : public shader {
        private:
            uint32_t validator(const std::string& n) const{
                uint32_t uniform_location = gl(glGetUniformLocation(m_id, n.c_str()));
                if(uniform_location == -1) {
                    gapi_debug_msg("Uniform not found: ", n);
                    return -1;
                }

                return uniform_location;
            }

            void compile(std::unordered_map<gl_shader_type, std::string> sources){
                uint32_t shader_program = gl(glCreateProgram());
                if(shader_program == GL_FALSE) return;

                std::vector<uint32_t> shaders;
                shaders.reserve(sources.size());

                for(auto& source : sources){
                    gl_shader_type type = source.first;
                    const std::string& src = source.second;

                    uint32_t shader_id = gl(glCreateShader(static_cast<GLenum>(type)));
                    const char* src_cstr = src.c_str();

                    gl(glShaderSource(shader_id, 1, &src_cstr, nullptr));
                    gl(glCompileShader(shader_id));

                    int32_t result{0};
                    gl(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result));

                    if(result == GL_FALSE){
                        int message_length = 0;
                        gl(glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &message_length));
                        std::vector<char> compile_error_message(message_length);
                        gl(glGetShaderInfoLog(shader_id, message_length, &message_length, &compile_error_message[0]));
                        gapi_debug_msg("Shader compilation error: ", compile_error_message.data());
                    }

                    gl(glAttachShader(shader_program, shader_id));
                    shaders.push_back(shader_id);
                }

                int result{0};
                gl(glLinkProgram(shader_program));
                gl(glGetProgramiv(shader_program, GL_LINK_STATUS, &result));

                if(result == GL_FALSE){
                    int message_length = 0;
                    gl(glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &message_length));
                    std::vector<char> link_error_message(message_length);
                    gl(glGetProgramInfoLog(shader_program, message_length, &message_length, &link_error_message[0]));
                    gapi_debug_msg("Shader linking error: ", link_error_message.data());
                }

                gl(glValidateProgram(shader_program));
                gl(glGetProgramiv(shader_program, GL_VALIDATE_STATUS, &result));

                if(result == GL_FALSE){
                    int message_length = 0;
                    gl(glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &message_length));
                    std::vector<char> validate_error_message(message_length);
                    gl(glGetProgramInfoLog(shader_program, message_length, &message_length, &validate_error_message[0]));
                    gapi_debug_msg("Shader validation error: ", validate_error_message.data());
                }

                for(auto& shader : shaders){
                    gl(glDetachShader(shader_program, shader));
                    gl(glDeleteShader(shader));
                }

                m_id = shader_program;
            }

            std::string read_file(const std::filesystem::path& file_path) const{
                std::string result;
                std::ifstream infile(file_path, std::ios::in | std::ios::binary);
                if(infile){
                    infile.seekg(0, std::ios::end);
                    result.resize(infile.tellg());
                    infile.seekg(0, std::ios::beg);
                    infile.read(&result[0], result.size());
                    infile.close();
                    return result;
                }

                gapi_debug_msg("Failed to read file: ", file_path.string());
                return "";
            }

            static gl_shader_type shader_type_from_string(const std::string &type){
                if(type == "vertex")                          return gl_shader_type::vertex;
                if(type == "fragment" || type == "pixel")     return gl_shader_type::fragment;
                if(type == "geometry")                        return gl_shader_type::geometry;
                gapi_assert(true, "Invalid shader type specified");
                return gl_shader_type::none;
            }

            std::unordered_map<gl_shader_type, std::string> pre_process(const std::string& src) const{
                std::unordered_map<gl_shader_type,std::string> shader_sources;
                const char *type_token = "#type";
                size_t type_token_length = strlen(type_token);
                size_t pos = src.find(type_token, 0);

                while(pos != std::string::npos){
                    size_t eol = src.find_first_of("\r\n", pos);
                    gapi_assert(eol == std::string::npos, "Syntax error, Did you forget to add shader type line #type declaration");
                    size_t begin = pos + type_token_length + 1;
                    std::string type = src.substr(begin, eol - begin);
                    gapi_assert(type != "vertex" && type != "fragment" && type != "pixel" && type != "geometry", "Invalid shader type specified");
                    size_t next_line_pos = src.find_first_not_of("\r\n", eol);
                    pos = src.find(type_token, next_line_pos);
                    shader_sources[shader_type_from_string(type)] = src.substr(next_line_pos, pos - (next_line_pos == std::string::npos ? src.size() - 1 : next_line_pos));
                }

                return shader_sources;
            }

        public:
            gl_shader(const std::string& sname, const std::filesystem::path& path){
                if(!std::filesystem::exists(path)){
                    gapi_debug_msg("Shader parse error: ", "Shader file does not exist");
                    return;
                }

                std::string source = read_file(path);
                auto shader_sources = pre_process(source);
                compile(shader_sources);
                m_name = sname;
            }

            gl_shader(const std::string& sname, const std::filesystem::path& vertex, const std::filesystem::path& fragment){
                if(!std::filesystem::exists(vertex) || !std::filesystem::exists(fragment)){
                    gapi_debug_msg("Shader parse error: ", "Shader file does not exist");
                    return;
                }

                std::string vertex_source = read_file(vertex);
                std::string fragment_source = read_file(fragment);
                std::unordered_map<gl_shader_type, std::string> shader_sources = {
                    {gl_shader_type::vertex, vertex_source},
                    {gl_shader_type::fragment, fragment_source}
                };
                compile(shader_sources);
                m_name = sname;
            }

            virtual ~gl_shader(){
                gl(glDeleteProgram(m_id));
            }

            void bind() const override{
                gl(glUseProgram(m_id));
            }

            void unbind() const override{
                gl(glUseProgram(0));
            }

            inline virtual const std::string& name() const override { return m_name; }

            virtual bool uniform(const std::string& n, uint32_t v) const override{
                uint32_t uniform_location = validator(n);
                if(uniform_location == -1) return false;
                gl(glUniform1i(uniform_location, v));
                return true;
            }

            virtual bool uniform(const std::string& n, float v) const override{
                uint32_t uniform_location = validator(n);
                if(uniform_location == -1) return false;
                gl(glUniform1f(uniform_location, v));
                return true;
            }

            virtual bool uniform(const std::string& n, float x, float y) const override{
                uint32_t uniform_location = validator(n);
                if(uniform_location == -1) return false;
                gl(glUniform2f(uniform_location, x, y));
                return true;
            }

            virtual bool uniform(const std::string& n, float x, float y, float z) const override{
                uint32_t uniform_location = validator(n);
                if(uniform_location == -1) return false;
                gl(glUniform3f(uniform_location, x, y, z));
                return true;
            }

            virtual bool uniform(const std::string& n, float x, float y, float z, float w) const override {
                uint32_t uniform_location = validator(n);
                if(uniform_location == -1) return false;
                gl(glUniform4f(uniform_location, x, y, z, w));
                return true;
            }

            virtual bool uniform(const std::string& n, const glm::vec2& v) const override{
                uint32_t uniform_location = validator(n);
                if(uniform_location == -1) return false;
                gl(glUniform2fv(uniform_location, 1, glm::value_ptr(v)));
                return true;
            }

            virtual bool uniform(const std::string& n, const glm::vec3& v) const override{
                uint32_t uniform_location = validator(n);
                if(uniform_location == -1) return false;
                gl(glUniform3fv(uniform_location, 1, glm::value_ptr(v)));
                return true;
            }

            virtual bool uniform(const std::string& n, const glm::vec4& v) const override{
                uint32_t uniform_location = validator(n);
                if(uniform_location == -1) return false;
                gl(glUniform4fv(uniform_location, 1, glm::value_ptr(v)));
                return true;
            }

            virtual bool uniform(const std::string& n, const glm::mat2& v) const override{
                uint32_t uniform_location = validator(n);
                if(uniform_location == -1) return false;
                gl(glUniformMatrix2fv(uniform_location, 1, GL_FALSE, glm::value_ptr(v)));
                return true;
            }

            virtual bool uniform(const std::string& n, const glm::mat3& v) const override{
                uint32_t uniform_location = validator(n);
                if(uniform_location == -1) return false;
                gl(glUniformMatrix3fv(uniform_location, 1, GL_FALSE, glm::value_ptr(v)));
                return true;
            }

            virtual bool uniform(const std::string& n, const glm::mat4& v) const override{
                uint32_t uniform_location = validator(n);
                if(uniform_location == -1) return false;
                gl(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(v)));
                return true;
            }

            inline uint32_t id() const { return m_id; }
            inline uint32_t uniformloc(const std::string& n) const { return glGetUniformLocation(m_id, n.c_str()); }


        private:
            uint32_t m_id{0};
            std::string m_name{};
    };

    class gl_texture_2d final : public texture {
        public:
            gl_texture_2d(std::filesystem::path path){
                m_path = path.string();
                stbi_set_flip_vertically_on_load(true);
                m_data = stbi_load(m_path.c_str(), &m_width, &m_height, &m_channels, 0);
                gapi_assert(m_data == nullptr, "Failed to load texture data");

                GLenum internal_format = 0, data_format = 0;
                if(m_channels == 4){
                    internal_format = GL_RGBA8;
                    data_format = GL_RGBA;
                }
                else if(m_channels == 3){
                    internal_format = GL_RGB8;
                    data_format = GL_RGB;
                }

                gapi_assert(internal_format == 0 || data_format == 0, "Texture format not supported");

                gl(glGenTextures(1, &m_id));
                gl(glBindTexture(GL_TEXTURE_2D, m_id));
                gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                gl(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_width, m_height, 0, data_format, GL_UNSIGNED_BYTE, m_data));
            }

            virtual ~gl_texture_2d(){
                gl(glDeleteTextures(1, &m_id));
                stbi_image_free(m_data);
            }

            virtual void bind(uint32_t slot = 0) const override{
                gl(glActiveTexture(GL_TEXTURE0 + slot));
                gl(glBindTexture(GL_TEXTURE_2D, m_id));
            }

            [[maybe_unused]] virtual void unbind() const override{
                gl(glBindTexture(GL_TEXTURE_2D, 0));
            }

            [[maybe_unused]] virtual void* data() const override { return m_data; }
            
            virtual uint32_t id() const override { return m_id; }
            virtual int32_t width() const override { return m_width; }
            virtual int32_t height() const override { return m_height; }
            virtual int32_t channels() const override { return m_channels; }
        
        private:
            int32_t m_width{0};
            int32_t m_height{0};
            int32_t m_channels{0};
            uint32_t m_id{0};
            std::string m_path{};
            uint8_t* m_data{nullptr};
    };

    class gl_api final : public base_api {
        public:
            gl_api() : base_api(_gapi::opengl){}
            virtual ~gl_api() = default;

            virtual void init() override{
                gl(glEnable(GL_BLEND));
                gl(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            }

            virtual void draw(const std::shared_ptr<vertex_array>& va) override{
                auto& index_buffer = va->index();
                gl(glDrawElements(GL_TRIANGLES, index_buffer->count(), GL_UNSIGNED_INT, nullptr));
            }

            virtual void clear() override{
                gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            }

            virtual void clear_color(float r, float g, float b, float a) override{
                gl(glClearColor(r, g, b, a));
            }
    };
}

namespace gapi{
    class renderer {
        public:
            static void init(){
                //[FIXME]:This should be platform specific
                m_api = std::make_shared<opengl::gl_api>();
                m_api->init();
            }

            static void clear(){
                m_api->clear();
            }

            static void clear_color(float r, float g, float b, float a){
                m_api->clear_color(r, g, b, a);
            }

            template<typename... Args>
            static inline void submit(const std::shared_ptr<vertex_array>& va,
                const std::function<void(Args...)>& func, Args... args){
                func(std::forward<Args>(args)...);
                va->bind();
                draw(va);
            }

            template<typename Ret, typename... TArgs>
            static inline void submit(const std::shared_ptr<vertex_array>& va, 
                const std::function<Ret(TArgs...)>& func, TArgs... args){
                func(std::forward<TArgs>(args)...);
                va->bind();
                draw(va);
            }

            template<typename... Args>
            static void begin(const std::function<void(Args...)>& func, Args... args){
                func(std::forward<Args>(args)...);
            }

            template<typename... Args>
            static void end(const std::function<void(Args...)>& func, Args... args){
                func(std::forward<Args>(args)...);
            }

            static inline _gapi api() { return m_api->api(); }

        private:
            renderer() = default;
            renderer(const renderer&) = delete;
            renderer& operator=(const renderer&) = delete;
            ~renderer() = default;

            static void draw(const std::shared_ptr<vertex_array>& va);

        private:
            static std::shared_ptr<base_api> m_api;

    };

    template<typename Ty>
    requires std::is_base_of<context, Ty>::value
    [[nodiscard]] std::shared_ptr<Ty> make_context() noexcept{
        return std::make_shared<Ty>();
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

    class shader_container{
        public:
            shader_container() = default;
            ~shader_container() = default;

            void emplace(const std::shared_ptr<shader>& shader){
                auto& shader_name = shader->name();
                gapi_assert(m_shaders.find(shader_name) == m_shaders.end(), "Shader already exists");
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
                gapi_assert(it != m_shaders.end(), "Shader not found");
                return it->second;
            }

        private:
            std::unordered_map<std::string, std::shared_ptr<shader>> m_shaders{};
    };
    
    std::shared_ptr<base_api> renderer::m_api = nullptr;
}