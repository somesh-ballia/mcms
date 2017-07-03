#include <stdlib.h>
#include "PcmMessage.h"
#include "ObjString.h"

WORD GetNumbSubImg(const LayoutType layoutType);

CDstCommonFunctions CPcmMessage::g_CommonFun;

const TMcmsPcmLayoutType CPcmMessage::g_LayoutTypesTbl[NUM_OF_MCMS_LAYOUT_TYPES] =
{
		{CP_LAYOUT_1X1 ,		101, 	{0,}										},
		{CP_LAYOUT_1X2 ,		201,	{0,1,}										},
		{CP_LAYOUT_2X1 ,		202, 	{0,1,}										},
		{CP_LAYOUT_1X2_FLEX ,	203, 	{0,1,}										}, //??
		{CP_LAYOUT_1x2HOR,		203, 	{0,1,2,}									},
		{CP_LAYOUT_1x2VER,		204, 	{0,1,2,}									},
		{CP_LAYOUT_1P2HOR_UP,	301, 	{0,1,2,}									},
		{CP_LAYOUT_1P2HOR ,		302, 	{0,1,2,}									},
		{CP_LAYOUT_1P2VER,		303, 	{0,1,2,}									},
		{CP_LAYOUT_1P2HOR_RIGHT_FLEX,		406,	{0,1,2,}						},
		{CP_LAYOUT_1P2HOR_LEFT_FLEX,		407, 	{0,1,2,}						},
		{CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX,	408, 	{0,1,2,}						},
		{CP_LAYOUT_1P2HOR_UP_LEFT_FLEX,		409, 	{0,1,2,}						},
		{CP_LAYOUT_2X2,			401, 	{0,1,2,3,}									},
		{CP_LAYOUT_1P3HOR,		402, 	{0,1,2,3,}									},
		{CP_LAYOUT_1P3VER,		403, 	{0,1,2,3,}									},
		{CP_LAYOUT_1P3HOR_UP,	404, 	{0,1,2,3,}									},
		{CP_LAYOUT_2X2_UP_RIGHT_FLEX,		504, 	{0,1,2,3,}									},
		{CP_LAYOUT_2X2_UP_LEFT_FLEX,		505, 	{0,1,2,3,}									},
		{CP_LAYOUT_2X2_DOWN_RIGHT_FLEX,		506, 	{0,1,2,3,}									},
		{CP_LAYOUT_2X2_DOWN_LEFT_FLEX,		507, 	{0,1,2,3,}									},
		{CP_LAYOUT_2X2_RIGHT_FLEX,			508, 	{0,1,2,3,}									},
		{CP_LAYOUT_2X2_LEFT_FLEX,			509, 	{0,1,2,3,}									},
		{CP_LAYOUT_1P4VER,		501, 	{0,1,2,3,4,}								},
		{CP_LAYOUT_1P4HOR,		502, 	{0,1,2,3,4,}								},
		{CP_LAYOUT_1P4HOR_UP,	503,	{0,1,2,3,4,}								},
		{CP_LAYOUT_1P5 ,		601,	{0,1,2,3,4,5,}								},
		{CP_LAYOUT_1P7,			801, 	{0,1,2,3,7,6,5,4,}							},
		{CP_LAYOUT_3X3,			901, 	{0,1,2,3,4,5,6,7,8,}						},
		{CP_LAYOUT_1P8CENT,		902, 	{0,1,2,3,4,8,7,6,5,}						},			
		{CP_LAYOUT_1P8UP,		903, 	{0,1,2,3,4,8,7,6,5,}						},
		{CP_LAYOUT_1P8HOR_UP,	904, 	{0,1,2,3,4,8,7,6,5,}						},
		{CP_LAYOUT_2P8,			1001, 	{0,1,2,3,4,5,6,7,8,9,}						},
		{CP_LAYOUT_1P12,		1301, 	{0,1,2,3,4,5,6,7,8,9,10,11,12,}				},
		{CP_LAYOUT_4X4,			1601,   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}		}
	
};

