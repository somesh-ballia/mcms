#ifndef LEDLIGHTTASK_H_
#define LEDLIGHTTASK_H_

#include "commonDate.h"

#define SYS_CALL_KEY 400

#define FIFO_SERVER "/tmp/lightalarm/fifoserver"
//compile comand: g++ -g cppTest.cpp -o gTest
//g++  cppTest.cpp -o cTest
using namespace std;

class LedLightTask
{
public:
                                LedLightTask();
    virtual                     ~LedLightTask();
    virtual int                 Run();
    void                         SetArgv(char** const argv) { m_Argv = argv; }
    void                         SetArgc(int argc) { m_Argc = argc; }
    char * const *               GetArgv() const { return m_Argv; }
    int                          GetArgc() const { return m_Argc; }
    int                          GetSysCallMsgQue(void);
    int                          CreateSysCallMsgQue(void);
	int 						 Stop();
//members
private:
    char** m_Argv;
    int    m_Argc;
};

#endif
