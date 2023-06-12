#ifndef _REDIS_H_
#define _REDIS_H_

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>

class Redis
{
public:
    Redis();
    ~Redis();

    bool connect();

    bool publish(int channel, std::string message);

    bool subscribe(int channel);

    bool unsubscribe(int channel);

    // 在独立线程中接收订阅通道中的消息
    void observerChannelMessage();

    // 初始化向业务层上报通道消息的回调对象
    void initNotifyHandler(std::function<void(int, std::string)> fn);

private:
    // hiredis同步上下文对象,负责publish消息
    redisContext* publishContext_;
    // hiredis同步上下文对象,负责subscribe消息
    redisContext* subscribeContext_;
    // 回调操作,收到订阅消息,给service层上报
    std::function<void(int, std::string)> notifyMessageHandler_;
};

#endif