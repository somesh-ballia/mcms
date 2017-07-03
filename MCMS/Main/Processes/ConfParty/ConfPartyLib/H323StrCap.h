// CH323StrCap.h: interface of the CH323StrCap class.
//
//////////////////////////////////////////////////////////////////////
#if !defined(_H323strCap_H__)
#define _H323strCap_H__


#include "H221Str.h"
#include "Capabilities.h"

class CSegment;

class CH323StrCap : public  CH221Str
{
CLASS_TYPE_1(CH323StrCap ,CH221Str)
public:
	CH323StrCap(const CH221Str &other):CH221Str(other){}
	CH323StrCap() {}
	~CH323StrCap() {}
    virtual void   Dump(std::ostream &m_ostr); //Capabilitys Dump
	virtual void   Serialize(WORD format, std::ostream &m_ostr); 
	virtual void   DeSerialize(WORD format, std::istream &m_istr);
	virtual void   Serialize(WORD format, CSegment *pSeg); 
	virtual void   DeSerialize(WORD format, CSegment *pSeg);
	int  ConvertAnnexOld(char *pStruct,WORD *tempArr,int &IndexArr,int &newSize,BYTE **ppAnnexPtr);
	int  ConvertAnnex(char *pStruct,WORD *tempArr,int &IndexArr,int &newSize, BYTE **ppAnnexPtr);
	int  ConverCustomFormatOld(char *pStruct,WORD *tempArr,int &IndexArr,int &newSize);
	int  ConverCustomFormat(char *pStruct,WORD *tempArr,int &IndexArr,int &newSize);
	int  ConvertCapBuffAndHeader(char *pBuffer,WORD *tempArr,int &IndexArr,int size);
	int  ConvertCapBuffAndHeaderOld(char *pBuffer,WORD *tempArr,int &IndexArr,int size);
	int  CopyStructToOldformArray(int startPoint, int toatlSize,WORD *pToArray,BYTE *pFromArray,int &IndexArr);
	
	int GetFullSizeOf263Struct(char *pBuffer);
	int GetFullSizeOf263StructNew(char *pBuffer);
	int GetSizeOfAnnexes(char *pStruct, BYTE **ppAnnexPtr);
	int GetSizeOfCustoms(char *pStruct);
	void HandleAnnexesMask(long *pAnnexMask);
	int CopyStruct(int startPoint, int toatlSize,BYTE *pToArray,BYTE *pFromArray,int &IndexArr);

};


class CH323strCom : public  CH221Str
{
CLASS_TYPE_1(CH323strCom ,CH221Str)
public:
    virtual void   Dump(std::ostream& ostr); //CommMode Dump
};
#endif // !defined(_H323strCap_H__)
