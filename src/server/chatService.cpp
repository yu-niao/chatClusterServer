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
    int id = js["id"].get<int>();
    std::string pwd = js["password"];

    User user = userModel_.query(id);

    if (user.getId() != -1 && user.getPassword() == pwd)
    {
        if (user.getState() == "online")
        {
            // 该用户已经登录
            json reponse;
            reponse["msgId"] = LOGIN_MSG_ACK;
            reponse["errno"] = 2;
            reponse["errmsg"] = "该用户已经登录!";
            conn->send(reponse.dump());       
        }
        else 
        {
            {
                // 记录用户连接信息
                std::lock_guard<std::mutex> lock(connMutex_);
                userConnMap_.emplace(std::make_pair(id, conn));
            }
            // 登录成功
            user.setState("online");
            userModel_.updateState(user);

            json reponse;
            reponse["msgId"] = LOGIN_MSG_ACK;
            reponse["errno"] = 0;
            reponse["id"] = user.getId();
            reponse["name"] = user.getName();
            conn->send(reponse.dump());
        }
    }
    else
    {
        // 登录失败
        json reponse;
        reponse["msgId"] = LOGIN_MSG_ACK;
        reponse["errno"] = 1;
        reponse["errmsg"] = "用户名不存在或密码错误!";
        conn->send(reponse.dump());       
    }
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

// 处理客户端异常退出
void ChatService::clientCloseExceptional(const TcpConnectionPtr& conn)
{
    User user;

    {
        std::lock_guard<std::mutex> lock(connMutex_);
        for (auto& it : userConnMap_)
        {
            if (it.second == conn)
            {
                user.setId(it.first);
                userConnMap_.erase(it.first);
                break;
            }
        }
    }

    if (user.getId() != -1)
    {
        user.setState("offline");
        userModel_.updateState(user);
    }
}