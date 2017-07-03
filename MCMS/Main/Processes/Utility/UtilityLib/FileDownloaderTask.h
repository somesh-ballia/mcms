#ifndef FILE_DOWNLOADER_TASK_H__
#define FILE_DOWNLOADER_TASK_H__

//////////////////////////////////////////////////////////////////////
#include "WorkerThread.h"

#include "McmsProcesses.h"

#include "Mutex.h"

//////////////////////////////////////////////////////////////////////
#include <string>

#include <list>

//////////////////////////////////////////////////////////////////////
struct FileDownloadRequest
{
	FileDownloadRequest(const std::string& url, const std::string& baseFolder, const std::string& filePath, eProcessType target, OPCODE opcode);

	const std::string url_;
	const std::string baseFolder_;
	const std::string filePath_;

	const eProcessType target_;
	const OPCODE       opcode_;
};

//////////////////////////////////////////////////////////////////////
class FileDownloaderTask : public WorkerThread
{
public:

	virtual ~FileDownloaderTask();

public:

	// public API to enqueue file download request
	// runs in the context of the *calling* thread
	void enqueue(const FileDownloadRequest& request);

private:

	void perform(const FileDownloadRequest& r);

private: // overridden

	// the actual thread routine to handle the requests
	// runs in the context of a new thread
	virtual int threadRoutine();

private:

	typedef std::list<FileDownloadRequest> Requests;

	Requests queue_;

	// mutually exclusive access to the queue of requests
	Mutex lock_;

	// indication on "request arrived" event
	ConditionVariable requestArrived_;
};

//////////////////////////////////////////////////////////////////////
#endif // FILE_DOWNLOADER_TASK_H__
