
#include <iostream>
#include <thread>
#include <mutex>
#include <set>
#include "SockStream.h"

using std::cout;

void sleep(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

std::set<ClientConnection> clients;
std::mutex clients_mutex;

using set_it = decltype(clients)::iterator;

void onReceived(std::string data, set_it who) {
    std::lock_guard l { clients_mutex };
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it == who) { continue; }
        try {
            it->Send(data);
        } catch (const SockError& err) {
            std::cout << err.what() << std::endl;
        }
    }
}

void clientThreadFunc(const set_it it) try {
    const ClientConnection& sock = *it;
    while (true) {
        onReceived(sock.ReceiveAvailable(), it);
    }
} catch (const SockError& e) {
    std::lock_guard l { clients_mutex };
    clients.erase(it);
}

int main() {
    SockServer server;
    server.Start(5555);

    while (true) {
        auto c = server.Accept();
        std::lock_guard l { clients_mutex };
        const auto it = clients.insert(std::move(c)).first;
        std::thread(clientThreadFunc, std::move(it)).detach();
    }
}
