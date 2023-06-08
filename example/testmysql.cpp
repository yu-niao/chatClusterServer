 #include <iostream>
 #include <mysql/mysql.h>
 using namespace std;
 
 int main()
 {
    MYSQL* mysql;
    mysql = mysql_init(nullptr);

    MYSQL* p = mysql_real_connect(mysql, "127.0.0.1", "root", "654826hxj..", "school", 3306, nullptr, 0);
    if (p != nullptr)
    {
        cout << "connect  success" << endl;
    }
    else 
    {
        cout << "failed" << endl;
        cout << mysql_error(mysql) << endl;
    }

    mysql_close(mysql);

    return 0;
 }