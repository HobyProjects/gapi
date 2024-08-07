#pragma once

#include "gapi_impl_opengl.hpp"

namespace gapi::renderer{

    class renderer {
        public:
            static void init(){
                //[FIXME]:This should be platform specific
                api = std::make_shared<gapi::opengl::gl_api>();
                api->init();
            }

            static void clear(){
                api->clear();
            }

            static void clear_color(float r, float g, float b, float a){
                api->clear_color(r, g, b, a);
            }

            static inline void submit(const std::shared_ptr<vertex_array>& va){
                va->bind();
                draw(va);
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

        private:
            renderer() = default;
            renderer(const renderer&) = delete;
            renderer& operator=(const renderer&) = delete;
            ~renderer() = default;

            static void draw(const std::shared_ptr<vertex_array>& va){
                api->draw(va);
            }

        public:
            static std::shared_ptr<base_api> api;

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

std::shared_ptr<gapi::base_api> gapi::renderer::renderer::api = nullptr;