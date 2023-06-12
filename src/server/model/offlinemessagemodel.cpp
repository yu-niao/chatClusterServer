#include "offlinemessagemodel.h"
#include "db.h"
#include <mysql/mysql.h>

void OffLineMsgModel::insert(int userid, std::string msg)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "insert into offLineMessage values(%d, '%s')", userid, msg.c_str());

    MySql mysql;
    if (mysql.connect()) 
    {
        mysql.update(sql);
    }
}

void OffLineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "delete from offLineMessage where userid = %d", userid);

    MySql mysql;
    if (mysql.connect()) 
    {
        mysql.update(sql);
    }

}

std::vector<std::string> OffLineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    snprintf(sql, 1024, "select message from offLineMessage where userid = %d", userid);

    std::vector<std::string> v;

    MySql mysql;
    if (mysql.connect())
    {   
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                v.emplace_back(row[0]);
            }

            mysql_free_result(res);
            return v;
        }
    }
    return std::vector<std::string>();
}