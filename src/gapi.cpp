#include "gapi_impl_opengl.hpp"
#include "gapi.hpp"

namespace gapi {

    void shader_container::emplace(const std::shared_ptr<shader>& shader) noexcept{
        auto& shader_name = shader->name();
        gapi_assert(m_shaders.find(shader_name) == m_shaders.end(), "Shader already exists");
        m_shaders[shader_name] = shader;
    }

    std::shared_ptr<shader> shader_container::get(const std::string & name) const noexcept{
        auto it = m_shaders.find(name);
        gapi_assert(it != m_shaders.end(), "Shader not found");
        return it->second;
    }

    std::shared_ptr<shader> shader_container::operator[](const std::string & name) const noexcept{
        return get(name);
    }

    std::shared_ptr<base_api> renderer::m_api = nullptr;

    void renderer::init(){
        //[FIXME]:This should be platform specific
        m_api = std::make_shared<opengl::gl_api>();
        m_api->init();
    }

    void renderer::clear(){
        m_api->clear();
    }

    void renderer::clear_color(float r, float g, float b, float a){
        m_api->clear_color(r, g, b, a);
    }

    void renderer::draw(const std::shared_ptr<vertex_array>& va){
        m_api->draw(va);
    }
}