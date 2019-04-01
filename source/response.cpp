#include "response.h"
using namespace std;

const map<string, string> ExtnToType = { { ".gif","image/gif\r\n" },{ ".mp3","audio/mp3\r\n" } ,
                                        { ".ogg","application/ogg\r\n" },{ ".mp4","audio/mp4\r\n" } ,
                                        { ".webm","video/webm\r\n" },{ ".html","text/html\r\n" },
                                         {".ico","application/x-ico\r\n"},{ "","text/html\r\n" } };

const char* page404 = "<html><body><h1 style=\"text-align:center\">404 NotFound</h1></body></html>";
Response::Response(char* req) {
    int i = 0;
    for (;req[i] != ' ';++i) {
        Method[i] = req[i];
    }Method[i++] = '\0';//添加结束符并跳过空格

    int j = 0, k = 0;
    int flag = 0;
    for (;req[i] != ' ';++i, ++j) {
        URL[j] = req[i];
        if (URL[j] == '/') URL[j] = '\\';//将请求中的'/'换作'\\'

        Extn[k] = req[i];
        if (req[i] == '.')flag = 1;
        k += flag;
    }URL[j] = '\0', Extn[k] = '\0';

    //如果请求根目录
    if (strcmp(URL, "\\") == 0) {//默认发送html
        strcat(URL, "index.html");
    };

}

void Response::ResponseHeader(const char *filename,char* buff) {
    buff[0] = '\0';
    FILE *fin = fopen(filename, "rb");
    if (!fin) {
        Stat[0] = '\0';
        strcat(Stat, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 73\r\n\r\n");//文件打开失败，写入状态信息404
        strcat(buff, Stat);
        strcat(buff,page404);//直接写入404页面
        return;
    }
    else {
        Stat[0] = '\0';
        strcat(Stat,"HTTP/1.1 200 OK\r\n");

        auto itor = ExtnToType.find(Extn);
        CntType[0] = '\0';
        strcat(CntType, "Content-Type: ");
        strcat(CntType, (itor->second).c_str());

        fseek(fin, 0, SEEK_END);
        int size = ftell(fin);
        fclose(fin);

        char num[7],tempc;
        int i;
        for (i = 0;size > 0;++i) {
            num[i] = size % 10 + '0';
            size /= 10;
        }num[i--] = '\0';//添加结束符，将i指向最后一个数字

        for (int j = 0;j < i;++j, --i) {
            tempc = num[j];
            num[j] = num[i];
            num[i] = tempc;
        }

        CntLen[0] = '\0';
        strcat(CntLen, "Content-Length: ");
        strcat(CntLen, num);
        strcat(CntLen,"\r\n\r\n");//结束一行，结束报文

        strcat(buff, Stat);
        strcat(buff, CntType);
        strcat(buff, CntLen);
        return;
    }
}
