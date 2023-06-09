#ifndef _FRIENDMODEL_H_
#define _FRIENDMODEL_H_

#include "user.h"

#include <vector>

class FriendModel
{
public:
    // 添加好友
    void insert(int userid, int friendid);
    // 获取好友列表
    std::vector<User> query(int userid);
};

#endif