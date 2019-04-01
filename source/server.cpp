#include "server.h"
#include "winsockenv.h"
#include "response.h"
#include <winsock2.h>

Requestion::Requestion(SOCKET AcSock) :sClient(AcSock){
    req = new char[512];
    int receivedBytes = recv(this->sClient, this->req,512, 0);
    if (receivedBytes == SOCKET_ERROR || receivedBytes == 0) {//接受数据错误
        delete req;
        req = nullptr;
    }
    else {//成功接收到数据
        req[receivedBytes] = '\0';
    }
}

Requestion::Requestion(const Requestion& rqst) :sClient(rqst.sClient){
    req = new char[512];
    int size = strlen(rqst.req);//复制请求内容
    for (int i = 0;i <= size;++i) {
        req[i] = rqst.req[i];
    }
}

Requestion::~Requestion() {
    if(req != nullptr)
        delete[] this->req;
    req = nullptr;
}


Server::Server(){
}

Server::Server(FILE* logfile,const string root,const string IP,const int port):\
 SrvConfig(root,IP,port)
{
    this->sendBuff = new char[SrvConfig.BUFFERLENGTH]; //初始化接受缓冲区
    memset(this->sendBuff, '\0', SrvConfig.BUFFERLENGTH);

    this->listRqst = new list<Requestion>();
    this->sessions = new list<SOCKET>();
    this->closedSessions = new list<SOCKET>();

    this->logfile = logfile;
}

Server::~Server(){


    //释放发送缓冲区
    if (this->sendBuff != nullptr) {
        delete this->sendBuff;
        this->sendBuff = nullptr;
    }

    //关闭server socket
    if (this->srvSocket != NULL) {
        closesocket(this->srvSocket);
        this->srvSocket = NULL;
    }

    //关闭所有会话socket并释放会话队列
    if (this->sessions != nullptr) {
        for (list<SOCKET>::iterator itor = this->sessions->begin();itor != this->sessions->end();itor++)
            closesocket(*itor); //关闭会话
        delete this->sessions;  //释放队列
        this->sessions = nullptr;
    }
    //释放失效会话队列
    if (this->closedSessions != nullptr) {
        for (list<SOCKET>::iterator itor = this->closedSessions->begin();itor != this->closedSessions->end();itor++)
            closesocket(*itor); //关闭会话
        delete this->closedSessions;//释放队列
        this->closedSessions = nullptr;
    }

    //释放接受消息队列
    if (this->listRqst != nullptr) {
        this->listRqst->clear(); //清除消息队列中的消息
        delete this->listRqst;	// 释放消息队列
        this->listRqst = nullptr;
    }

    WSACleanup(); //清理winsock 运行环境
    this->wait();
    this->quit();
}

//将收到的客户端消息保存到消息队列
void Server::AddRequestion(Requestion rqst){
if (listRqst != nullptr && rqst.sClient != INVALID_SOCKET)
        this->listRqst->insert(this->listRqst->end(), rqst);

}

//将新的会话SOCKET加入队列
void Server::AddSession(SOCKET session) {
if (sessions != nullptr && session != INVALID_SOCKET) {
        this->sessions->insert(this->sessions->end(), session);
    }
}

//将失效的会话SOCKET加入队列
void Server::AddClosedSession(SOCKET session) {
if (closedSessions != nullptr && session != INVALID_SOCKET) {
        this->closedSessions->insert(this->closedSessions->end(), session);
    }
}

//将失效的SOCKET从会话SOCKET队列删除
void Server::RemoveClosedSession(SOCKET closedSession) {
if (sessions !=nullptr && closedSession != INVALID_SOCKET) {
        list<SOCKET>::iterator itor = find(this->sessions->begin(), this->sessions->end(), closedSession);
        if (itor != this->sessions->end())
            this->sessions->erase(itor);
    }
}

//将失效的SOCKET从会话SOCKET队列删除
void Server::RemoveClosedSession() {
if(closedSessions != nullptr && !(closedSessions->empty()))
    for (list<SOCKET>::iterator itor = this->closedSessions->begin();itor != this->closedSessions->end();itor++) {
        this->RemoveClosedSession(*itor);
    }
}

