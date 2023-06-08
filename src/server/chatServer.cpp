#include "chatServer.h"
#include "json.hpp"
#include "chatService.h"

#include <functional>
#include <string>
using json = nlohmann::json;

chatServer::chatServer(EventLoop* loop, 
        const InetAddress& listenAddr, 
        const std::string& nameArg)
        : server_(loop, listenAddr, nameArg)
        , loop_(loop)
{
    server_.setConnectionCallback(std::bind(
        &chatServer::onConnection, this, std::placeholders::_1
    ));

    server_.setMessageCallback(std::bind(
        &chatServer::onMessage, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3
    ));

    server_.setThreadNum(4);

}

void chatServer::start()
{
    server_.start();
}

void chatServer::onConnection(const TcpConnectionPtr& conn)
{
    if (!conn->connected())
    {
        ChatService::getInstance()->clientCloseExceptional(conn);
        conn->shutdown();
    }
}

void chatServer::onMessage(const TcpConnectionPtr& conn, 
                        Buffer* buffer, 
                        Timestamp time)
{
    std::string buf = buffer->retrieveAllAsString();
    json js = json::parse(buf);

    auto msgHandler = ChatService::getInstance()->getHandler(js["msgId"].get<int>());
    msgHandler(conn, js, time);
}