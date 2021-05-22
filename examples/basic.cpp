#include <iostream>
#include <thread>
#include "SockStream.h"

using std::cout;

void sleep(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }


void clientThread() {
    SockClient client;
    client.Connect("localhost", 5555);
    client.Send("Hello");
    cout << "Client received: " << client.ReceiveAvailable() << '\n';
}

void serverThread() {
    SockServer server;
    server.Start(5555);

    ClientConnection cl = server.Accept();
    cout << "Server received: " << cl.ReceiveAvailable() << '\n';
    cl.Send("World");
}

int main() {
    auto t1 = std::thread(serverThread);
    sleep(100);
    auto t2 = std::thread(clientThread);
    t1.join();
    t2.join();
}
