#ifndef _GROUPUSER_H_
#define _GROUPUSER_H_

#include "user.h"

class GroupUser : public User
{
public:
    void setRole(std::string role) { role_ = role; }
    std::string getRole() { return role_; }

private:
    std::string role_;
};

#endif