#ifndef FILERSRVMANAGER_H_
#define FILERSRVMANAGER_H_

#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include "StatusesGeneral.h"
#include "PObject.h"
#include "OsFileIF.h"
#include "SerializeObject.h"
#include "Trace.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//                        CFileRsrvManager
////////////////////////////////////////////////////////////////////////////
template<class DataObject>
class CFileRsrvManager: public CPObject
{
	CLASS_TYPE_1(CFileRsrvManager,CPObject)

public:
	typedef std::vector<std::string> FilesArray;
	typedef typename std::vector<DataObject *> DataObjectArray;

	CFileRsrvManager(std::string dirName);
	CFileRsrvManager(const CFileRsrvManager &);

	CFileRsrvManager& operator =(const CFileRsrvManager&);

	const char* NameOf() const { return "CFileRsrvManager"; }
	std::string GetDirName() const { return m_dirName; }

	//Files Interface
	DataObject* GetFileData(const std::string fileName) const;
	int         AddFileData(DataObject& obj, const std::string fileName);
	int         DeleteFileData(std::string fileName);

	//Load the Files in to a vector of DataObjects
	int         LoadDataToVect(DataObjectArray& vect);

	template<class FilesNameFunctor>
	std::string GetFileName(FilesNameFunctor functor) const
	{
		FilesArray::const_iterator it = find_if(m_filesArray.begin(), m_filesArray.end(), functor);
		if (it == m_filesArray.end())
			return "";
		return *it;
	}

protected:
	bool        InitArrayFromDir();

	FilesArray  m_filesArray;
	std::string m_dirName;
};

////////////////////////////////////////////////////////////////////////////
//                        CFileRsrvManager
////////////////////////////////////////////////////////////////////////////
template<class DataObject>
CFileRsrvManager<DataObject>::CFileRsrvManager(std::string dirName) :
		m_dirName(dirName)
{
	InitArrayFromDir();
}

////////////////////////////////////////////////////////////////////////////
template<class DataObject>
CFileRsrvManager<DataObject>::CFileRsrvManager(const CFileRsrvManager& other) :
		CPObject(other), m_dirName(other.m_dirName), m_filesArray(other.m_filesArray)
{
}

////////////////////////////////////////////////////////////////////////////
template<class DataObject>
bool CFileRsrvManager<DataObject>::InitArrayFromDir()
{
	//load all files from the specified dir to the array
	std::vector<FDStruct> dirContentArr;
	if (!GetDirectoryContents(m_dirName.c_str(), dirContentArr))
	{
		TRACEINTOLVLERR << "DirName:" << m_dirName.c_str() << " - Problem in reading files from directory";
		return false;
	}

	std::vector<FDStruct>::iterator it;
	for (it = dirContentArr.begin(); it != dirContentArr.end(); ++it)
		if (it->type == 'F' && it->name.find(".xml") != std::string::npos)
			m_filesArray.push_back(it->name);

	return true;
}

////////////////////////////////////////////////////////////////////////////
template<class DataObject>
CFileRsrvManager<DataObject>& CFileRsrvManager<DataObject>::operator=(const CFileRsrvManager& other)
{
	m_dirName    = other.m_dirName;
	m_filesArray = other.m_filesArray;
	return *this;
}

////////////////////////////////////////////////////////////////////////////
template<class DataObject>
DataObject* CFileRsrvManager<DataObject>::GetFileData(const std::string fileName) const
{
	//check if file is in the array
	if (std::find(m_filesArray.begin(), m_filesArray.end(), fileName) == m_filesArray.end())
	{
		PASSERTSTREAM_AND_RETURN_VALUE(1, "DirName:" << m_dirName.c_str() << ", FileName:" << fileName.c_str() << " - XML file does not exist", NULL);
	}

	//read from file the new DataObject
	DataObject* pDataObject = new DataObject;

	//the file should be Meeting rooms/Profiles only
	STATUS status = pDataObject->ReadXmlFile((m_dirName + "/" + fileName).c_str(), eActiveAlarmExternal, eRemoveFile);
	if (status != STATUS_OK)
	{
		delete pDataObject;
		PASSERTSTREAM_AND_RETURN_VALUE(1, "DirName:" << m_dirName.c_str() << ", FileName:" << fileName.c_str() << " - Failed read XML file", NULL);
	}

	return pDataObject;
}

////////////////////////////////////////////////////////////////////////////
template<class DataObject>
int CFileRsrvManager<DataObject>::AddFileData(DataObject& obj, const std::string fileName)
{
	if (!CPObject::IsValidPObjectPtr(&obj))
	{
		PASSERTSTREAM_AND_RETURN_VALUE(1, "DirName:" << m_dirName.c_str() << ", FileName:" << fileName.c_str() << " - File already exists", STATUS_FAIL);
	}

	//make sure the file does not exists
	if (std::find(m_filesArray.begin(), m_filesArray.end(), fileName) != m_filesArray.end())
	{
		PASSERTSTREAM_AND_RETURN_VALUE(1, "DirName:" << m_dirName.c_str() << ", FileName:" << fileName.c_str() << " - File already exists", STATUS_FAIL);
	}

	//Create the new file
	obj.WriteXmlFile((m_dirName + "/" + fileName).c_str());

	//Add the file to the Files array
	m_filesArray.push_back(fileName);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
template<class DataObject>
int CFileRsrvManager<DataObject>::DeleteFileData(std::string fileName)
{
	//make sure the file is in the Array
	FilesArray::iterator it = std::find(m_filesArray.begin(), m_filesArray.end(), fileName);
	if (it == m_filesArray.end())
	{
		PASSERTSTREAM_AND_RETURN_VALUE(1, "DirName:" << m_dirName.c_str() << ", FileName:" << fileName.c_str() << " - File is not exists", STATUS_FAIL);
	}

	//delete the file from the directory
	if (unlink((m_dirName + "/" + fileName).c_str()) != 0)
	{
		PASSERTSTREAM_AND_RETURN_VALUE(1, "DirName:" << m_dirName.c_str() << ", FileName:" << fileName.c_str() << " - Failed delete file", STATUS_FAIL);
	}

	//remove it from the array
	m_filesArray.erase(it);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
template<class DataObject>
int CFileRsrvManager<DataObject>::LoadDataToVect(DataObjectArray & vect)
{
	int count = 0;
	//VNGR-9280, if we fail to get data from a specific file we will continue to the next and not return
	DWORD numFaultyFiles = 0;
	for (FilesArray::iterator it = m_filesArray.begin(); it != m_filesArray.end(); ++it)
	{
		DataObject* element = GetFileData(*it);
		if (!element)
		{
			numFaultyFiles++;
		}
		else
			vect.push_back(element);

		count++;
		if (count % 50 == 0)
		//We go to sleep for a little while every 100 reservations.
		//To allow other tasks to be performed in the Resource Process (vngr-9656,11705) - especially answering the watchdog
		{
			PTRACE2INT(eLevelError, "CFileRsrvManager::LoadDataToVect the number of reserved files is = ", count);
			SystemSleep(10, TRUE);
		}
	}
	if (numFaultyFiles)
	{
		PTRACE2INT(eLevelError, "CFileRsrvManager::LoadDataToVect number of faulty files that weren't uploaded = ", numFaultyFiles);
		PASSERT(numFaultyFiles);
		return STATUS_FAIL;
	}
	return STATUS_OK;
}

#endif /*FILERSRVMANAGER_H_*/
