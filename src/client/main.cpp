#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <string>
#include <thread>
#include <unordered_map>
#include <atomic>

#include "user.h"
#include "group.h"
#include "public.h"

#include "json.hpp"
using json = nlohmann::json;

// 当前登录用户
User g_currentUser;
// 当前用户的好友列表信息
std::vector<User> g_currentUserFriendList;
// 当前用户的群组列表信息
std::vector<Group> g_currentUserGroupList;
// 显示当前成功登录用户的基本信息
void showCurrentUserData();
// 控制聊天页面退出
std::atomic_bool isMainMenuRunning = false;

// 接收线程
void readTaskHandler(int clientfd);
// 获取系统时间
std::string getCurrentTime();
// 主聊天页面
void mainMnu();

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: ./chatClient 127.0.0.1 12345." << std::endl;
        exit(1);
    }

    const char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        std::cerr << "socket create error" << std::endl;
        exit(1);
    }

    sockaddr_in cliaddr;
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &cliaddr.sin_addr.s_addr);

    if (connect(clientfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr)) == -1)
    {
        std::cerr << "connect server error" << std::endl;
        close(clientfd);
        exit(1);
    }

    while(1)
    {
        std::cout << "======================" << std::endl;
        std::cout << "1. login" << std::endl;
        std::cout << "2. register" << std::endl;
        std::cout << "3. quit" << std::endl;
        std::cout << "======================" << std::endl;
        std::cout << "Enter your choice:";
        int choice = 0;
        std::cin >> choice;
        std::cin.get(); // 读掉缓冲区残留的回车

        switch(choice)
        {
        case 1:
        {
            int id = 0;
            char pwd[50] = {0};
            std::cout << "userid: ";
            std::cin >> id;
            std::cin.get();
            std::cout << "userpassword: ";
            std::cin.getline(pwd, 50);

            json js;
            js["msgId"] = REG_MSG;
            js["id"] = id;
            js["password"] = pwd;
            std::string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                std::cerr << "send login message error" << std::endl;
            }
            else
            {
                char buffer[1024] = {0};
                int len = recv(clientfd, buffer, sizeof(buffer), 0);
                if (len == -1)
                {
                    std::cerr << "recv login message error" << std::endl;
                }
                else 
                {
                    json responsejs = json::parse(buffer);
                    if (responsejs["errno"].get<int>() != 0)
                    {
                        std::cerr << responsejs["errmsg"] << std::endl;
                    }
                    else
                    {
                        g_currentUser.setId(responsejs["id"].get<int>());
                        g_currentUser.setName(responsejs["name"]);

                        // 记录当前用户的好友列表信息
                        if (responsejs.contains("friends"))
                        {
                            // 初始化
                            g_currentUserFriendList.clear();

                            std::vector<std::string>  vec = responsejs["friends"];
                            for (auto& str : vec)
                            {
                                json j = json::parse(str);
                                User user;
                                user.setId(j["id"].get<int>());
                                user.setName(j["name"]);
                                user.setState(j["state"]);
                                g_currentUserFriendList.emplace_back(user);
                            }
                        }

                        // 记录当前用户的群组列表信息
                        if (responsejs.contains("groups"))
                        {
                            // 初始化
                            g_currentUserGroupList.clear();

                            std::vector<std::string> groupVec = responsejs["groups"];
                            for (auto& group : groupVec)
                            {
                                json j1 = json::parse(group);
                                Group grp;
                                grp.setId(j1["id"].get<int>());
                                grp.setName(j1["groupname"]);
                                grp.setDesc(j1["groupdesc"]);
                                
                                std::vector<std::string> uservec = j1["users"];
                                for (auto& userstr : uservec)
                                {
                                    GroupUser user;
                                    json j2 = json::parse(userstr);
                                    user.setId(j2["id"]);
                                    user.setName(j2["name"]);
                                    user.setState(j2["state"]);
                                    user.setRole(j2["role"]);
                                    grp.getUsers().emplace_back(user);
                                }

                                g_currentUserGroupList.emplace_back(grp);
                            }
                        }

                        // 显示用户登录的基本信息
                        showCurrentUserData();

                        // 显示当前用户的离线消息
                        if (responsejs.contains("offlinemsg"))
                        {
                            std::vector<std::string> vec = responsejs["offlinemsg"];

                            for (auto &str : vec) 
                            {
                                json js = json::parse(str);
                                if (ONE_CHAT_MSG == js["msgId"].get<int>()) 
                                {
                                    std::cout << js["time"].get<std::string>() << "[" << js["id"] << "]" << js["name"].get<std::string>() << " said:"
                                             << js["msg"].get<std::string>() << std::endl;          
                                }

                                if (GROUP_CHAT_MSG == js["msgId"].get<int>()) 
                                {
                                    std::cout << "群消息[" << js["groupid"] << "]" << js["time"].get<std::string>() << "[" << js["id"] << "]"
                                        << js["name"].get<std::string>() << " said:" << js["msg"].get<std::string>() << std::endl;      
                                }
                            }
                        }

                        static int readThreadNum = 0;
                        if (readThreadNum == 0)
                        {
                            std::thread readTask(readTaskHandler, clientfd);
                            readTask.detach();
                            readThreadNum++;
                        }
                        
                        isMainMenuRunning = true;
                        mainMnu();
                    }
                }
            }

        }
        break;
        case 2:
        {
            char name[50] = {0};
            char pwd[50] = {0};
            std::cout << "username: ";
            std::cin.getline(name, 50);
            std::cout << "userpassword: ";
            std::cin.getline(pwd, 50);

            json js;
            js["msgId"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            std::string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                std::cerr << "send message error" << std::endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, sizeof(buffer), 0);
                if (len == -1)
                {
                    std::cerr << "recv register message failed" << std::endl;
                }
                else
                {
                    json response = json::parse(buffer);
                    if (response["msgId"].get<int>())
                    {
                        std::cout << name << "is already exist, register error" << std::endl;
                    }
                    else
                    {
                        std::cout << name << "register success, and your userid is " << response["id"].get<int>() << std::endl;
                    }
                }
            }
        }
        break;
        case 3:
        {
            close(clientfd);
            exit(0);
        }
        default:
            std::cerr << "invalid input" << std::endl;
            break;
        }
    }

    return 0;
}

