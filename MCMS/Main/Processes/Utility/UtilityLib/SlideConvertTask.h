#ifndef SLIDE_CONVERT_TASK_H__
#define SLIDE_CONVERT_TASK_H__

//////////////////////////////////////////////////////////////////////
#include "WorkerThread.h"

#include "McmsProcesses.h"

#include "Mutex.h"

//////////////////////////////////////////////////////////////////////
#include <string>

#include <list>
//////////////////////////////////////////////////////////////////////
struct SlideConvertRequest
{
	SlideConvertRequest(const std:: string & url, const std::string& outSlidesPath, const std::string& srcImageFile
                            , bool isOnlyForH264, int nConversionMethod/* 0-low, 1-high, 2-low&high*/, int nImageType
                            , eProcessType target, OPCODE opcode);

    const std::string   url_;
    const std::string   outSlidesPath_;
	const std::string   srcImageFile_;
	const bool          isOnlyForH264_;
    const int           nConversionMethod_;
    const int           nImageType_;

	const eProcessType target_;
	const OPCODE       opcode_;
};

//////////////////////////////////////////////////////////////////////
class CSlideConvertTask : public WorkerThread
{
public:

	virtual ~CSlideConvertTask();

public:

	// public API to enqueue slide convert request
	// runs in the context of the *calling* thread
	void enqueue(const SlideConvertRequest& request);

private:

	void perform(const SlideConvertRequest& r);

private: // overridden

	// the actual thread routine to handle the requests
	// runs in the context of a new thread
	virtual int threadRoutine();

private:

	typedef std::list<SlideConvertRequest> Requests;

	Requests queue_;

	// mutually exclusive access to the queue of requests
	Mutex lock_;

	// indication on "request arrived" event
	ConditionVariable requestArrived_;
};

//////////////////////////////////////////////////////////////////////
#endif // SLIDE_CONVERT_TASK_H__
