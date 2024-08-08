#include "gapi_impl_opengl.hpp"

#ifdef _DEBUG
#include <iostream>
struct gl_error_message{

    gl_error_message() {}
    gl_error_message(uint32_t error, const char* str, const char* file, int line): 
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

    uint32_t error{0};
    const char* enum_str{nullptr};
    const char* file{nullptr};
    int line{0};
};

inline gl_error_message get_error_message(uint32_t error){
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
    
    info::info(){
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

    bool context::init() {
        gapi_asserts(m_window != nullptr, "Window is nullptr");
        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1);

        glewExperimental = GL_TRUE;
        if(GLenum status = glewInit() != GLEW_OK){
            gapi_asserts(status, "Failed to initialize GLEW");
            return false;
        }

        m_info = std::make_shared<gapi::opengl::info>();
        return true;
    }

    void context::swap(){
        gapi_asserts(m_window != nullptr, "Window is nullptr");
        glfwSwapBuffers(m_window);
    }

    void context::interval(uint32_t interval){
        gapi_asserts(m_window != nullptr, "Window is nullptr");
        glfwSwapInterval(interval);
    }

    vertex_buffer::vertex_buffer(float * v, uint32_t s, DRAW t){
        gl(glGenBuffers(1, &m_id));
        gl(glBindBuffer(GL_ARRAY_BUFFER, m_id));
        gl(glBufferData(GL_ARRAY_BUFFER, s, v, static_cast<GLenum>(t)));
    }

    vertex_buffer::~vertex_buffer(){
        gl(glDeleteBuffers(1, &m_id));
    }

    void vertex_buffer::bind() const{
        gl(glBindBuffer(GL_ARRAY_BUFFER, m_id));
    }

    void vertex_buffer::unbind() const{
        gl(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

    index_buffer::index_buffer(uint32_t* i, size_t c, DRAW t){
        gl(glGenBuffers(1, &m_id));
        gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
        gl(glBufferData(GL_ELEMENT_ARRAY_BUFFER, c, i, static_cast<GLenum>(t)));
    }

    index_buffer::~index_buffer(){
        gl(glDeleteBuffers(1, &m_id));
    }

    void index_buffer::bind() const {
        gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
    }

    void index_buffer::unbind() const {
        gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    vertex_array::vertex_array(){
        gl(glGenVertexArrays(1, &m_id));
    }

    vertex_array::~vertex_array(){
        gl(glDeleteVertexArrays(1, &m_id));
    }

    void vertex_array::bind() const {
        gl(glBindVertexArray(m_id));
    }

    void vertex_array::unbind() const {
        gl(glBindVertexArray(0));
    }

    void vertex_array::emplace_vertex(const std::shared_ptr<gapi::vertex_buffer>& vb){
        vb->bind();
        const auto& layout = vb->layout();
        const auto& elements = layout.elements();
        size_t index = 0;
        for(const auto& element : elements)
        {
            gl(glEnableVertexAttribArray(index));
            gl(glVertexAttribPointer(index, element.component, FLOAT, 
                element.normalized, layout.stride(), (const void*)(element.offset)));
            index++;
        }
        m_vertex_buffers.emplace_back(vb);
    }

    void vertex_array::emplace_index(const std::shared_ptr<gapi::index_buffer>& ib){
        ib->bind();
        m_index_buffer = ib;
    }

    uint32_t shader::validator(const std::string& n) const {
        uint32_t uniform_location = gl(glGetUniformLocation(m_id, n.c_str()));
        if(uniform_location == -1) {
            gapi_debug_msg("Uniform not found: ", n);
            return -1;
        }

        return uniform_location;
    }

    void shader::compile(std::unordered_map<SHADER_TYPE, std::string> sources){
        uint32_t shader_program = gl(glCreateProgram());
        if(shader_program == GL_FALSE) return;

        std::vector<uint32_t> shaders;
        shaders.reserve(sources.size());

        for(auto& source : sources){
            SHADER_TYPE type = source.first;
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

    std::string shader::read_file(const std::filesystem::path& file_path) const{
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

    static SHADER_TYPE shader_type_from_string(const std::string &type){
        if(type == "vertex")                          return SHADER_TYPE::SHADER_VERTEX;
        if(type == "fragment" || type == "pixel")     return SHADER_TYPE::SHADER_FRAGMENT;
        if(type == "geometry")                        return SHADER_TYPE::SHADER_GEOMETRY;
        gapi_asserts(true, "Invalid shader type specified");
        return SHADER_TYPE::SHADER_NONE;
    }

    std::unordered_map<SHADER_TYPE, std::string> shader::pre_process(const std::string& src) const {
        std::unordered_map<SHADER_TYPE,std::string> shader_sources;
        const char *type_token = "#type";
        size_t type_token_length = strlen(type_token);
        size_t pos = src.find(type_token, 0);

        while(pos != std::string::npos){
            size_t eol = src.find_first_of("\r\n", pos);
            gapi_asserts(eol == std::string::npos, "Syntax error, Did you forget to add shader type line #type declaration");
            size_t begin = pos + type_token_length + 1;
            std::string type = src.substr(begin, eol - begin);
            gapi_asserts(type != "vertex" && type != "fragment" && type != "pixel" && type != "geometry", "Invalid shader type specified");
            size_t next_line_pos = src.find_first_not_of("\r\n", eol);
            pos = src.find(type_token, next_line_pos);
            shader_sources[shader_type_from_string(type)] = src.substr(next_line_pos, pos - (next_line_pos == std::string::npos ? src.size() - 1 : next_line_pos));
        }

        return shader_sources;
    }

    shader::shader(const std::string& sname, const std::filesystem::path& path){
        if(!std::filesystem::exists(path)){
            gapi_debug_msg("Shader parse error: ", "Shader file does not exist");
            return;
        }

        std::string source = read_file(path);
        auto shader_sources = pre_process(source);
        compile(shader_sources);
        m_name = sname;
    }

    shader::shader(const std::string& sname, const std::filesystem::path& vertex, const std::filesystem::path& fragment){
        if(!std::filesystem::exists(vertex) || !std::filesystem::exists(fragment)){
            gapi_debug_msg("Shader parse error: ", "Shader file does not exist");
            return;
        }

        std::string vertex_source = read_file(vertex);
        std::string fragment_source = read_file(fragment);
        std::unordered_map<SHADER_TYPE, std::string> shader_sources = {
            {SHADER_VERTEX, vertex_source},
            {SHADER_FRAGMENT, fragment_source}
        };
        compile(shader_sources);
        m_name = sname;
    }

    shader::~shader(){
        gl(glDeleteProgram(m_id));
    }

    void shader::bind() const {
        gl(glUseProgram(m_id));
    }

    void shader::unbind() const {
        gl(glUseProgram(0));
    }

    bool shader::uniform(const std::string& n, uint32_t v) const {
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform1i(uniform_location, v));
        return true;
    }

    bool shader::uniform(const std::string& n, float v) const {
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform1f(uniform_location, v));
        return true;
    }

    bool shader::uniform(const std::string& n, float x, float y) const {
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform2f(uniform_location, x, y));
        return true;
    }

    bool shader::uniform(const std::string& n, float x, float y, float z) const {
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform3f(uniform_location, x, y, z));
        return true;
    }

    bool shader::uniform(const std::string& n, float x, float y, float z, float w) const {
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform4f(uniform_location, x, y, z, w));
        return true;
    }

    bool shader::uniform(const std::string& n, const glm::vec2& v) const {
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform2fv(uniform_location, 1, glm::value_ptr(v)));
        return true;
    }

    bool shader::uniform(const std::string& n, const glm::vec3& v) const {
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform3fv(uniform_location, 1, glm::value_ptr(v)));
        return true;
    }

    bool shader::uniform(const std::string& n, const glm::vec4& v) const {
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniform4fv(uniform_location, 1, glm::value_ptr(v)));
        return true;
    }

    bool shader::uniform(const std::string& n, const glm::mat2& v) const {
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniformMatrix2fv(uniform_location, 1, GL_FALSE, glm::value_ptr(v)));
        return true;
    }

    bool shader::uniform(const std::string& n, const glm::mat3& v) const {
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniformMatrix3fv(uniform_location, 1, GL_FALSE, glm::value_ptr(v)));
        return true;
    }

    bool shader::uniform(const std::string& n, const glm::mat4& v) const {
        uint32_t uniform_location = validator(n);
        if(uniform_location == -1) return false;
        gl(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(v)));
        return true;
    }

    texture_2d::texture_2d(std::filesystem::path path, TEXTURE_FILTER filter, TEXTURE_WRAP wrap,  bool flip){
        m_path = path.string();
        stbi_set_flip_vertically_on_load(flip);
        m_data = stbi_load(m_path.c_str(), &m_width, &m_height, &m_channels, 0);
        gapi_asserts(m_data == nullptr, "Failed to load texture data");

        GLenum internal_format = 0, data_format = 0;
        if(m_channels == 4){
            internal_format = GL_RGBA8;
            data_format = GL_RGBA;
        }
        else if(m_channels == 3){
            internal_format = GL_RGB8;
            data_format = GL_RGB;
        }

        gapi_asserts(internal_format == 0 || data_format == 0, "Texture format not supported");

        gl(glGenTextures(1, &m_id));
        gl(glBindTexture(TEXTURE_2D, m_id));
        gl(glTexParameteri(TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
        gl(glTexParameteri(TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
        gl(glTexParameteri(TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
        gl(glTexParameteri(TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));
        gl(glTexImage2D(TEXTURE_2D, 0, internal_format, m_width, m_height, 0, data_format, GL_UNSIGNED_BYTE, m_data));
    }

    texture_2d::~texture_2d(){
        gl(glDeleteTextures(1, &m_id));
        stbi_image_free(m_data);
    }

    void texture_2d::bind(uint32_t slot) const {
        gl(glActiveTexture(GL_TEXTURE0 + slot));
        gl(glBindTexture(GL_TEXTURE_2D, m_id));
    }

    void texture_2d::unbind() const {
        gl(glBindTexture(GL_TEXTURE_2D, 0));
    }

    void api::init() {
        gl(glEnable(GL_BLEND));
        gl(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    }
    void api::draw(const std::shared_ptr<gapi::vertex_array>& va) {
        auto& index_buffer = va->index();
        gl(glDrawElements(GL_TRIANGLES, index_buffer->count(), GL_UNSIGNED_INT, nullptr));
    }

    void api::clear() {
        gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    void api::clear_color(float r, float g, float b, float a) {
        gl(glClearColor(r, g, b, a));
    }

    std::shared_ptr<context> make_context(GLFWwindow* window) noexcept{
        return std::make_shared<context>(window);
    }

    std::shared_ptr<vertex_buffer> make_vertex(float* v, uint32_t s, DRAW t) noexcept{
        return std::make_shared<gapi::opengl::vertex_buffer>(v, s, t);
    }

    std::shared_ptr<index_buffer> make_index(uint32_t* i, size_t c, DRAW t) noexcept{
        return std::make_shared<index_buffer>(i, c, t);
    }

    std::shared_ptr<vertex_array> make_array() noexcept{
        return std::make_shared<vertex_array>();
    }

    std::shared_ptr<texture_2d> make_texture2d(std::filesystem::path path, TEXTURE_FILTER filter, TEXTURE_WRAP wrap,  bool flip) noexcept{
        return std::make_shared<texture_2d>(path, filter, wrap, flip);
    }

    std::shared_ptr<shader> make_shader(const std::string& sname, const std::filesystem::path& path) noexcept{
        return std::make_shared<shader>(sname, path);
    }

    std::shared_ptr<shader> make_shader(const std::string& sname, const std::filesystem::path& vertex, const std::filesystem::path& fragment) noexcept{
        return std::make_shared<shader>(sname, vertex, fragment);
    }

}