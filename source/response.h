#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdio.h>
#include <string.h>
#include <map>
using namespace std;
struct Response {
    char Method[8];	//请求方法
    char URL[32];	//请求路径
    char Extn[8];	//请求文件扩展名

    char Stat[32];		//状态行
    char CntType[32];	//Content-Type;
    char CntLen[32];	//Content-Length

    Response(char* requestion);			//抓取请求信息中的信息
    void ResponseHeader(const char *filename,char* buff);//根据文件信息组织响应报文，写入buff中
};
#endif // RESPONSE_H
