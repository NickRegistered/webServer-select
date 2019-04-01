#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <winsock2.h>
using namespace std;

class Config
{
public:
    const string root;
    const int MAXCONNECTION;	//最大连接数
    const int BUFFERLENGTH;		//缓冲区大小
    const string SERVERADDRESS; //服务器地址
    const int PORT;				//服务器端口
    const u_long BLOCKMODE;			//SOCKET阻塞模式
    Config(void);
    ~Config(void);
    Config(const string root,const string IP,const int port);
};

#endif // CONFIG_H
