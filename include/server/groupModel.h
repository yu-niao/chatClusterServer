#ifndef GROUPMODEL_H_
#define GROUPMODEL_H_

#include "group.h"

#include <vector>
#include <string>

// 维护群组的方法
class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group& group);
    // 加入群组
    void addGroup(int userid, int groupid, std::string role);
    // 查询用户所在组信息
    std::vector<Group> queryGroups(int userid);
    // 根据指定的groupid查询群组用户id列表，用于群聊业务中群发信息
    std::vector<int> queryGroupUsers(int userid, int groupid);
};

#endif