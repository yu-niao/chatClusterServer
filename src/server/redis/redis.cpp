#include "redis.h"

#include <hiredis/hiredis.h>
#include <iostream>


Redis::Redis()
    : publishContext_(nullptr)
    , subscribeContext_(nullptr)
{}

Redis::~Redis()
{
    if (publishContext_ != nullptr)
    {
        redisFree(publishContext_);
    }
    if (subscribeContext_ != nullptr)
    {
        redisFree(subscribeContext_);
    } 
}

bool Redis::connect()
{
    publishContext_ = redisConnect("127.0.0.1", 6379);
    if (publishContext_ == nullptr)
    {
        std::cerr << "redis connect failed!" << std::endl;
        return false;
    }

    subscribeContext_ = redisConnect("127.0.0.1", 6379);
    if (subscribeContext_ == nullptr)
    {
        std::cerr << "redis connect failed!" << std::endl;
        return false;
    }

    // 在单独线程中监听通道上的事件
    std::thread t([&](){
        observerChannelMessage();
    });
    t.detach();

    std::cout << "connect redis-server success!" << std::endl;

    return true;
}

bool Redis::publish(int channel, std::string message)
{
    redisReply* reply = (redisReply*)redisCommand(publishContext_, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr)
    {
        std::cerr << "publish command failed!" << std::endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel)
{
    if (redisAppendCommand(subscribeContext_, "SUBSCRIBE %d", channel) == REDIS_ERR)
    {
        std::cerr << "subscribe command failed!" << std::endl;
        return false;
    }
    int done = 0;
    while(!done)
    {
        if (redisBufferWrite(subscribeContext_, &done) == REDIS_ERR)
        {
            std::cerr << "subscribe command failed!" << std::endl;
            return false;
        }
    }

    return true;
}

bool Redis::unsubscribe(int channel)
{
    if (redisAppendCommand(subscribeContext_, "UNSUBSCRIBE %d", channel) == REDIS_ERR)
    {
        std::cerr << "unsubscribe command failed!" << std::endl;
        return false;
    }
    int done = 0;
    while(!done)
    {
        if (redisBufferWrite(subscribeContext_, &done) == REDIS_ERR)
        {
            std::cerr << "unsubscribe command failed!" << std::endl;
            return false;
        }
    }

    return true;
}

// 在独立线程中接收订阅通道中的消息
void Redis::observerChannelMessage()
{
    redisReply* reply = nullptr;
    while(redisGetReply(subscribeContext_, (void**)&reply))
    {
        // 订阅收到的消息是一个带三个元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            notifyMessageHandler_(atoi(reply->element[1]->str), reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    std::cerr << "observerChannelMessage quit <<<<<<<<<" << std::endl;
}

// 初始化向业务层上报通道消息的回调对象
void Redis::initNotifyHandler(std::function<void(int, std::string)> fn)
{
    this->notifyMessageHandler_ = fn;
}