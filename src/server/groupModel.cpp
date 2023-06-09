#include "groupModel.h"
#include "db.h"
#include "groupuser.h"

#include <mysql/mysql.h>

// 创建群组
bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "insert into allGroup(groupname, groupdesc) values('%s', '%s')",
        group.getName().c_str(), group.getDesc().c_str());

    MySql mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}

// 加入群组
void GroupModel::addGroup(int userid, int groupid, std::string role)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "insert into groupUser values(%d, %d, '%s')",
        userid, groupid, role.c_str());

    MySql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }    
}

// 查询用户所在组信息
std::vector<Group> GroupModel::queryGroups(int userid)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "select a.id, a.name, a.desc from allGroup a    \
    join groupUser b on a.id = b.groupid where b.userid = %d", userid);

    std::vector<Group> groupVec;
    MySql mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);

                groupVec.emplace_back(group);
            }

            mysql_free_result(res);
        }
    }

    // 查询群组用户信息
    for (auto& group : groupVec)
    {
        snprintf(sql, 1024, "select a.id, a.name, a.state, b.grouprole from user a \
        join groupUser b on b.userid = a.id where b.groupid = %d", group.getId());

        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().emplace_back(user);
            }
            mysql_free_result(res);
        }
    }

    return groupVec;
}

// 根据指定的groupid查询群组用户id列表，用于群聊业务中群发信息
std::vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "select userid from groupUser where\
    groupid = %d and userid != %d", groupid, userid);

    std::vector<int> idVec;
    MySql mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.emplace_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    
    return idVec;
}