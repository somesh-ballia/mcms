#if !defined(__PIPE_TO_LOGGER__)
#define __PIPE_TO_LOGGER__

#include "TaskApp.h"

class CPipeToLogger : public CTaskApp  
{
CLASS_TYPE_1(CPipeToLogger, CTaskApp)
public:
	CPipeToLogger(void);
	virtual ~CPipeToLogger(void);
	virtual const char* NameOf(void) const;
    const char* GetTaskName(void) const;

    void InitTask();
    void HandlePrivateFileDescriptor(void);
    int GetPrivateFileDescriptor();
    unsigned char IsSingleton(void) const;
    
protected:
    static std::string m_pipe_fname;

	int m_pipe_file_descriptor;
};

#endif
