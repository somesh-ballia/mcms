
// IpCsContentRoleToken.h
// Uri Avni

#ifndef _IP_CS_CONTENT_ROLE_TOKEN_H_
#define _IP_CS_CONTENT_ROLE_TOKEN_H_


//opcodes

typedef enum
{
	kUnknownRoleTokenOpcode  = -1,
	
	kRoleTokenFirstOpcodeReq =  0,
	kRoleProviderIdentityReq,		//0x01
	kNoRoleProviderReq,				//0x02
	kRoleTokenAcquireReq,			//0x03
	kRoleTokenWithdrawReq,			//0x04
	kRoleTokenReleaseReq,			//0x05
	kRoleTokenAcquireAckReq,		//0x06
	kRoleTokenAcquireNakReq,		//0x07
	kRoleTokenWithdrawAckReq,		//0x08
	kRoleTokenReleaseAckReq,		//0x09
	kRoleTokenLastOpcodeReq,		//0x0A

	kRoleTokenFirstOpcodeInd,		//0x0B
	kRoleProviderIdentityInd,		//0x0C
	kNoRoleProviderInd,				//0x0D
	kRoleTokenAcquireInd,			//0x0E
	kRoleTokenWithdrawInd,			//0x0F
	kRoleTokenReleaseInd,			//0x10
	kRoleTokenAcquireAckInd,		//0x11
	kRoleTokenAcquireNakInd,		//0x12
	kRoleTokenWithdrawAckInd,		//0x13
	kRoleTokenReleaseAckInd,		//0x14
	kRoleTokenLastOpcodeInd,		//0x15

	/*start H239 Generic Opcodes: */
	
	kStartH239TokenOpcodes = 0x20,
	kFlowControlReleaseRequest, 	 //0x21
	kFlowControlReleaseResponse,	 //0x22 
	kPresentationTokenRequest,		 //0x23
	kPresentationTokenResponse, 	 //0x24
	kPresentationTokenRelease,		 //0x25
	kPresentationTokenIndicateOwner  //0x26
} ERoleTokenOpcode;

// labels:
//--------

// label is 8 bits as folowing:
// ---------------------------
//   7   6    5   4-0
// | E | SR | T | Role |

// E:    Extension bit must be zero
// SR:   Sub-Role bit: 1=Sub-Role label follows, 0=No sub-role label
// T:    Terminator bit: 1 = end of list, 0 more labels follow
// Role: 000000:      Reserved
//       000001:      People
//       000010:      Content
//       000011-11111:Reserved


#define LABEL_PEOPLE  0x21
#define LABEL_CONTENT 0x22

typedef enum
{
	kRolePeople					= 0,
	kRoleContent				= 1,	//0001
	//structures with the following roles will be sent as extendedVideoCapability:
	kRolePresentation			= 2,	//0010
	kRoleContentOrPresentation	= 3,	//0011 // Don't change value!! kRoleContent & kRolePresentation.
	kRoleLive					= 4,	//0100
	kRoleLiveOrPresentation		= 6,	//0110 // Don't change value!! kRoleLive & kRolePresentation.
	kRoleUnknown				= 7

}ERoleLabel;

//  H239 generic message
// ======================

typedef enum
{
	kFlowControlReleaseRequestId		=  1,
	kFlowControlReleaseResponseId,		// 2 
	kPresentationTokenRequestId,		// 3
	kPresentationTokenResponseId, 		// 4
	kPresentationTokenReleaseId,		// 5
	kPresentationTokenIndicateOwnerId	// 6

}Eh239SubMsgIdentifier;

#define kGenParamBitRate				41
#define kGenParamChannelId				42
#define kGenParamSymBreak				43
#define kGenParamTerminalLabel			44
#define kGenParamAck					126
#define kGenParamReject					127

#endif //_IP_CS_CONTENT_ROLE_TOKEN_H_
