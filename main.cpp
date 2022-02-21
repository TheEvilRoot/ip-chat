#include "imgui_wrapper.h"

#include <string>

struct GlobalContext
{
    std::string hello;
};

struct Renderer
{

    GlobalContext& context;

    auto render() const
    {
        ImGui::Begin("Socket");
        ImGui::Text("Hello, %s", context.hello.c_str());
        ImGui::End();
    }
};

int main()
{
    GlobalContext context{"World"};
    Renderer renderer{context};
    backend::run(renderer);
}