
#include <iostream>
#include <thread>
#include <mutex>
#include <set>
#include "SockStream.h"

using std::cout;

void sleep(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

std::set<ClientConnection> clients;
std::mutex clients_mutex;

void onReceived(std::string data) {
    std::lock_guard l { clients_mutex };
    for (const auto& e : clients) {
        try {
            e.Send(data);
        } catch (const SockError& err) {
            clients.erase(e);
            std::cout << err.what() << std::endl;
        }
    }
}

void clientThreadFunc(const ClientConnection& sock) try {
    while (true) {
        onReceived(sock.ReceiveAvailable());
    }
} catch (const SockError& e) { }

int main() {
    SockServer server;
    server.Start(5555);

    while (true) {
        const auto it = clients.emplace(server.Accept()).first;
        std::thread(clientThreadFunc, std::ref(*it)).detach();
    }
}
