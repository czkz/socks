#include <iostream>
#include <thread>
#include "SockStream.h"

void clientThread() {
    SockClient client;
    client.Connect("127.0.0.1", 5555);
    client.Send("Hello");
    std::string s = client.Receive();
    std::cout << "Got from server: " << s << '\n';
}

void serverThread() {
    SockServer server;
    server.Start(5555);
    ConnectedClient client = server.Accept();
    std::string s = client.Receive();
    std::cout << "Got from client: " << s << '\n';
    client.Send("World");
}

int main() {
    auto t1 = std::thread(serverThread);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto t2 = std::thread(clientThread);

    t1.join();
    t2.join();
}
