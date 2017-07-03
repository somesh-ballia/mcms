#include "FileDownloaderTask.h"

#include "IFileDownloader.h"

#include "ManagerApi.h"

#include "Trace.h"
#include "TraceStream.h"

//////////////////////////////////////////////////////////////////////
FileDownloadRequest::FileDownloadRequest(const std::string& url, const std::string& baseFolder, const std::string& filePath, eProcessType target, OPCODE opcode)
	: url_(url)
	, baseFolder_(baseFolder)
	, filePath_(filePath)
	, target_(target)
	, opcode_(opcode)
{}

//////////////////////////////////////////////////////////////////////
void FileDownloaderTask::enqueue(const FileDownloadRequest& r)
{
	{
		ScopeGuard<Mutex> guard(lock_);

		queue_.push_back(r);

		FTRACEINTO
			<< "url:" << r.url_ << ", baseFolder:" << r.baseFolder_ << ", filePath:" << r.filePath_ << "\n"
			<< queue_.size() << " requests to complete";
	}

	requestArrived_.notify();

	run(); // will create the thread upon first request
}

//////////////////////////////////////////////////////////////////////
int FileDownloaderTask::threadRoutine()
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
void FileDownloaderTask::perform(const FileDownloadRequest& r)
{
	IFileDownloader* downloader = CFileDownloadersFactory::const_instance().lookup(r.url_);
	FPASSERTSTREAM_AND_RETURN(!downloader, "Do not know how to download from " << r.url_;);

    std::string path = r.baseFolder_;
	path += '/' + r.filePath_;
	FTRACEINTO << "url:" << r.url_ << ", filePath:" << path;
	DownloadFileResult downloadResult = downloader->download(r.url_, path);

	CFileDownloadersFactory::const_instance().destroy(downloader);

	CSegment* pSeg = new CSegment;
	*pSeg << r.url_ << static_cast<int>(downloadResult) << r.baseFolder_;

	CManagerApi api(r.target_);
	api.SendMsg(pSeg, r.opcode_);
}

//////////////////////////////////////////////////////////////////////
FileDownloaderTask::~FileDownloaderTask()
{
	FTRACEINTO << "Shutting down, unhandled requests:" << queue_.size();
}

//////////////////////////////////////////////////////////////////////
