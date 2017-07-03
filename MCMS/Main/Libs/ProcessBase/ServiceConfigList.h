
#ifndef SERVICECONFIGLIST_H_
#define SERVICECONFIGLIST_H_

#include <string>
#include <iostream>
#include "ServiceConfig.h"




class CServiceConfigList: public CPObject
{
CLASS_TYPE_1(CServiceConfigList, CPObject)

public:
	CServiceConfigList();
	CServiceConfigList(const CServiceConfigList&);
	~CServiceConfigList();
	virtual const char* NameOf() const { return "CServiceConfigList";}
	CServiceConfigList& operator=( const CServiceConfigList& other);
	void DeSerialize(CSegment *pSeg);
	void Serialize(WORD format, CSegment *pSeg);
	//void FillMap(CServiceConfig *pServiceConfig);
	int Add(CServiceConfig *pServiceConfig);
	//int FindService( const CIPService &other )const;
	BOOL GetStrDataByKey(DWORD service_id, const std::string &key, std::string &data)const;
	BOOL GetDWORDDataByKey(DWORD service_id, const std::string &key, DWORD &data)const;
	BOOL GetHexDataByKey(DWORD service_id, const std::string &key, DWORD &data)const;
	BOOL GetBOOLDataByKey(DWORD service_id, const std::string &key, BOOL &data)const;
	BOOL IsServiceExists(DWORD service_id)const;
	void PrintServiceConfigList();

protected:
	CServiceConfig * m_pServiceConfig[MAX_SERV_PROVIDERS_IN_LIST];

private:
	WORD  m_numb_of_serv;

private:






};


#endif /* SERVICECONFIGLIST_H_ */
