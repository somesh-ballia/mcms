//+========================================================================+
//                         ConfigManagerApi.cpp                            |
//+========================================================================+

#include "SnmpManagerApi.h"
//#include "SnmpDefines.h"/////////////
#include "OpcodesMcmsInternal.h"
#include "ProcessBase.h"
#include "ApiStatuses.h"

/////////////////////////////////////////////////////////////////////////////
CSnmpManagerApi::CSnmpManagerApi() // constructor
        :CManagerApi(eProcessSNMPProcess)
{

}

/////////////////////////////////////////////////////////////////////////////
CSnmpManagerApi::~CSnmpManagerApi() // destructor
{
}

/////////////////////////////////////////////////////////////////////////////
/*STATUS CSnmpManagerApi::UpdateManagmentIp(const std::string & mamagment_ip,const std::string & switch_ip)
{

	CSegment*  seg = new CSegment;
			
	*seg << mamagment_ip
	     << switch_ip;

    OPCODE opcode;

	STATUS res =  SendMessageSync(seg,
                                  UPDATE_SNMP_MANAGMENT_IP,
                                  30*SECOND,
                                  opcode);

    if (res == STATUS_OK)
        return opcode;
    else
        return res;

}*/
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// STATUS CSnmpManagerApi::UpdateSNMPnotification()
// {

// 	CSegment*  seg = new CSegment;
			
// 	/**seg << mamagment_ip
// 	     << switch_ip;
// */
//     OPCODE opcode;

// 	STATUS res =  SendMessageSync(seg,
//                                   UPDATE_SNMP_IP_NOTIFICATION,
//                                   30*SECOND,
//                                   opcode);

//     if (res == STATUS_OK)
//         return opcode;
//     else
//         return res;

// }
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/*STATUS CSnmpManagerApi::CsMNGR_UpdateCsIp_MFA_IpAndGK_IP(const std::string & cs_ip,
                                                         const std::string & MFA1_ip,
                                                         const std::string & MFA2_ip,
                                                         const std::string & Gk_ip)
{

	CSegment*  seg = new CSegment;
			
	*seg << cs_ip
         << MFA1_ip
	     << MFA1_ip
         << Gk_ip;

    OPCODE opcode;

	STATUS res =  SendMessageSync(seg,
                                  CS_UPDATE_SNMP,
                                  30*SECOND,
                                  opcode);

    if (res == STATUS_OK)
        return opcode;
    else
        return res;

}*/
/////////////////////////////////////////////////////////////////////////////