//ctor
CPcmMessage::CPcmMessage(int termId):source_guid(0),target_guid(0),term_id(termId),type(eInvalid),rootName("TVUI_API"),rootVer("1.0.0.0")
{
	
}	
////////////////////////////////////////////////////////////////////////
CPcmMessage::CPcmMessage(CPcmMessage& other):CPObject(other)
{
	rootName = other.rootName;
	rootVer = other.rootVer;
	source_guid = other.source_guid;
	target_guid = other.target_guid;
	term_id = other.term_id;
	type = other.type;
	action_name = other.action_name;
	
}
////////////////////////////////////////////////////////////////////////
CPcmMessage::~CPcmMessage()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmMessage::NameOf() const
{
	return "CPcmMessage";
}
////////////////////////////////////////////////////////////////////////
const char* CPcmMessage::MsgTypeToString(EPcmMsgType type)
{
	const char *name = (0 <= type && type < NUM_OF_PCM_TYPES
						?
						PcmMsgTypeString[type] : "Invalid");
	return name;
}
////////////////////////////////////////////////////////////////////////
EPcmMsgType CPcmMessage::StringToMsgType(string& str)
{
	for (int i = 0; i < NUM_OF_PCM_TYPES;i++)
		if (!strcmp(PcmMsgTypeString[i],str.c_str()))
			return 	(EPcmMsgType)i;
			
	return eInvalid;
}
////////////////////////////////////////////////////////////////////////
bool CPcmMessage::GetNodeValueBool(strmap& mpMsg, string str)//char* str1)
{
	//string str(str1);
	bool ans = false;
	strmap::iterator it;
	it = mpMsg.find(str);
    if (it != mpMsg.end())
    {
    	string strVal = mpMsg[str];
    	ans = g_CommonFun.stringToBool(strVal);
    }
    else
    {
    	DBGPASSERT(301);
    	PTRACE2(eLevelInfoNormal,"CPcmMessage::GetNodeValueBool can't find str in strMap ",str.c_str());
    }
    
    return ans;
}
////////////////////////////////////////////////////////////////////////
int	CPcmMessage::GetNodeValueInteger(strmap& mpMsg, string str)//char* str1)
{
	//string str(str1);
	int ans = -1;
	strmap::iterator it;
	it = mpMsg.find(str);
	if (it != mpMsg.end())
	{
		string strVal = mpMsg[str];
		ans = atoi(strVal.c_str());	
	}
	else
	{
		DBGPASSERT(301);
	    PTRACE2(eLevelInfoNormal,"CPcmMessage::GetNodeValueInteger can't find str in strMap ",str.c_str());
	}
	
	return ans;
}
////////////////////////////////////////////////////////////////////////
string CPcmMessage::GetNodeValueString(strmap& mpMsg, string str)//char* str1)
{
	//string str(str1);
	string ans = "";
	strmap::iterator it;
	it = mpMsg.find(str);
	if (it != mpMsg.end())
	{
		ans = mpMsg[str];	
	}
	else
	{
		DBGPASSERT(301);
	    PTRACE2(eLevelInfoNormal,"CPcmMessage::GetNodeValueString can't find str in strMap ",str.c_str());
	}
		
	return ans;
	
}
////////////////////////////////////////////////////////////////////////
void CPcmMessage::AddChildNodeBool(strmap& mpMsg, string strKey, bool val)
{
	mpMsg[strKey]=g_CommonFun.boolToString(val);
}
////////////////////////////////////////////////////////////////////////
void CPcmMessage::AddChildNodeInteger(strmap& mpMsg, string strKey, int val)
{
	mpMsg[strKey]=itoa(val);
}
////////////////////////////////////////////////////////////////////////
void CPcmMessage::AddChildNodeString(strmap& mpMsg, string strKey, string val)
{
	mpMsg[strKey]=val;
}
////////////////////////////////////////////////////////////////////////
void CPcmMessage::AddChildNodeByteToBoolStr(strmap& mpMsg, string strKey, BYTE val)
{
	if (val)
		mpMsg[strKey]="true";
	else
		mpMsg[strKey]="false";
}
////////////////////////////////////////////////////////////////////////
LayoutType CPcmMessage::TranslatePcmApiLayoutTypeToMcmsLayoutType(int pcmApiLayoutType)
{
	LayoutType ans = CP_NO_LAYOUT;
	//TMcmsPcmLayoutType CPcmMessage::g_LayoutTypesTbl[NUM_OF_MCMS_LAYOUT_TYPES]
	for (int i=0; (i<NUM_OF_MCMS_LAYOUT_TYPES) && (ans == CP_NO_LAYOUT); i++)
	{
		if (g_LayoutTypesTbl[i].pcmApiLayoutType == pcmApiLayoutType)
			ans = g_LayoutTypesTbl[i].mcmsLayoutType;
	}

	return ans;
}
////////////////////////////////////////////////////////////////////////
int	CPcmMessage::TranslateMcmsLayoutTypeToPcmApiLayoutType(LayoutType mcmsLayoutType)
{
	int ans = -1;
	//TMcmsPcmLayoutType CPcmMessage::g_LayoutTypesTbl[NUM_OF_MCMS_LAYOUT_TYPES]
	for (int i=0; (i<NUM_OF_MCMS_LAYOUT_TYPES) && (ans == -1); i++)
	{
		if (g_LayoutTypesTbl[i].mcmsLayoutType == mcmsLayoutType)
			ans = g_LayoutTypesTbl[i].pcmApiLayoutType;
	}

	return ans;
}
////////////////////////////////////////////////////////////////////////
int	CPcmMessage::TranslatePcmPaneIndexToMcmsSubImageId(LayoutType mcmsLayoutType, int paneIndex)
{
	int ans = -1;
	WORD numOfSubImages = ::GetNumbSubImg(mcmsLayoutType);
	if ( numOfSubImages <=  paneIndex)
	{
		CMedString cstr;
		cstr << "CPcmMessage::TranslatePcmPaneIndexToMcmsSubImageId illegal pane index!!!\n";
		cstr << ::LayoutTypeAsString[mcmsLayoutType] << " ,pane index: " << paneIndex;
		if (numOfSubImages == paneIndex && IsFlexLayout(mcmsLayoutType))
		{
			cstr << "\nthis is flex layout and pane index == num sub images (it is pcm limitiation) --> return status ok";
			ans = MAX_SUB_IMAGES_IN_LAYOUT;
		}
		PTRACE(eLevelInfoNormal,cstr.GetString());
		return ans;
	}
	//TMcmsPcmLayoutType CPcmMessage::g_LayoutTypesTbl[NUM_OF_MCMS_LAYOUT_TYPES]
	for (int i=0; (i<NUM_OF_MCMS_LAYOUT_TYPES) && (ans == -1); i++)
	{
		if (g_LayoutTypesTbl[i].mcmsLayoutType == mcmsLayoutType)
			ans = g_LayoutTypesTbl[i].paneIndexes[paneIndex];
	}

	return ans;
}
////////////////////////////////////////////////////////////////////////
bool CPcmMessage::IsFlexLayout(LayoutType mcmsLayoutType)
{
	bool ans = false;
	switch(mcmsLayoutType)
	{
	case CP_LAYOUT_1X2_FLEX:
	case CP_LAYOUT_1P2HOR_RIGHT_FLEX:
	case CP_LAYOUT_1P2HOR_LEFT_FLEX:
	case CP_LAYOUT_1P2HOR_UP_RIGHT_FLEX:
	case CP_LAYOUT_1P2HOR_UP_LEFT_FLEX:
	case CP_LAYOUT_2X2_UP_RIGHT_FLEX:
	case CP_LAYOUT_2X2_UP_LEFT_FLEX:
	case CP_LAYOUT_2X2_DOWN_RIGHT_FLEX:
	case CP_LAYOUT_2X2_DOWN_LEFT_FLEX:
	case CP_LAYOUT_2X2_RIGHT_FLEX:
	case CP_LAYOUT_2X2_LEFT_FLEX:
	{
		ans = true;
		break;
	}
	default:
	{
		break;
	}
	}
	return ans;
}
////////////////////////////////////////////////////////////////////////
bool CPcmMessage::IsValidPcmMessage(CSmallString& details)
{
	bool ans = true; 
	if (0 > type || type > NUM_OF_PCM_TYPES)
	{
		details << "illegal type: " << type << " (expected value should be between  to " << NUM_OF_PCM_TYPES << "\n";
		ans = false;
	}
	if (action_name.size() <= 0)
	{
		details << "illegal action name: " << action_name.c_str() << "\n";
		ans = false;
	}
	return ans;	
}
////////////////////////////////////////////////////////////////////////
void CPcmMessage::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	mpMsg["SOURCE_GUID"]=source_guid? itoa(source_guid) : "";
	mpMsg["TARGET_GUID"]=target_guid? itoa(target_guid) : "";
	mpMsg["TERM_ID"]=(term_id >= 0 )? itoa(term_id) : ""; 
	mpMsg["_xml_msg_name"]= MsgTypeToString(type);
	mpMsg["_xml_msg_id"]=action_name;
	
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}
////////////////////////////////////////////////////////////////////////
// build the object from strmap
void CPcmMessage::DeSerializeXml(strmap& mpMsg)
{
	/*source_guid = atoi(mpMsg["SOURCE_GUID"].c_str());
	target_guid = atoi(mpMsg["TARGET_GUID"].c_str());
	term_id = atoi(mpMsg["TERM_ID"].c_str());
	type = StringToMsgType(mpMsg["_xml_msg_name"]);
	action_name = mpMsg["_xml_msg_id"];
	*/
	source_guid = GetNodeValueInteger(mpMsg,"SOURCE_GUID");
	target_guid = GetNodeValueInteger(mpMsg,"TARGET_GUID");
	term_id = GetNodeValueInteger(mpMsg,"TERM_ID");
	string msgTypeStr = GetNodeValueString(mpMsg,"_xml_msg_name");
	type = StringToMsgType(msgTypeStr);
	action_name = GetNodeValueString(mpMsg,"_xml_msg_id");
}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
CPcmIndication::CPcmIndication(int termId):CPcmMessage(termId)
{
	type = eIndication;
}
////////////////////////////////////////////////////////////////////////
CPcmIndication::CPcmIndication(CPcmIndication& other):CPcmMessage(other)
{
	
}
////////////////////////////////////////////////////////////////////////
CPcmIndication::~CPcmIndication()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmIndication::NameOf() const
{
	return "CPcmIndication";
}
////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
CPcmCommand::CPcmCommand(int termId):CPcmMessage(termId)
{
	type = eCommand;
}
////////////////////////////////////////////////////////////////////////
CPcmCommand::CPcmCommand(CPcmCommand& other):CPcmMessage(other)
{
	
}
////////////////////////////////////////////////////////////////////////
CPcmCommand::~CPcmCommand()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmCommand::NameOf() const
{
	return "CPcmCommand";
}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
CPcmConfirm::CPcmConfirm(int termId, string action ,int result):CPcmMessage(termId)
{
	type = eConfirm;
	action_name = action;
	m_result = result;
}
////////////////////////////////////////////////////////////////////////
CPcmConfirm::CPcmConfirm(CPcmConfirm& other):CPcmMessage(other)
{
	
}
////////////////////////////////////////////////////////////////////////
CPcmConfirm::CPcmConfirm(CPcmCommand& other):CPcmMessage(other)
{
	type = eConfirm;
	m_result = STATUS_OK;
}
////////////////////////////////////////////////////////////////////////
CPcmConfirm::~CPcmConfirm()
{
}
////////////////////////////////////////////////////////////////////////
const char* CPcmConfirm::NameOf() const
{
	return "CPcmConfirm";
}
////////////////////////////////////////////////////////////////////////
void CPcmConfirm::SerializeXmlToStr(strmap& mpMsg, string& str)
{
	CPcmMessage::SerializeXmlToStr(mpMsg, str);
	mpMsg["RESULT"]=itoa(m_result);
	
	str = g_CommonFun.xml_MapToXmlStr(mpMsg,rootName,rootVer);
}

