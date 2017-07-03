// FileManager.h

#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_

#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>

#include "PObject.h"
#include "OsFileIF.h"
#include "TraceStream.h"
#include "SerializeObject.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"

template <class DataObject>
class CFileManager :  public CPObject
{
	CLASS_TYPE_1(CFileManager,CPObject)
public:
	typedef std::vector< std::string> FilesArray;
	typedef typename std::vector< DataObject * > DataObjectArray;

	virtual const char* NameOf() const { return "CFileManager";}
	CFileManager(std::string dirName);
	CFileManager(const CFileManager &);

	CFileManager & operator =(const CFileManager &);

	std::string GetDirName()const {return m_dirName;}

	//Files Interface
	DataObject * GetFileData(const std::string fileName)const;
	int AddFileData(DataObject & obj,const std::string fileName);
	int ReplaceFileData(DataObject* pObj,const std::string fileName);
	int DeleteFileData(std::string fileName);
	int RemoveFile(std::string& sFileName);
	void RemoveAllFiles();

	//Load the Files in to a vector of DataObjects
	int LoadDataToVect(DataObjectArray & vect);
	int GetMaxFileNumAccordingToPrefix(std::string& prefix);

	void DeleteVectData();

	template <class FilesNameFunctor>
	std::string GetFileName(FilesNameFunctor functor)const{
		FilesArray::const_iterator it;
		it = find_if(m_filesArray.begin(),m_filesArray.end(),functor);
		if(it == m_filesArray.end())
			return "";
		return *it;
	}

protected:
	//Iternal methods
	bool InitArrayFromDir();
	FilesArray m_filesArray;
	std::string m_dirName;
};


//----------------------------------- IMPLEMENTATION --------------------------

template <class DataObject>
CFileManager<DataObject>::CFileManager(std::string dirName)
:m_dirName(dirName)
{
	InitArrayFromDir();
}

template <class DataObject>
CFileManager<DataObject>::CFileManager(const CFileManager & other)
:CPObject(other),m_filesArray(other.m_filesArray),m_dirName(other.m_dirName)
{

}

template <class DataObject>
bool CFileManager<DataObject>::InitArrayFromDir()
{
	// Loads all files from the specified dir to the array
	std::vector<FDStruct> dirContentArr;
	if (!GetDirectoryContents(m_dirName.c_str(),dirContentArr))
	{
		PTRACE(eLevelError,"CFileManager::InitArrayFromDir problem in reading files from directory");
		return false;
	}

	std::vector<FDStruct>::const_iterator it;
	for (it = dirContentArr.begin(); it != dirContentArr.end(); ++it)
		if (it->type == 'F' && it->name.find(".xml") != std::string::npos)
			m_filesArray.push_back(it->name);

	return true;
}

template <class DataObject>
CFileManager<DataObject> &  CFileManager<DataObject>::operator =(const CFileManager & other)
{
	m_dirName=other.m_dirName;
	m_filesArray = other.m_filesArray;
	return *this;
}

template <class DataObject>
DataObject* CFileManager<DataObject>::GetFileData(const std::string fileName) const
{
	// Checks if file is in the array
	if (std::find(m_filesArray.begin(), m_filesArray.end(), fileName) == m_filesArray.end())
	{
	  TRACEWARN << "File " << fileName << " doesn't exist in the DB";
		return NULL;
	}
	//read from file the new DataObject
	DataObject * pDataObject = new DataObject;

	//the file should be Meeting rooms/Profiles only
	STATUS status = pDataObject->ReadXmlFile(	(m_dirName+ "/" +fileName).c_str(),
			                                     eActiveAlarmExternal,
												 eRemoveFile);
	if (status != STATUS_OK)
	{
		PTRACE(eLevelError,"CFileManager::GetFileData Read Xml file fails");
		PASSERT(1);
		POBJDELETE(pDataObject);
		return NULL;
	}

	return pDataObject;
}

template <class DataObject>
int CFileManager<DataObject>::AddFileData(DataObject & obj,const std::string fileName)
{
	if (!CPObject::IsValidPObjectPtr(&obj) )
		{
			PTRACE(eLevelError,"CFileManager::AddFileData : invalid file object");
			PASSERT(1);
			return STATUS_FAIL;
		}

	//make sure the file does not exists
	if ( std::find(m_filesArray.begin(),m_filesArray.end(),fileName) != m_filesArray.end() ){
		std:string msg = "CFileManager::AddFileData : file already exists ";
		msg += fileName;
		PTRACE(eLevelError,msg.c_str());
		//PASSERT(1);
		return STATUS_FILE_ALREADY_EXISTS;

	}

	//Create the new file
	obj.WriteXmlFile((m_dirName+ "/" +fileName).c_str());

    //Add the file to the Files array
    m_filesArray.push_back(fileName);

	return STATUS_OK;
}

