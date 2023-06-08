#ifndef _CHATSERVER_H_
#define _CHATSERVER_H_

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class chatServer
{
public:
    chatServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg);

    void start();

private:
    void onConnection(const TcpConnectionPtr&);

    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp);

    TcpServer server_;
    EventLoop* loop_;
};

#endif