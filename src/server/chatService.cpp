#include "chatService.h"
#include "public.h"

#include <iostream>
#include <functional>

// 获取单例
ChatService* ChatService::getInstance()
{
    static ChatService service;
    return &service;
}

ChatService::ChatService()
{
    msgHandlerMap_.emplace(std::make_pair(LOGIN_MSG, std::bind(
        &ChatService::login, this, std::placeholders::_1, 
        std::placeholders::_2, std::placeholders::_3
    )));

    msgHandlerMap_.emplace(std::make_pair(REG_MSG, std::bind(
        &ChatService::reg, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3
    )));
}

// 获取消息对应的处理器
msgHandler ChatService::getHandler(int msgId)
{
    auto it = msgHandlerMap_.find(msgId);
    if (it != msgHandlerMap_.end())
    {
        return msgHandlerMap_[msgId];
    }
    else
    {
        return [=](const TcpConnectionPtr& conn, json& js, Timestamp time) {
            std::cout << "msgId not find" << std::endl;
        };
    }
}

// 处理登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::cout << "do login" << std::endl;
}

// 处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    std::string name = js["name"];
    std::string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPassword(pwd);

    if (userModel_.insert(user))
    {
        // 注册成功
        json response;
        response["msgId"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgId"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}