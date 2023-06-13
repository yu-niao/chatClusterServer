#include "friendModel.h"
#include "db.h"


// 添加好友
void FriendModel::insert(int userid, int friendid)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "insert into friend values(%d, %d)", userid, friendid);

    MySql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 获取好友列表
std::vector<User> FriendModel::query(int userid)
{
    std::vector<User> v;

    char sql[1024] = {0};
    snprintf(sql, 1024, 
    "select a.id, a.name, a.state from user a inner join friend b on a.id = b.friendid where b.userid = %d", userid);

    MySql mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);

                v.emplace_back(user);
            }   
            mysql_free_result(res);

            return v;
        }
    }

    return v;
}