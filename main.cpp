#include "imgui_wrapper.h"

#include "utils.h"

#include "error_wrapper.h"
#include "socket_wrapper.h"
#include "ifs_wrapper.h"
#include "protocol_wrapper.h"

#include <pthread.h>

struct Message
{
    std::string sender;
    std::string time;
    std::string message;

    bool is_outgoing;
};

struct GlobalContext
{
    Socket inet{SOCK_RAW, IPPROTO_RAW - 40};
    std::vector<Interface> interfaces{Interface::interfaces()};

    char broadcastChat[1024]{};
    std::vector<Message> messages{};

    int selectedInterface{0};
    std::string broadcastAddress{interfaces[selectedInterface].broadcast};

    bool isTerminated{false};
};

struct Renderer
{

    GlobalContext& context;

    static auto renderError(const Error& e)
    {
        ImGui::Text("Error %s: %d - %s", e.place.c_str(), e.code, e.description.c_str());
    }

    auto render() const
    {
        ImGui::Begin("Interfaces");
        if (ImGui::BeginListBox("Interfaces"))
        {
            for (int i = 0; const auto& in : context.interfaces)
            {
                ImGui::BeginGroup();
                ImGui::Text("%s", in.name.c_str());
                ImGui::SameLine();
                auto isSelected = context.selectedInterface == i;
                ImGui::BeginDisabled(isSelected);
                if (ImGui::SmallButton(isSelected ? "Selected" : "Select"))
                {
                    context.selectedInterface = i;
                    context.broadcastAddress = in.broadcast;
                }
                ImGui::EndDisabled();
                ImGui::Text("IPv4 %s", in.ipv4.c_str());
                ImGui::Text("Mask %s", in.mask.c_str());
                ImGui::Text("Broadcast %s", in.broadcast.c_str());
                ImGui::Separator();
                ImGui::EndGroup();
                i++;
            }
            ImGui::EndListBox();
        }
        ImGui::End();

        ImGui::Begin("Socket");
        ImGui::Text("File descriptor %d", context.inet.handle);
        if (context.inet.error)
        {
            renderError(context.inet.error);
        }
        ImGui::Text("Sent %lu bytes", context.inet.sent);
        ImGui::Text("Received %lu bytes", context.inet.recved);
        ImGui::Separator();
        ImGui::Text("Selected broadcast %s", context.broadcastAddress.c_str());
        ImGui::End();

        ImGui::Begin("Broadcast");
        if (ImGui::BeginListBox("Chat"))
        {
            for (const auto &msg : context.messages)
            {
                ImGui::BeginGroup();
                ImGui::TextColored(ImVec4{0.5, 0.5, 0.0, 1.0}, "%s", msg.time.c_str());
                ImGui::SameLine();
                if (msg.is_outgoing)
                {
                    ImGui::TextColored(ImVec4{1.0, 0.0, 0.0, 1.0}, "%s", msg.sender.c_str());
                }
                else
                {
                    ImGui::TextColored(ImVec4{0.0, 0.5, 0.0, 1.0}, "%s", msg.sender.c_str());
                }

                ImGui::SameLine();
                ImGui::TextWrapped("%s", msg.message.c_str());
                ImGui::EndGroup();
            }
            ImGui::EndListBox();
        }
        ImGui::InputTextWithHint("Input", "Message...", context.broadcastChat, 1024);
        ImGui::SameLine();
        if (ImGui::Button("Send"))
        {
            const auto text = std::string{context.broadcastChat};
            if (!text.empty())
            {
                Protocol::send(context.inet, text, context.broadcastAddress);
                context.broadcastChat[0] = 0;
            }
        }
        ImGui::End();
    }
};

void threadHandler(const std::shared_ptr<GlobalContext>& context)
{
    char buffer[1024]{};
    while (!context->isTerminated)
    {
        const auto message = Protocol::recv(context->inet, buffer, sizeof(buffer));
        const auto isOutgoing = [&context](const auto x) -> bool {
            const auto it = std::find_if(std::begin(context->interfaces), std::end(context->interfaces), [&x](const auto y) {
                return y.ipv4 == x;
            });
            return it != std::end(context->interfaces);
        }(utils::addr_to_string(message.source));
        context->messages.push_back(Message{utils::addr_to_string(message.source), message.time, message.payload, isOutgoing});
    }
}

int main()
{
    auto context = std::make_shared<GlobalContext>();
    Renderer renderer{*context};
    pthread_t thread{};
    pthread_create(&thread, nullptr, [](void* p) -> void* {
        auto *context = static_cast<std::shared_ptr<GlobalContext>*>(p);
        threadHandler(*context);
        return nullptr;
    }, &context);
    backend::run(renderer);
    context->inet.close();
    context->isTerminated = true;
    pthread_join(thread, nullptr);
}