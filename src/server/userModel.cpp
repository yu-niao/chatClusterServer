#include "userModel.h"
#include "db.h"
#include <cstdio>
#include <mysql/mysql.h>

bool userModel::insert(User& user)
{
    char buf[1024] = {0};
    snprintf(buf, 1024, "insert into user(name, password, state) values('%s', '%s', '%s')", 
            user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());

    MySql mysql;
    if (mysql.connect())
    {
        if (mysql.update(buf))
        {
            // 获取插入成功的用户数据生成的主键
            user.setId(mysql_insert_id(mysql.getConnection()));

            return true;
        }
    }

    return false;
}

// 根据号码查询用户信息
User userModel::query(int id)
{
    char buf[1024] = {0};
    snprintf(buf, 1024, "select* from user where id = %d", id);

    MySql mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(buf);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);

                mysql_free_result(res);

                return user;
            }
        }
    }

    return User();
}

// 更改用户状态
bool userModel::updateState(User& user)
{
    char buf[1024] = {0};
    snprintf(buf, 1024, "update user set state = '%s' where id = %d", 
            user.getState().c_str(), user.getId());

    MySql mysql;
    if (mysql.connect())
    {
        if (mysql.update(buf))
        {
            return true;
        }
    }
    return false;
}