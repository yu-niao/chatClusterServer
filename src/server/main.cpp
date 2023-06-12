#include "chatServer.h"
#include "chatService.h"

#include <iostream>
#include <signal.h>

void resetHandler(int s)
{
    ChatService::getInstance()->reset();
    exit(0);
}

int main(int argc, char* argv[])
{   
    if (argc < 3)
    {
        std::cerr << "Usage: ./chatServer 127.0.0.1 6000/6002 !" << std::endl;
    }

    signal(SIGINT, resetHandler);
    signal(SIGABRT, resetHandler);
    signal(SIGSEGV, resetHandler);

    const char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    EventLoop loop;
    InetAddress addr(ip, port);

    chatServer server(&loop, addr, "chatServer");

    server.start();
    loop.loop();

    return 0;
}