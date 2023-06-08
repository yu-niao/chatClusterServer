#include "chatServer.h"
#include "chatService.h"

#include <iostream>
#include <signal.h>

void resetHandler(int s)
{
    ChatService::getInstance()->reset();
    exit(0);
}

int main()
{   
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr("127.0.0.1", 12345);

    chatServer server(&loop, addr, "chatServer");

    server.start();
    loop.loop();

    return 0;
}