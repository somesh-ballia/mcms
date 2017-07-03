#include <stdlib.h>
#include "PcmConfirms.h"
#include "ObjString.h"

////////////////////////////////////////////////////////////////////////
CPcmPopMenuStatusConfirm::CPcmPopMenuStatusConfirm(int termId):CPcmConfirm(termId)
{
	action_name = "pop_menu_status";
	result = 0;
}
////////////////////////////////////////////////////////////////////////
CPcmPopMenuStatusConfirm::CPcmPopMenuStatusConfirm(CPcmPopMenuStatusConfirm& other):CPcmConfirm(other)
{
	result = other.result;
}
////////////////////////////////////////////////////////////////////////
CPcmPopMenuStatusConfirm::~CPcmPopMenuStatusConfirm()
{
}
////////////////////////////////////////////////////////////////////////
void CPcmPopMenuStatusConfirm::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	mpMsg["RESULT"]=itoa(result);
	
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
void CPcmPopMenuStatusConfirm::DeSerializeXml(strmap& mpMsg) // build the object from strmap
{
	CPcmMessage::DeSerializeXml(mpMsg);
	result = atoi(mpMsg["RESULT"].c_str());
}
////////////////////////////////////////////////////////////////////////
void CPcmPopMenuStatusConfirm::Dump() // build the object from strmap
{
	CLargeString cstr;
	cstr << "CPcmPopMenuStatusConfirm::Dump\n";
	cstr << "SOURCE_GUID:" << source_guid << "\n";
	cstr << "TARGET_GUID:" << target_guid << "\n";
	cstr << "TERM_ID:" << term_id << "\n";
	cstr << "_xml_msg_name:" << MsgTypeToString(type) << "\n";
	cstr << "_xml_msg_id:" << action_name.c_str() << "\n";
	cstr << "RESULT:" << result;
	
	PTRACE(eLevelInfoNormal,cstr.GetString());
}
