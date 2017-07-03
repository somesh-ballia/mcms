/*
 * PipeToSystemMonitoring.h
 *
 *  Created on: Dec 6, 2012
 *      Author: racohen
 */

#ifndef PIPETOSYSTEMMONITORING_H_
#define PIPETOSYSTEMMONITORING_H_


#define P802_1x_STR_ETH0 "eth0"
#define P802_1x_STR_ETH1 "eth1"
#define P802_1x_STR_ETH2 "eth2"
#define P802_1x_STR_ETH3 "eth3"
#define P802_1x_STR_CONNECTED "CONNECTED"
#define P802_1x_STR_DISCONNECTED "DISCONNECTED"

#include "TaskApp.h"

class CPipeToSystemMonitoring : public CTaskApp
{
CLASS_TYPE_1(CPipeToSystemMonitoring, CTaskApp)
public:
CPipeToSystemMonitoring(void);
	virtual ~CPipeToSystemMonitoring(void);
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



#endif /* PIPETOSYSTEMMONITORING_H_ */
