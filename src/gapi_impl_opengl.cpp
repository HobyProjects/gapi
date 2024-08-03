#include "gapi_impl_opengl.hpp"

namespace gapi::opengl{

    gl_info::gl_info(){
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


    bool gl_context::init() noexcept{
        gapi_assert(m_window != nullptr, "Window is nullptr");
        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1);

        glewExperimental = GL_TRUE;
        GLenum status = glewInit();
        gapi_assert(status != GLEW_OK, "Failed to initialize GLEW");

        m_info = std::make_shared<gl_info>();
        return true;
    }

    void gl_context::swap() noexcept{
        gapi_assert(m_window != nullptr, "Window is nullptr");
        glfwSwapBuffers(m_window);
    }

    void gl_context::interval(uint32_t interval) noexcept{
        gapi_assert(m_window != nullptr, "Window is nullptr");
        glfwSwapInterval(interval);
    }

    gl_vertex_buffer::gl_vertex_buffer(float* v, size_t s, gl_draw_type t){
        gl(glGenBuffers(1, &m_id));
        gl(glBindBuffer(GL_ARRAY_BUFFER, m_id));
        gl(glBufferData(GL_ARRAY_BUFFER, s, v, static_cast<GLenum>(t)));
    }

    gl_vertex_buffer::~gl_vertex_buffer(){
        gl(glDeleteBuffers(1, &m_id));
    }

    void gl_vertex_buffer::bind() const noexcept{
        gl(glBindBuffer(GL_ARRAY_BUFFER, m_id));
    }

    void gl_vertex_buffer::unbind() const noexcept{
        gl(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

    gl_index_buffer::gl_index_buffer(uint32_t * i, size_t c, gl_draw_type t){
        gl(glGenBuffers(1, &m_id));
        gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
        gl(glBufferData(GL_ELEMENT_ARRAY_BUFFER, c, i, static_cast<GLenum>(t)));
    }

    gl_index_buffer::~gl_index_buffer(){
        gl(glDeleteBuffers(1, &m_id));
    }

    void gl_index_buffer::bind() const noexcept{
        gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
    }

    void gl_index_buffer::unbind() const noexcept{
        gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    gl_vertex_array::gl_vertex_array(){
        gl(glGenVertexArrays(1, &m_id));
    }

    gl_vertex_array::~gl_vertex_array(){
        gl(glDeleteVertexArrays(1, &m_id));
    }

    void gl_vertex_array::bind() const noexcept{
        gl(glBindVertexArray(m_id));
    }

    void gl_vertex_array::unbind() const noexcept{
        gl(glBindVertexArray(0));
    }

    void gl_vertex_array::emplace_vbuffer(const std::shared_ptr<vertex_buffer>& vb) noexcept{
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

    void gl_vertex_array::emplace_ibuffer(const std::shared_ptr<index_buffer>& ib) noexcept{
        ib->bind();
        m_index_buffer = ib;
    }

    gl_shader::gl_shader(const std::string & sname, const std::filesystem::path & path){
        if(!std::filesystem::exists(path)){
            gapi_debug_msg("Shader parse error: ", "Shader file does not exist");
            return;
        }

        std::string source = read_file(path);
        auto shader_sources = pre_process(source);
        compile(shader_sources);
        m_name = sname;
    }

    gl_shader::gl_shader(const std::string & sname, const std::filesystem::path & vertex, const std::filesystem::path & fragment){
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

    gl_shader::~gl_shader(){
        gl(glDeleteProgram(m_id));
    }

    void gl_shader::bind() const noexcept{
       gl(glUseProgram(m_id));
    }

    void gl_shader::unbind() const noexcept{
        gl(glUseProgram(0));
    }

    bool gl_shader::uniform(const std::string & n, uint32_t v) const noexcept{
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform1i(uniform_location, v));
        return true;
    }

    bool gl_shader::uniform(const std::string & n, float v) const noexcept{
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform1f(uniform_location, v));
        return true;
    }

    bool gl_shader::uniform(const std::string & n, float x, float y) const noexcept{
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform2f(uniform_location, x, y));
        return true;
    }

    bool gl_shader::uniform(const std::string & n, float x, float y, float z) const noexcept{
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform3f(uniform_location, x, y, z));
        return true;
    }

    bool gl_shader::uniform(const std::string& n, float x, float y, float z, float w) const noexcept{
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform4f(uniform_location, x, y, z, w));
        return true;
    }

    bool gl_shader::uniform(const std::string & n, const glm::vec2 & v) const noexcept{
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform2fv(uniform_location, 1, glm::value_ptr(v)));
        return true;
    }

    bool gl_shader::uniform(const std::string & n, const glm::vec3 & v) const noexcept{
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform3fv(uniform_location, 1, glm::value_ptr(v)));
        return true;
    }

    bool gl_shader::uniform(const std::string & n, const glm::vec4 & v) const noexcept{
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform4fv(uniform_location, 1, glm::value_ptr(v)));
        return true;
    }

    bool gl_shader::uniform(const std::string & n, const glm::mat2 & v) const noexcept{
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniformMatrix2fv(uniform_location, 1, GL_FALSE, glm::value_ptr(v)));
        return true;
    }

    bool gl_shader::uniform(const std::string & n, const glm::mat3 & v) const noexcept{
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniformMatrix3fv(uniform_location, 1, GL_FALSE, glm::value_ptr(v)));
        return true;
    }

    bool gl_shader::uniform(const std::string & n, const glm::mat4 & v) const noexcept{
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(v)));
        return true;
    }

    uint32_t gl_shader::validator(const std::string & n) const noexcept{
        uint32_t uniform_location = gl(glGetUniformLocation(m_id, n.c_str()));
        if(uniform_location == -1) {
            gapi_debug_msg("Uniform not found: ", n);
            return -1;
        }

        return uniform_location;
    }

    void gl_shader::compile(std::unordered_map<gl_shader_type, std::string> sources){
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

    std::string gl_shader::read_file(const std::filesystem::path & file_path) const{
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

    static gl_shader_type shader_type_from_string(const std::string &type)
    {
        if(type == "vertex")                          return gl_shader_type::vertex;
        if(type == "fragment" || type == "pixel")     return gl_shader_type::fragment;
        if(type == "geometry")                        return gl_shader_type::geometry;

        gapi_assert(true, "Invalid shader type specified");
        return gl_shader_type::none;
    }

    std::unordered_map<gl_shader_type,std::string> gl_shader::pre_process(const std::string & src) const{
        std::unordered_map<gl_shader_type,std::string> shader_sources;
        const char *type_token = "#type";
        size_t type_token_length = strlen(type_token);
        size_t pos = src.find(type_token, 0);

        while(pos != std::string::npos)
        {
            size_t eol = src.find_first_of("\r\n", pos);
            gapi_assert(eol == std::string::npos, "Syntax error, Did you forget to add shader typeline #type declaration");

            size_t begin = pos + type_token_length + 1;
            std::string type = src.substr(begin, eol - begin);
            gapi_assert(type != "vertex" && type != "fragment" && type != "pixel" && type != "geometry", "Invalid shader type specified");

            size_t next_line_pos = src.find_first_not_of("\r\n", eol);
            pos = src.find(type_token, next_line_pos);
            shader_sources[shader_type_from_string(type)] = src.substr(next_line_pos, pos - (next_line_pos == std::string::npos ? src.size() - 1 : next_line_pos));
        }

        return shader_sources;
    }

}