template <class DataObject>
int CFileManager<DataObject>::ReplaceFileData(DataObject* pObj, const std::string fileName)
{
	if (!CPObject::IsValidPObjectPtr(pObj) )
	{
		PTRACE(eLevelError,"CFileManager::ReplaceFileData - Invalid Data Object");
		PASSERT(1);
		return STATUS_FAIL;
	}

	//make sure the file does not exists
	if ( std::find(m_filesArray.begin(), m_filesArray.end(), fileName) != m_filesArray.end() )
	{
		//make sure the file is in the Array
		FilesArray::iterator it = std::find(m_filesArray.begin(),m_filesArray.end(),fileName);
		if (  it == m_filesArray.end() )
		{
			PTRACE(eLevelError,"CFileManager::DeleteFileData file is not exists");
			PASSERT(1);
			return STATUS_FAIL;
		}

		//remove it from the array
		m_filesArray.erase(it);

	}

	//Create the new file
	pObj->WriteXmlFile((m_dirName+ "/" +fileName).c_str());

    //Add the file to the Files array
    m_filesArray.push_back(fileName);

	return STATUS_OK;
}


template <class DataObject>
int CFileManager<DataObject>::DeleteFileData(std::string fileName)
{
  //make sure the file is in the Array
  FilesArray::iterator it = std::find(m_filesArray.begin(), m_filesArray.end(), fileName);
  if (it == m_filesArray.end())
  {
    PTRACE2(eLevelError, "CFileManager::DeleteFileData - Failed, file is not exists, FileName:", fileName.c_str());
    PASSERT(1);
    return STATUS_FAIL;
  }

  //delete the file from the directory
  if (unlink((m_dirName + "/" + fileName).c_str()) != 0)
  {
    PTRACE2(eLevelError, "CFileManager::DeleteFileData - Failed, cannot unlink file, FileName:", fileName.c_str());
    PASSERT(2);

    //remove it from the array
    m_filesArray.erase(it);
    return STATUS_FAIL;
  }

  //remove it from the array
  m_filesArray.erase(it);
  return STATUS_OK;
}
template <class DataObject>
int CFileManager<DataObject>::RemoveFile(std::string& sFileName)
{
	//make sure the file is in the Array
	FilesArray::iterator it = std::find(m_filesArray.begin(), m_filesArray.end(), sFileName);
	if (it == m_filesArray.end())
		return STATUS_OK;

	//delete the file from the directory
	if (unlink((m_dirName + "/" + sFileName).c_str()) != 0)
	{
		PTRACE(eLevelError,"CFileManager::RemoveFile unlink fails");
		PASSERT(1);
		return STATUS_FAIL;
	}

	//remove it from the array
	m_filesArray.erase(it);
	return STATUS_OK;
}

template <class DataObject>
void CFileManager<DataObject>::RemoveAllFiles()
{
	FilesArray::iterator it = m_filesArray.begin();
	for ( ; it != m_filesArray.end(); ++it)
	{
		 unlink((m_dirName+"/" + *it).c_str());
	}

	m_filesArray.clear();
}


template <class DataObject>
int CFileManager<DataObject>::LoadDataToVect(DataObjectArray & vect)
{
	DataObject * element=0;
	//VNGR-9280, if we fail to get data from a specific file we will continue to the next and not return
	DWORD numFaultyFiles = 0;
	for ( FilesArray::iterator it=m_filesArray.begin() ; it != m_filesArray.end() ;++it)
	{
		element = GetFileData(*it);
		if (!element)
		{
			numFaultyFiles++;
		}
		else
			vect.push_back(element);
	}
	if(numFaultyFiles)
	{
		PTRACE2INT(eLevelError,"CFileManager::LoadDataToVect number of faulty files that weren't uploaded = ",numFaultyFiles);
		PASSERT(numFaultyFiles);
		return STATUS_FAIL;
	}
	return STATUS_OK;
}


template <class DataObject>
void CFileManager<DataObject>::DeleteVectData()
{
	for (FilesArray::iterator it = m_filesArray.begin() ; it != m_filesArray.end() ; ++it)
	{
		std::string fileName = *it;
		if ( unlink((m_dirName+"/"+fileName).c_str()) != 0 )
		{
			PTRACE(eLevelError,"CFileManager::DeleteVectData unlink fails");
			PASSERT(1);
		}
	}

	m_filesArray.clear();
}

template <class DataObject>
int CFileManager<DataObject>::GetMaxFileNumAccordingToPrefix(std::string& prefix)
{
	int max=-1;
	int tmpNumber;
	const char* strToFind = NULL;

	for (FilesArray::iterator it = m_filesArray.begin() ; it != m_filesArray.end() ; ++it)
	{
		if (strstr((*it).c_str(),prefix.c_str()))
		{
			strToFind = strstr((*it).c_str(),".xml");
			if (strToFind == NULL)
			{
				TRACEINTO << "Sub string: '.xml' isn't found in string: " << (*it).c_str();
				PASSERTMSG_AND_RETURN_VALUE(1, "CFileManager::GetMaxFileNumber fails",-1);
			}

			char number[6];
			strncpy(number,strToFind-5,5);
			number[5] = '\0';

			if (strcmp(number,"00000") == 0)
			{
				tmpNumber = 0;
			}
			else
			{
				tmpNumber = atoi(number);
				if (tmpNumber == 0)
				{
					TRACEINTO << "Number is invalid: " << number;
					PASSERTMSG_AND_RETURN_VALUE(1, "CFileManager::GetMaxFileNumber fails invalid file name",-1);
				}
			}

			if (tmpNumber > max)
				max =tmpNumber;
		}
	}

	return max;
}
#endif
