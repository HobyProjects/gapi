#pragma once

#include "gapi_impl_opengl.hpp"

namespace gapi::renderer{

    template<typename GApi>
    class gapi_render {

        public:
            gapi_render() = default;
            gapi_render(const gapi_render&) = delete;
            gapi_render& operator=(const gapi_render&) = delete;
            ~gapi_render() = default;

            void init(){
                api = std::make_shared<GApi>();
                api->init();
            }

            void clear(){
                api->clear();
            }

           void clear_color(float r, float g, float b, float a){
                api->clear_color(r, g, b, a);
            }

            void submit(const std::shared_ptr<vertex_array>& va){
                va->bind();
                draw(va);
            }
    
        private:
            void draw(const std::shared_ptr<vertex_array>& va){
                api->draw(va);
            }

        private:
            std::shared_ptr<GApi> api;

    };

    using gl_renderer = gapi_render<ggl::api>;
}

namespace gapir = gapi::renderer;