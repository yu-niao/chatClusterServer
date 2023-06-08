#ifndef _OFFLINEMESSAGEMODEL_H_
#define _OFFLINEMESSAGEMODEL_H_

#include <string>
#include <vector>

// 处理离线消息
class OffLineMsgModel
{
public:
    void insert(int userid, std::string msg);

    void remove(int userid);

    std::vector<std::string> query(int userid);
};

#endif