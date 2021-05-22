#include <iostream>
#include "SockStream.h"

int main() {
    SockClient client;
    if (!client.Connect("example.com", 80)) {
        std::cout << "Connection failed.";
        return 1;
    }

    std::string request = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
    client.Send(request);
    std::cout << client.ReceiveAvailable() << '\n';

    return 0;
}

// Note: can't write `const char* request = "...";`
// because it ends with a null byte, which must not be sent
//
// Note: ReceiveAvailable() may not return the whole page
// on a slow connection, consider a while loop
