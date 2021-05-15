#include <iostream>
#include <thread>
#include "SockStream.h"
#include "../cp/dbg.h"

void sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void clientThread() try {
    SockClient client;
    client.Connect("127.0.0.1", 5555);
    client.Send("Hello");
    client.Disconnect();
} catch (const std::exception& e) {
    std::cout << "Caught: " << e.what();
}

void serverThread() try {
    SockServer server;
    server.Start(5555);
    dp(server.HasClients());
    sleep(200);
    dp(server.HasClients());
    ConnectedClient sc = server.Accept();

    dp(sc.HasData());
    dp(sc.ReceiveAvailable());
    sleep(100);

    dp(sc.HasData());
    dp(sc.DisconnectGet());
    sleep(100);

} catch (const std::exception& e) {
    std::cout << "Caught: " << e.what();
}

int main() try {
    auto t1 = std::thread(serverThread);
    sleep(100);
    auto t2 = std::thread(clientThread);

    t1.join();
    t2.join();

} catch (const std::exception& e) {
    std::cout << "Caught: " << e.what();
}
