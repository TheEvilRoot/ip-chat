#include "imgui_wrapper.h"

#include "utils.h"

#include "error_wrapper.h"
#include "socket_wrapper.h"
#include "ifs_wrapper.h"
#include "protocol_wrapper.h"

#include <pthread.h>
#include <optional>
#include <set>

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct Message
{
    std::string sender;
    std::string time;
    std::string message;

    bool is_outgoing;
};

struct Client
{
    std::string address;
    std::string network;

    bool operator!=(const Client& cl) const {
        return cl.address != address;
    }

    bool operator==(const Client& cl) const {
        return cl.address == address;
    }

};

struct GlobalContext
{
    Socket inet{SOCK_RAW, IPPROTO_RAW - 40};
    std::vector<Interface> interfaces{Interface::interfaces()};
    std::vector<std::string> multicastGroups{};

    char broadcastChat[1024]{};
    std::vector<Message> broadcastMessages{};
    std::vector<Message> multicastMessages{};
    std::set<Client, std::equal_to<>> clients{};
    std::set<std::string> blacklist{};

    std::string selectedAddress{interfaces[0].broadcast};

    bool isTerminated{false};

    char multicastControlField[17]{};
    Error multicastControlError{};
};

struct Renderer
{

    GlobalContext& context;

    static auto renderError(const Error& e)
    {
        ImGui::Text("Error %s: %d - %s", e.place.c_str(), e.code, e.description.c_str());
    }

