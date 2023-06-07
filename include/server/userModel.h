#ifndef _USERMODEL_H_
#define _USERMODEL_H_

#include "user.h"

class userModel
{
public:
    // user表增加方法
    bool insert(User& user);

    // 根据号码查询用户信息
    User query(int id);

    // 更改用户状态
    bool updateState(User& user);
};

#endif