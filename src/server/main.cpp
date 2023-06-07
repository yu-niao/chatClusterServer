#include "chatServer.h"

#include <iostream>

int main()
{   
    EventLoop loop;
    InetAddress addr("127.0.0.1", 12345);

    chatServer server(&loop, addr, "chatServer");

    server.start();
    loop.loop();

    return 0;
}