    static std::optional<Error> handleMulticastControl(int handle, int type, const std::string& address)
    {
        ip_mreq req{};
        if (!inet_pton(AF_INET, address.c_str(), static_cast<void*>(&req.imr_multiaddr)))
        {
            return Error{"inet_pton"};
        }
        if (setsockopt(handle, IPPROTO_IP, type, static_cast<const void*>(&req), sizeof(req)) != 0)
        {
            return Error{"setsockopt"};
        }
        return std::optional<Error>{};
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
                auto isSelected = context.selectedAddress == in.broadcast;
                ImGui::BeginDisabled(isSelected);
                if (ImGui::SmallButton(isSelected ? "Selected" : (std::string{"Select "} + in.name).c_str()))
                {
                    printf("select interface %s\n", in.name.c_str());
                    if (context.selectedAddress != in.broadcast)
                    {
                        context.selectedAddress = in.broadcast;
                        Protocol::discover(context.inet, in.broadcast);
                    }
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

        if (ImGui::BeginListBox("Multicast groups"))
        {
            for (int i = 0; const auto& in : context.multicastGroups)
            {
                ImGui::BeginGroup();
                ImGui::Text("%s", in.c_str());
                ImGui::SameLine();
                auto isSelected = context.selectedAddress == in;
                ImGui::BeginDisabled(isSelected);
                if (ImGui::SmallButton(isSelected ? "Selected" : (std::string{"Select "} + in).c_str()))
                {
                    if (context.selectedAddress != in)
                    {
                        context.selectedAddress = in;
                        Protocol::discover(context.inet, in);
                    }
                }
                ImGui::EndDisabled();
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
        ImGui::Text("Selected address %s", context.selectedAddress.c_str());
        ImGui::End();

        ImGui::Begin("Broadcast");
        if (ImGui::BeginListBox("Chat"))
        {
            for (const auto &msg : context.broadcastMessages)
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
                Protocol::send(context.inet, text, context.selectedAddress);
                context.broadcastChat[0] = 0;
            }
        }
        ImGui::End();

        ImGui::Begin("Multicast Control");
        if (context.multicastControlError)
        {
            ImGui::TextColored(ImVec4{1.0, 0.0, 0.0, 1.0}, "Error %s: %d - %s",
                               context.multicastControlError.place.c_str(), context.multicastControlError.code,
                               context.multicastControlError.description.c_str());
        }
        ImGui::InputTextWithHint("Address", "IPv4...", context.multicastControlField, 17);
        if (ImGui::Button("Add"))
        {
            std::string address{context.multicastControlField};
            if (const auto error = handleMulticastControl(context.inet.handle, IP_ADD_MEMBERSHIP, address); error)
            {
                context.multicastControlError = error.value();
            }
            else
            {
                context.multicastGroups.push_back(address);
                context.multicastControlError = Error{};
            }
        }
        if (ImGui::Button("Drop"))
        {
            std::string address{context.multicastControlField};
            if (const auto error = handleMulticastControl(context.inet.handle, IP_DROP_MEMBERSHIP, address); error)
            {
                context.multicastControlError = error.value();
            }
            else
            {
                auto it = std::find(context.multicastGroups.begin(), context.multicastGroups.end(), address);
                if (it != context.multicastGroups.end())
                {
                    if (context.selectedAddress == *it)
                    {
                        context.selectedAddress = context.interfaces[0].broadcast;
                    }
                    context.multicastGroups.erase(it);
                }
                context.multicastControlError = Error{};
            }
        }
        ImGui::End();

        ImGui::Begin("Clients");
        if (ImGui::BeginListBox("Clients"))
        {
            for (const auto& in : context.clients)
            {
                ImGui::BeginGroup();
                ImGui::Text("%s", in.address.c_str());
                ImGui::SameLine();
                const auto it = context.blacklist.find(in.address);
                if (it == context.blacklist.end())
                {
                    if (ImGui::SmallButton((std::string{"Ignore "} + in.address).c_str()))
                    {
                        context.blacklist.insert(in.address);
                    }
                }
                else
                {
                    if (ImGui::SmallButton((std::string{"Un-ignore "} + in.address).c_str()))
                    {
                        context.blacklist.erase(it);
                    }
                }
                ImGui::Text("via %s", in.network.c_str());
                ImGui::Separator();
                ImGui::EndGroup();
            }
            ImGui::EndListBox();
        }
        if (ImGui::SmallButton("Discover new clients"))
        {
            Protocol::discover(context.inet, context.selectedAddress);
        }
        if (ImGui::BeginListBox("Blacklist"))
        {
            for (const auto& in : context.blacklist)
            {
                ImGui::BeginGroup();
                ImGui::Text("%s", in.c_str());
                ImGui::Separator();
                ImGui::EndGroup();
            }
            ImGui::EndListBox();
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

        const auto source = std::visit(overloaded {
            [](const HelloMessage& hello) {
                return hello.source;
            },
            [](const TextMessage& text) {
                return text.source;
            },
            [](const DiscoverMessage& discover) {
                return discover.source;
            }
        }, message);

        const auto isOutgoing = [&context](const auto x) -> bool {
            const auto it = std::find_if(std::begin(context->interfaces), std::end(context->interfaces), [&x](const auto y) {
                return y.ipv4 == x;
            });
            return it != std::end(context->interfaces);
        }(utils::addr_to_string(source));

        std::visit(overloaded {
                [isOutgoing, &context](const HelloMessage& hello) {
                    if (!isOutgoing)
                    {
                        context->clients.insert(Client{utils::addr_to_string(hello.source), hello.payload});
                    }
                },
                [isOutgoing, &context](const TextMessage& text) {
                    const auto sender = utils::addr_to_string(text.source);
                    const auto it = context->blacklist.find(sender);
                    if (it == context->blacklist.end())
                    {
                        context->broadcastMessages.push_back(Message{sender, text.time, text.payload, isOutgoing});
                    }
                    else
                    {
                        printf("ignore message from %s: %s\n", sender.c_str(), text.payload.c_str());
                    }
                },
                [isOutgoing, &context](const DiscoverMessage& discover) {
                    if (!isOutgoing)
                    {
                        printf("hello to %s\n", utils::addr_to_string(discover.source).c_str());
                        Protocol::hello(context->inet, discover.source, discover.payload);
                    }
                }
        }, message);
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