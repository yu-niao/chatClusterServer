#ifndef _PUBLIC_H_
#define _PUBLIC_H_

enum MsgType
{
    LOGIN_MSG = 1,  // 登录消息
    LOGIN_MSG_ACK,  // 登录响应消息
    REG_MSG,    // 注册消息
    REG_MSG_ACK,    // 注册响应消息
    ONE_CHAT_MSG,   // 聊天消息
    ADD_FRIEND_MSG, // 添加好友
};

#endif