#include "userModel.h"
#include "db.h"

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