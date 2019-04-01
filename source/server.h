#ifndef SERVER_H
#define SERVER_H
#include <QThread>
#include <winsock2.h>
#include <list>
#include <string>
#include "config.h"
using namespace std;

//请求类
class Requestion {
public:
    SOCKET sClient;         //客户套接字
    char  *req;             //请求
    Requestion(SOCKET AcSock);
    Requestion(const Requestion& rqst);
    virtual ~Requestion();
};

class Server:public QThread
{
    Q_OBJECT
protected:

    SOCKET srvSocket;	//服务器socket
    char *sendBuff;		//发送缓冲区
    fd_set rfds;		//用于检查socket是否有数据到来的的文件描述符，用于socket非阻塞模式下等待网络事件通知（有数据到来）
    fd_set wfds;		//用于检查socket是否可以发送的文件描述符，用于socket非阻塞模式下等待网络事件通知（可以发送数据）
    sockaddr_in srvAddr;//服务器端IP地址
    FILE* logfile;      //服务器日志文件
    const Config SrvConfig;   //服务器配置成员
    bool working;       //服务器状态变量

    list<Requestion> *listRqst;						//请求消息队列
    list<SOCKET> *sessions;							//当前的会话socket队列
    list<SOCKET> *closedSessions;					//所有已失效的会话socket队列
    int numOfSocketSignaled;						//可读、写、有请求到来的socket总数

    void run();
    void sendMsg(const string&);//发送信号
protected:
    virtual void AddRequestion(Requestion rqst);							//将收到的客户端消息保存到消息队列
    virtual void AddSession(SOCKET session);								//将新的会话socket加入队列
    virtual void AddClosedSession(SOCKET session);							//将失效的会话socket加入队列
    virtual void RemoveClosedSession(SOCKET closedSession);					//将失效的SOCKET从会话socket队列删除
    virtual void RemoveClosedSession();										//将失效的SOCKET从会话socket队列删除
    virtual void ResponseRqst();											//向其他客户转发信息
    virtual void recvRequestion(SOCKET s);										//从SOCKET s接受消息
    static  string  GetClientAddress(SOCKET s);								//得到客户端IP地址
    virtual void  ReceiveMessageFromClients();								//接受客户端发来的信息
    virtual int AcceptRequestionFromClient();							//等待客户端连接请求
public:
    Server();
    Server(FILE*,const string,const string,const int);
    void stop();
    virtual ~Server(void);
    virtual int ServerStartup();    //初始化Server，包括创建SOCKET，绑定到IP和PORT
    virtual int ListenStartup();    //开始监听客户端请求*/
    virtual int phase_0();          //循环执行"等待客户端请求"->“向其他客户转发信息”->"等待客户端消息"
    virtual int phase_1();
    virtual int phase_2();

    friend class Requestion;
signals:
    void Msg(const string);
    //void finished(bool);
};

#endif // SERVER_H
