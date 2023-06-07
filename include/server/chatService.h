#ifndef _CHATSERVICE_H_ 
#define _CHATSERVICE_H_

#include "json.hpp"
#include "userModel.h"

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
    // 获取消息对应的处理器
    msgHandler getHandler(int msgId);
    // 处理客户端异常退出
    void clientCloseExceptional(const TcpConnectionPtr&);

    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) = delete;

private:
    ChatService();
    
    // 存储消息id及对应的业务处理方法
    std::unordered_map<int, msgHandler> msgHandlerMap_;

    // 存储在线用户的通信连接
    std::unordered_map<int, TcpConnectionPtr> userConnMap_;

    // 数据操作类对象
    userModel userModel_;

    std::mutex connMutex_;
};

#endif