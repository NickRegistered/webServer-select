#ifndef WINSOCKENV_H
#define WINSOCKENV_H
class WinsockEnv
{
private:
    WinsockEnv(void);
    ~WinsockEnv(void);
public:
    static int Startup();
};


#endif // WINSOCKENV_H
