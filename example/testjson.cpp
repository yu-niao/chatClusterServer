#include "json.hpp"

#include <vector>
#include <map>
#include <iostream>
#include <string>

using json = nlohmann::json;
using namespace std;

void test01()
{
    json js1;
    js1["msg_type"] = 2;
    js1["name"] = "jun";
    js1["msg"] = "jin tian you ao ye le";

    cout << js1 << endl;

    string sendBuf = js1.dump();

    cout << sendBuf.c_str() << endl;
}

// json 序列化容器
void test02()
{
    json js;
    vector<int> v{ 1, 2, 3, 4 };
    js["list"] = v;

    map<int, string> m{ {1, "jun"}, {2, "wei"}, {3, "hua"}, {4, "tian"} };
    js["name"] = m;

    cout << js << endl;
}

int main()
{
    test02();

    return 0;
}