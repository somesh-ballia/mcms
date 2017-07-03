//+========================================================================+
//                       PcmMessage.h                                      |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PcmMessage.h                                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Eitan                                                            |
//-------------------------------------------------------------------------|
// Who  | Date  August - 2007  | Description                                   |
//-------------------------------------------------------------------------|
//			
//+========================================================================++

#ifndef _PCMMESSAGE_H_
#define _PCMMESSAGE_H_

#include "PObject.h"
#include "Segment.h"
#include "StatusesGeneral.h"
#include <string>
#include "DstCommonFunctions.h"
#include "VideoDefines.h"
#include "ObjString.h"

//CDstCommonFunctions

using namespace std;
# define NUM_OF_PCM_TYPES 3
# define NUM_OF_MCMS_LAYOUT_TYPES CP_NO_LAYOUT//this is the last available layout (changed from cop version - added support in RPX layouts)

typedef enum {
	eInvalid	= -1,
	eIndication = 0, 
	eCommand,
	eConfirm
}EPcmMsgType;

static const char *PcmMsgTypeString[] =
{
    "INDICATION",		// eIndication
    "COMMAND",			// eCommand
    "CONFIRM",			// eConfirm
};

typedef struct
{
	LayoutType mcmsLayoutType;
	int		   pcmApiLayoutType;
	int  	   paneIndexes[MAX_SUB_IMAGES_IN_LAYOUT];
} TMcmsPcmLayoutType;

class CPcmMessage : public CPObject {
CLASS_TYPE_1(CPcmMessage,CPObject)
public:
	CPcmMessage(int termId = 0);
	virtual ~CPcmMessage();
	CPcmMessage(CPcmMessage& other);
	virtual const char*  NameOf() const = 0;
	
	string itoa(int n){return g_CommonFun.itoa(n);}
	
	bool 	GetNodeValueBool(strmap& mpMsg, string/*char**/ str);
	int  	GetNodeValueInteger(strmap& mpMsg, string/*char**/ str);
	string  GetNodeValueString(strmap& mpMsg, string/*char**/ str);
	
	void	AddChildNodeBool(strmap& mpMsg, string strKey, bool val);
	void	AddChildNodeInteger(strmap& mpMsg, string strKey, int val);
	void	AddChildNodeString(strmap& mpMsg, string strKey, string val);
	void    AddChildNodeByteToBoolStr(strmap& mpMsg, string strKey, BYTE val);
	
	LayoutType  TranslatePcmApiLayoutTypeToMcmsLayoutType(int pcmApiLayoutType);
	int			TranslateMcmsLayoutTypeToPcmApiLayoutType(LayoutType mcmsLayoutType);
	int			TranslatePcmPaneIndexToMcmsSubImageId(LayoutType mcmsLayoutType, int paneIndex);
	
	bool IsValidPcmMessage(CSmallString& details);
	bool IsFlexLayout(LayoutType mcmsLayoutType);
	
	void SetTerminalId(int terminalId){term_id = terminalId;}
	int GetTerminalId(){return term_id;}

	const char*  MsgTypeToString(EPcmMsgType type);
	EPcmMsgType StringToMsgType(string& str);
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
	

protected:
	 	
	static CDstCommonFunctions g_CommonFun;
	static const TMcmsPcmLayoutType g_LayoutTypesTbl[NUM_OF_MCMS_LAYOUT_TYPES];
	
	int	source_guid;
	int	target_guid;
	int term_id;
	EPcmMsgType type;
	string action_name;
	
	string rootName; 
	string rootVer;

};

class CPcmIndication : public CPcmMessage {
CLASS_TYPE_1(CPcmIndication,CPcmMessage)

public:
	CPcmIndication(int termId = 0);
	virtual ~CPcmIndication();
	CPcmIndication(CPcmIndication& other);
	virtual const char*  NameOf() const;
			
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str) = 0; // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg) = 0;				    // build the object from strmap
	
};


class CPcmCommand : public CPcmMessage {
CLASS_TYPE_1(CPcmCommand,CPcmMessage)

public:
	CPcmCommand(int termId = 0);
	virtual ~CPcmCommand();
	CPcmCommand(CPcmCommand& other);
	virtual const char*  NameOf() const;
	
	virtual CPcmCommand* Clone() = 0; 
		
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str) = 0; // serialize object to an xml string
	virtual void DeSerializeXml(strmap& mpMsg) = 0;				    // build the object from strmap
	
};

class CPcmConfirm : public CPcmMessage {
CLASS_TYPE_1(CPcmConfirm,CPcmMessage)

public:
	CPcmConfirm(int termId = 0,string action="",int result=STATUS_OK);
	virtual ~CPcmConfirm();
	CPcmConfirm(CPcmConfirm& other);
	CPcmConfirm(CPcmCommand& other);
	virtual const char*  NameOf() const;
	
	virtual void SerializeXmlToStr(strmap& mpMsg, string& str); // serialize object to an xml string
	//virtual void DeSerializeXml(strmap& mpMsg);				    // build the object from strmap
	void SetResult(int result){m_result=result;}
	void SetActionName(string action){action_name=action;}
	
protected:
	int m_result;
};


#endif //_PCMMESSAGE_H_
