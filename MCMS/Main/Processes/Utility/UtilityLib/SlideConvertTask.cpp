#include "SlideConvertTask.h"

#include "ManagerApi.h"

#include "Trace.h"
#include "TraceStream.h"

#include <sstream>
#include <sys/types.h>
//////////////////////////////////////////////////////////////////////
SlideConvertRequest::SlideConvertRequest(const std:: string & url, const std::string& outSlidesPath, const std::string& srcImageFile
                                                , bool isOnlyForH264, int nConversionMethod, int nImageType
                                                , eProcessType target, OPCODE opcode)
    : url_(url)
    , outSlidesPath_(outSlidesPath)
	, srcImageFile_(srcImageFile)
	, isOnlyForH264_(isOnlyForH264)
	, nConversionMethod_(nConversionMethod)
	, nImageType_(nImageType)
	, target_(target)
	, opcode_(opcode)
{}

//////////////////////////////////////////////////////////////////////
void CSlideConvertTask::enqueue(const SlideConvertRequest& r)
{
	{
		ScopeGuard<Mutex> guard(lock_);

		queue_.push_back(r);

		FTRACEINTO
			<< "srcImageFile:" << r.srcImageFile_ << ", outSlidesPath:" << r.outSlidesPath_
			<< ", isOnlyForH264:" << r.isOnlyForH264_<< ", nConversionMethod:" << r.nConversionMethod_ << "\n"
			<< queue_.size() << " requests to complete";
	}

	requestArrived_.notify();

	run(); // will create the thread upon first request
}

//////////////////////////////////////////////////////////////////////
int CSlideConvertTask::threadRoutine()
{
	for ( ; ; )
	{
		Requests::const_iterator it;

		{
			ScopeGuard<Mutex> guard(lock_);

			// unlocks the lock and waits on the condition
			if (queue_.empty() && s_Running == state())
				requestArrived_.wait(lock_);
			// locks the lock back upon condition is met

			it = queue_.begin();

			if (s_Running != state() || it == queue_.end()) // should occur only at the process shut-down
				break;
		}

		perform(*it);

		{
			ScopeGuard<Mutex> guard(lock_);

			queue_.pop_front();

			FTRACEINTO << queue_.size() << " requests to complete";
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
void CSlideConvertTask::perform(const SlideConvertRequest& r)
{
    std::stringstream sstr;
    sstr    << "Bin/ConvertImage2IVRSlides "
            << r.outSlidesPath_ << " "
            << r.srcImageFile_ << " "
            << r.isOnlyForH264_ << " "
            << r.nConversionMethod_ << " "
            << "&> /dev/null";

    int res = system(sstr.str().c_str());
    if (0 != res)
    {
        res = WEXITSTATUS(res);
    }
    CSegment * pSeg = new CSegment;
    *pSeg << r.url_ << res;

    if (0 != res)
    {
        // If convert failed, return the output path
        *pSeg << r.outSlidesPath_;
    }
    else
    {
        *pSeg << r.nImageType_ << r.srcImageFile_;
    }

    FTRACEINTO << "Convert slide result: url is:" << r.url_ << ", convert status is:" << res;

    CManagerApi api(r.target_);
    api.SendMsg(pSeg, r.opcode_);
}

//////////////////////////////////////////////////////////////////////
CSlideConvertTask::~CSlideConvertTask()
{
	FTRACEINTO << "Shutting down, unhandled requests:" << queue_.size();
}

//////////////////////////////////////////////////////////////////////