void Server::sendMsg(const string& str){
    fwrite(str.c_str(),str.size(),1,logfile);//将信息写入日志文件
    emit Msg(str);
}

void Server::run(){
    working = true;
    phase_0();
    while(working){
        phase_1();
        phase_2();
    }
    return;
}

void Server::stop(){
    this->working = false;
}

int Server::phase_0(){

    switch(WinsockEnv::Startup()){
    case(-1):
        sendMsg("Winsock startup error!\n");
        return -1;
    case(-2):
        sendMsg("Winsock version error!\n");
        return -1;
    case(0):
        sendMsg("Winsock startup ok!\n");
    }

    if(ServerStartup() == -1) return -1;
    if(ListenStartup() == -1) return -1;

    //设置服务器阻塞模式
    u_long blockMode = SrvConfig.BLOCKMODE;//将srvSock设为非阻塞模式以监听客户连接请求
    int rtn;

    if ((rtn = ioctlsocket(this->srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
        sendMsg("ioctlsocket() failed with error!\n");
        return -1;
    }
    sendMsg("ioctlsocket() for server socket ok!Waiting for client connection and data\n");


    return 0;
}

int Server::ServerStartup(){
    //创建 TCP socket
        this->srvSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (this->srvSocket == INVALID_SOCKET) {
            WSACleanup();
            sendMsg("Server socket creare error !\n");
            return 1;
        }
        sendMsg("Server socket create ok!\n");
        //设置服务器IP地址和端口号
        this->srvAddr.sin_family = AF_INET;
        this->srvAddr.sin_port = htons(this->SrvConfig.PORT);

        this->srvAddr.sin_addr.S_un.S_addr = inet_addr(SrvConfig.SERVERADDRESS.c_str()); //这是另外一种设置IP地址的方法

        //bind socket to Server's IP and port
        int rtn = bind(this->srvSocket, (LPSOCKADDR)&(this->srvAddr), sizeof(this->srvAddr));
        if (rtn == SOCKET_ERROR) {
            sendMsg("Server socket bind error!\n");
            closesocket(this->srvSocket);
            WSACleanup();
            return -1;
        }

        sendMsg("Server socket bind ok!\n");
        return 0;
}

int Server::ListenStartup() {
    int rtn = listen(this->srvSocket,SrvConfig.MAXCONNECTION);
    if (rtn == SOCKET_ERROR) {
        sendMsg("Server socket listen error!\n");
        closesocket(this->srvSocket);
        WSACleanup();
        return -1;
    }

    sendMsg("Server socket listen ok!\n");
    return 0;
}


//检查请求队列，发送请求文件
void Server::ResponseRqst() {
    if (this->numOfSocketSignaled > 0 && listRqst) {
        if (!this->listRqst->empty()) {//如果请求队列不为空
            for (list<Requestion>::iterator itor = this->listRqst->begin();itor != this->listRqst->end();) {//对消息队列中的每一个请求
                if (FD_ISSET(itor->sClient, &this->wfds)) {
                    Response rsp(itor->req);
                    sendMsg("Method: "+string(rsp.Method)+"\n"+\
                             "URL: "+string(rsp.URL)+"\n"+\
                             "IP: "+GetClientAddress(itor->sClient)+"\n");
                    string filename = SrvConfig.root + rsp.URL;
                    rsp.ResponseHeader(filename.c_str(), sendBuff);

                    send(itor->sClient, sendBuff,strlen(sendBuff), 0);

                    FILE* fin = fopen(filename.c_str(), "rb");

                    if (!fin) {//打开文件失败
                        sendMsg("\nOpen file :"+filename+" failed!\n");
                    }

                    else{
                        int size;
                        sendMsg("sending file: " + filename  +"\n");
                        while (!feof(fin)) {
                            size = fread(sendBuff, 1, SrvConfig.BUFFERLENGTH - 1, fin);
                            send(itor->sClient, sendBuff, size, 0);
                        }
                        sendMsg("\nfile sending succeeded!\n");
                        fclose(fin);
                    }
                }
                itor = listRqst->erase(itor);
            }
        }
    }
}


int Server::AcceptRequestionFromClient() {
    sockaddr_in clientAddr;		//客户端IP地址
    int nAddrLen = sizeof(clientAddr);
    u_long blockMode = SrvConfig.BLOCKMODE;//将session socket设为非阻塞模式以监听客户连接请求

                                         //检查srvSocket是否收到用户连接请求
    if (this->numOfSocketSignaled > 0) {
        if (FD_ISSET(this->srvSocket, &rfds)) {  //有客户连接请求到来
            this->numOfSocketSignaled--;

            //产生会话socket
            SOCKET newSession = accept(this->srvSocket, (LPSOCKADDR)&(clientAddr), &nAddrLen);
            if (newSession == INVALID_SOCKET) {
                sendMsg("Server accept connection request error!\n");
                return -1;
            }

            //"New client connection request arrived and new session created\n";

            //将新的会话socket设为非阻塞模式，
            if (ioctlsocket(newSession, FIONBIO, &blockMode) == SOCKET_ERROR) {
                sendMsg("ioctlsocket() for new session failed with error!\n");
                return -1;
            }

            //将新的session加入会话队列
            this->AddSession(newSession);
            sendMsg("Connection form IP: " + this->GetClientAddress(newSession) +'\n');
        }
    }
    return 0;
}

void Server::ReceiveMessageFromClients() {
    if (this->numOfSocketSignaled > 0 && sessions) {
        //遍历会话列表中的所有socket，检查是否有数据到来
        for (list<SOCKET>::iterator itor = this->sessions->begin();itor != this->sessions->end();itor++) {
            if (FD_ISSET(*itor, &rfds)) {  //某会话socket有数据到来
                                           //接受数据
                recvRequestion(*itor);
            }
        }//end for
    }
}

//从SOCKET s接受请求
void Server::recvRequestion(SOCKET socket) {
    Requestion newRequestion(socket);
    if (newRequestion.req == nullptr) {//错误的请求加入到失效会话队列中
        this->AddClosedSession(socket);
        sendMsg("receive requestion erorr from IP: " + Server::GetClientAddress(socket) +'\n');
    }
    else {
        AddRequestion(newRequestion);//正确的请求加入请求队列
        sendMsg("\n\nOne requestion received!\n");
    }
}

//得到客户端IP地址
string  Server::GetClientAddress(SOCKET s) {
    string clientAddress; //clientAddress是个空字符串， clientAddress.empty() is true
    sockaddr_in clientAddr;
    int nameLen, rtn;

    nameLen = sizeof(clientAddr);
    rtn = getsockname(s, (LPSOCKADDR)&clientAddr, &nameLen);
    if (rtn != SOCKET_ERROR) {
        clientAddress += inet_ntoa(clientAddr.sin_addr);
    }

    return clientAddress;
}

int Server::phase_2(){
    //首先从会话SOCKET队列中删除掉已经关闭的会话socket
    this->RemoveClosedSession();
    //Prepare the read and write socket sets for network I/O notification.
    FD_ZERO(&this->rfds);
    FD_ZERO(&this->wfds);

    //把srvSocket加入到rfds，等待用户连接请求
    FD_SET(this->srvSocket, &this->rfds);

    //把当前的会话socket加入到rfds,等待用户数据的到来;加到wfds，等待socket可发送数据
    if(sessions != nullptr)
    for (list<SOCKET>::iterator itor = this->sessions->begin();itor != this->sessions->end();itor++) {
        FD_SET(*itor, &rfds);
        FD_SET(*itor, &wfds);
    }

    //然后向客户端发送数据
    this->ResponseRqst();
}

int Server::phase_1(){
    //等待用户连接请求或用户数据到来或会话socke可发送数据
    if ((this->numOfSocketSignaled = select(0, &this->rfds, &this->wfds, NULL, NULL)) == SOCKET_ERROR) { //select函数返回有可读或可写的socket的总数，保存在rtn里.最后一个参数设定等待时间，如为NULL则为阻塞模式
        sendMsg("select() failed with error!\n");
        return -1;
    }

    //当程序运行到这里，意味着有用户连接请求到来，或有用户数据到来，或有会话socket可以发送数据

    //首先检查是否有客户请求连接到来
    if (this->AcceptRequestionFromClient() != 0) return -1;

    //最后接受客户端发来的数据
    this->ReceiveMessageFromClients();
}
