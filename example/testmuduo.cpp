#include <iostream>
#include <muduo/net/InetAddress.h>
#include <string>
#include <functional>

#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"

using namespace muduo;
using namespace muduo::net;
using namespace std;

class chatServer
{
public:
    chatServer(EventLoop* loop, InetAddress& listenAddr, const string& nameArg)
        : server_(loop, listenAddr, nameArg)
        , loop_(loop)
    {
        server_.setConnectionCallback(bind(&chatServer::onConnection, this, placeholders::_1));
        server_.setMessageCallback(bind(&chatServer::onMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));

        server_.setThreadNum(4);
    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        cout << "aaaaaaa" << endl;
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv :" << buf << "time : " << time.toString() << endl;
        conn->send(buf);

    }

    TcpServer server_;
    EventLoop* loop_;
};

int main()
{
    EventLoop loop;
    InetAddress addr("127.0.0.1", 12345);
    chatServer server(&loop, addr, "myServer");

    server.start();
    loop.loop();

    return 0;
}