#include "chatService.h"
#include "public.h"

#include <iostream>
#include <functional>
#include <mutex>
#include <vector>
#include <string>

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

    msgHandlerMap_.emplace(std::make_pair(ONE_CHAT_MSG, std::bind(
        &ChatService::oneChat, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3
    )));

    msgHandlerMap_.emplace(std::make_pair(ADD_FRIEND_MSG, std::bind(
        &ChatService::addFriend, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3
    )));

    msgHandlerMap_.emplace(std::make_pair(CREATE_GROUP_MSG, std::bind(
        &ChatService::createGroup, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3
    )));

    msgHandlerMap_.emplace(std::make_pair(ADD_GROUP_MSG, std::bind(
        &ChatService::addGroup, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3
    )));

    msgHandlerMap_.emplace(std::make_pair(GROUP_CHAT_MSG, std::bind(
        &ChatService::groupChat, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3
    )));

    msgHandlerMap_.emplace(std::make_pair(LOGINOUT_MSG, std::bind(
        &ChatService::loginout, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3
    )));

    if (redis_.connect())
    {
        // 设置回调
        redis_.initNotifyHandler(std::bind(&ChatService::handleRedisSubscribeMessage, this,
            std::placeholders::_1, std::placeholders::_2));
    }
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
void ChatService::login(const TcpConnectionPtr &conn, json& js, Timestamp time)
{
    int id = js["id"].get<int>();
    std::string pwd = js["password"];

    User user = userModel_.query(id);
    json response;
    
    if (user.getId() == id && user.getPassword() == pwd)
    {

        if (user.getState() == "online")
        {
            // 该用户已经登录
            response["msgId"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this account is already login!";
            conn->send(response.dump());       
        }
        else 
        {
            {
                // 记录用户连接信息
                std::lock_guard<std::mutex> lock(connMutex_);
                userConnMap_.emplace(std::make_pair(id, conn));
            }
            // 登录成功

            redis_.subscribe(id);

            user.setState("online");
            userModel_.updateState(user);

            response["msgId"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            
            // 查询是否有离线信息
            std::vector<std::string> vec = offLineMsgModel_.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                offLineMsgModel_.remove(id);
            }
            // 查询好友信息
            std::vector<User> vec1 = friendModel_.query(id);
            if (!vec1.empty())
            {
                std::vector<std::string> v;
                for (auto& it : vec1)
                {
                    json js1;
                    js1["id"] = it.getId();
                    js1["name"] = it.getName();
                    js1["state"] = it.getState();

                    v.emplace_back(js1.dump());
                }
                response["friends"] = v;
            }
            

            // 查询用户群组信息
            std::vector<Group> groupUserVec = groupModel_.queryGroups(id);
            if (!groupUserVec.empty())
            {
                std::vector<std::string> groupVec;
                for (auto& group : groupUserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();

                    std::vector<std::string> userVec;

                    for (auto& user : group.getUsers())
                    {
                        json js2;
                        js2["id"] = user.getId();
                        js2["name"] = user.getName();
                        js2["state"] = user.getState();
                        js2["role"] = user.getRole();
                        userVec.emplace_back(js2.dump());
                    }
                    grpjson["users"] = userVec;
                    groupVec.emplace_back(grpjson.dump());
                }

                response["groups"] = groupVec;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // 登录失败
        response["msgId"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password is invalid!";
        conn->send(response.dump());       
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

// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    friendModel_.insert(userid, friendid);
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

    redis_.unsubscribe(user.getId());

    if (user.getId() != -1)
    {
        user.setState("offline");
        userModel_.updateState(user);
    }
}

// 处理服务器异常退出
void ChatService::reset()
{
    userModel_.resetState();
}

// 一对一聊天
void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int toid = js["to"].get<int>();

    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = userConnMap_.find(toid);
        if (it != userConnMap_.end())
        {
            // toid 在线
            it->second->send(js.dump());
            return;
        }
    }
    // toid 是否在线
    User user = userModel_.query(toid);
    if (user.getState() == "online")
    {
        redis_.publish(toid, js.dump());
        return;
    }

    // toid 不在线
    offLineMsgModel_.insert(toid, js.dump());
}

//  创建群组
void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int creator = js["id"].get<int>();
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];

    Group group(-1, name, desc);

    if (groupModel_.createGroup(group))
    {
        groupModel_.addGroup(creator, group.getId(), "creator");
    }
}

// 加入群组
void ChatService::addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    groupModel_.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    auto userVec = groupModel_.queryGroupUsers(userid, groupid);

    std::lock_guard<std::mutex> lock(connMutex_);
    for(auto& id: userVec)
    {
        auto it = userConnMap_.find(id);
        if (it != userConnMap_.end())
        {
            it->second->send(js.dump());
        }
        else 
        {
            User user = userModel_.query(id);
            if (user.getState() == "online")
            {
                redis_.publish(id, js.dump());
            }
            else  
            {
                offLineMsgModel_.insert(id, js.dump());
            }
            
        }
    }
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    int id = js["id"].get<int>();

    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = userConnMap_.find(id);
        if (it != userConnMap_.end())
        {
            userConnMap_.erase(it);
        }
    }

    redis_.unsubscribe(id);

    User user(id);
    user.setState("offline");
    userModel_.updateState(user);    
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, std::string msg)
{
    std::lock_guard<std::mutex> lock(connMutex_);
    auto it = userConnMap_.find(userid);
    if (it != userConnMap_.end())
    {
        it->second->send(msg);
        return;
    }

    offLineMsgModel_.insert(userid, msg);
}