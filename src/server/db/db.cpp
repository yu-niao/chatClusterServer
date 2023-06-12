#include "db.h"

#include <mysql/mysql.h>
#include <string>
#include <muduo/base/Logging.h>
#include <iostream>

static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "654826hxj..";
static std::string dbname = "chat";

MySql::MySql()
{
    conn_ = mysql_init(nullptr);
}

MySql::~MySql()
{
    if (conn_ != nullptr)
    {
        mysql_close(conn_);
    }
}

bool MySql::connect()
{
    MYSQL* p = mysql_real_connect(conn_, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        // 设置字符编码
        mysql_query(conn_, "set names gbk");
        LOG_INFO << "connect mysql success!";
        return true;
    }
    LOG_INFO << "connect mysql failed!";
    std::cout << "reason is : " << mysql_errno(conn_) << std::endl;
    return false;
}

bool MySql::update(std::string sql)
{
    if (mysql_query(conn_, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败!";
        std::cout << mysql_error(conn_) << std::endl;
        return false;
    }
    return true;
}

MYSQL_RES* MySql::query(std::string sql)
{
    if (mysql_query(conn_, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << sql << "查询失败!";
        return nullptr;
    }
    return mysql_use_result(conn_);
}

MYSQL* MySql::getConnection()
{
    return conn_;
}