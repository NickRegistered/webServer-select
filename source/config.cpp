#include "config.h"

Config::Config():root("C:\\Users\\华硕电脑\\Desktop"),MAXCONNECTION(5),\
BUFFERLENGTH(512),SERVERADDRESS("127.0.0.1"),\
PORT(80),BLOCKMODE(1)
{

}

Config::Config(const string root,const string IP,const int port):\
root(root),MAXCONNECTION(5),BUFFERLENGTH(512),\
SERVERADDRESS(IP),PORT(port),BLOCKMODE(1)
{

}

Config::~Config(){

}