void help(int fd = 0, std::string str = "");
void chat(int, std::string);
void addfriend(int, std::string);
void creategroup(int, std::string);
void addgroup(int, std::string);
void groupchat(int, std::string);
void loginout(int = 0, std::string = "");


std::unordered_map<std::string, std::string> commandMap = {
  {"help", "显示所有支持的命令,格式help"},
  {"chat", "一对一聊天,格式chat:friendid:message"},
  {"addfriend", "添加好友,格式addfriend:friendid"},
  {"creategroup", "添加群组,格式creategroup:groupname:groupdesc"},
  {"addgroup", "加入群组,格式addgroup:groupid"},
  {"groupchat", "群聊,格式groupchat:groupid:message"},
  {"loginout", "注销,格式quit"}
};

std::unordered_map<std::string, std::function<void(int, std::string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}    
};

// 接收线程
void readTaskHandler(int clientfd)
{
    while(1)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (len == -1)
        {
            close(clientfd);
            std::cerr << "thread recv error" << std::endl;
            exit(1);
        }

        json js = json::parse(buffer);
        int msgType = js["msgId"].get<int>();
        if (ONE_CHAT_MSG == msgType)
        {
            std::cout << js["time"].get<std::string>() << "[" << js["id"] << "]" << js["name"].get<std::string>()
            << " said:" << js["msg"].get<std::string>() << std::endl;
            continue;
        }

        if (GROUP_CHAT_MSG == msgType)
        {
            std::cout << "群消息[" << js["groupid"] << "]" << js["time"].get<std::string>() << "[" << js["id"] << "]" << js["name"].get<std::string>()
            << " said:" << js["msg"].get<std::string>() << std::endl;
            continue;          
        }
    }
}

void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while(isMainMenuRunning)
    {
        std::cin.getline(buffer, 1024);
        std::string commandbuf(buffer);
        std::string command;    // 存储命令
        int idx = commandbuf.find(":");
        if (idx == -1)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }

        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            std::cerr << "invalid input command!" << std::endl;
            continue;
        }

        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }
}
// 帮助界面
void help(int fd, std::string str)
{
    std::cout << "show command list >>>" << std::endl;
    for (auto& it : commandMap)
    {
        std::cout << it.first << ": " << it.second << std::endl;
    }
    std::cout << std::endl;
}
// 添加好友
void addfriend(int clientfd, std::string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgId"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;

    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), sizeof(buffer), 0);
    if (len == -1)
    {
        std::cerr << "send addfriend message failed!" << std::endl;
    }
}
// 一对一聊天
void chat(int clientfd, std::string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        std::cerr << "invalid command" << std::endl;
        return;
    }
    int friendid = atoi(str.substr(0, idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgId"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["to"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), sizeof(buffer), 0);
    if (len == -1)
    {
        std::cerr << "send chat message failed!" << std::endl;
    }
}

// 创建群组
void creategroup(int clientfd, std::string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        std::cerr << "invalid command" << std::endl;
        return;
    }
    std::string groupname = str.substr(0, idx);
    std::string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgId"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;

    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), sizeof(buffer), 0);
    if (len == -1)
    {
        std::cerr << "ctrate group failed!" << std::endl;
    }
}

// 加群
void addgroup(int clientfd, std::string str)
{
    json js;
    js["msgId"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = atoi(str.c_str());

    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), sizeof(buffer), 0);
    if (len == -1)
    {
        std::cerr << "add group failed!" << std::endl;
    }    
}

// 群聊
void groupchat(int clientfd, std::string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        std::cerr << "invalid command" << std::endl;
        return;
    }
    int groupid = atoi(str.substr(0, idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);   

    json js;
    js["msgId"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), sizeof(buffer), 0);
    if (len == -1)
    {
        std::cerr << "chat in group failed!" << std::endl;
    } 
}

// 注销
void loginout(int clientfd, std::string str)
{
    json js;
    js["msgId"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();

    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), sizeof(buffer), 0);
    if (len == -1)
    {
        std::cerr << "loginout failed!" << std::endl;
    } 
    else
    {
        isMainMenuRunning = false;
    }
}

void showshowCurrentUserData()
{
    std::cout << "======================login user======================" << std::endl;
    std::cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << std::endl;
    std::cout << "----------------------friend list---------------------" << std::endl;
    if (!g_currentUserFriendList.empty())
    {
        for (auto& user : g_currentUserFriendList)
        {
            std::cout << user.getId() << " " << user.getName() << " " << user.getState() << std::endl;
        }
    }
    std::cout << "----------------------group list----------------------" << std::endl;
    if (!g_currentUserGroupList.empty())
    {
        for (auto& group : g_currentUserGroupList)
        {
            std::cout << group.getId() << " " << group.getName() << " " << group.getDesc() << std::endl;
            for (auto& user : group.getUsers())
            {
                std::cout << user.getId() << " " << user.getName() << " " << user.getState() << " " << user.getRole() << std::endl;
            }
        }
    }
    std::cout << "======================================================" << std::endl;
}

// 获取系统时间
std::string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}