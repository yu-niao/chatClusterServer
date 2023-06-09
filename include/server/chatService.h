#ifndef _CHATSERVICE_H_ 
#define _CHATSERVICE_H_

#include "json.hpp"
#include "userModel.h"
#include "offlinemessagemodel.h"
#include "friendModel.h"
#include "groupModel.h"
#include "redis.h"

#include <functional>
#include <muduo/net/Callbacks.h>
#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include <mutex>

using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
// 处理消息的事件回调方法
using msgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

// 聊天服务器业务类
class ChatService
{
public:
    // 获取单例
    static ChatService* getInstance();

    // 处理登录业务
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 处理注册业务
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 一对一聊天
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 创建群组
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 加入群组
    void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 群组聊天业务
    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 处理注销业务
    void loginout(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // 获取消息对应的处理器
    msgHandler getHandler(int msgId);
    // 处理客户端异常退出
    void clientCloseExceptional(const TcpConnectionPtr&);
    // 处理服务器异常退出
    void reset();
    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, std::string);

    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) = delete;

private:
    ChatService();
    
    // 存储消息id及对应的业务处理方法
    std::unordered_map<int, msgHandler> msgHandlerMap_;

    // 存储在线用户的通信连接
    std::unordered_map<int, TcpConnectionPtr> userConnMap_;

    // 数据库操作类对象
    userModel userModel_;
    OffLineMsgModel offLineMsgModel_;
    FriendModel friendModel_;
    GroupModel groupModel_;

    // redis 操作对象
    Redis redis_;

    std::mutex connMutex_;
};

#endif