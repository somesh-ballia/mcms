#ifndef LEDLIGHTCLIENT_H_
#define LEDLIGHTCLIENT_H_

#include "commonDate.h"
#include "LedLightTask.h"

using namespace std;
//compile comand: g++ -g cppTest.cpp -o gTest
//g++  cppTest.cpp -o cTest

#define SYSCALL_ERR_STR  -2
#define SYSCALL_ERR_QUE  -3
#define SYSCALL_ERR_SEND -4
#define SYSCALL_ERR_RCV  -5

class LedLightClient: public LedLightTask
{
public:
    LedLightClient();
    virtual ~LedLightClient();

    virtual int            Run();
    int SetUp();
    void SetCommandLine();
    string GetCommandLine() { return m_cmdStr; };
    string GetProcName() { return m_procName; };
    int SendMsg();
private:
    string 			m_cmdStr;
    string 			m_procName;
    int      SysCallQueId;
    unsigned int s_sysCallmsgInd;
};

#endif

