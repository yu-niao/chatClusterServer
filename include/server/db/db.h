#ifndef _DB_H_ 
#define _DB_H_

#include <mysql/mysql.h>  
#include <string>


class MySql
{
public:
    MySql();
    ~MySql();

    bool connect();

    bool update(std::string sql);

    MYSQL_RES* query(std::string sql);

    MYSQL* getConnection();

private:
    MYSQL* conn_;

};

#endif