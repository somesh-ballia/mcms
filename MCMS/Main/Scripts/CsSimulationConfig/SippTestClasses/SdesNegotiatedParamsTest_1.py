#!/mcms/python/bin/python
import os, string
import re
import sys
import subprocess
import abc
import SippTestUtils
import SippTest
from SippTestUtils import *
from SippTest import *

#### Name   	: SdesNegotiatedParamsTest_1
#### Description: Verify 200OK chooses the same values as the INVITE_IND first SDES of every m-line with same MKI, TAG values as 
#### Author		: Arbel M.
#### Reviewer   : Arbel M.
#### Date   	: 24/09/14
class SdesNegotiatedParamsTest_1(SippTest):
	
	def GetSippXmlName(self):
		return "sdesNegotiatedParams_1"
	#--------------------------------
	
	def GetConfXmlName(self):
		return "AddEncryptWPConfTemplate"
	#--------------------------------
	
	def PreTestActionsFunc(self, sippTestUtils):
		#sippTestUtils.SetSysFlag("MAX_TRACE_LEVEL", "d"); # Set some system flags
		return
	#--------------------------------
	
	def CheckInviteWithResponseFieldValue(self, sippTestUtils, strErrMsg, strFieldInTest , strInviteInd, strInviteRespReq):
		isTestPassed = 1;
		
		print "CheckInviteWithResponseFieldValue()  <<" + str(strFieldInTest) + ">> under test:"

		#Get all  values from every m-line in the INVITE_IND
		#We know the sipp script has 2 SDES per m-line in this test so we get the first for every m-line only
		nAudioValFromInvite = -1;
		nVideoValFromInvite = -1;
		nFeccValFromInvite = -1;
		if(isTestPassed):
			nAudioValFromInvite = sippTestUtils.GetFieldNumValue(strFieldInTest , strInviteInd, 1)	
			print "---AudioValFromInvite = " + str(nAudioValFromInvite)
			nVideoValFromInvite = sippTestUtils.GetFieldNumValue(strFieldInTest , strInviteInd, 3)	
			print "---VideoValFromInvite = " + str(nVideoValFromInvite)
			nFeccValFromInvite = sippTestUtils.GetFieldNumValue(strFieldInTest, strInviteInd, 5)	
			print "---FeccValFromInvite = " + str(nFeccValFromInvite)
		
		#Get all  values from every m-line in the INVITE_RESPONSE_REQ
		#We should answer with only one SDES per m-line
		nAudioValFromInviteResp = -1
		nVideoValFromInviteResp = -1
		nFeccValFromInviteResp = -1
		if(isTestPassed):
			nAudioValFromInviteResp = sippTestUtils.GetFieldNumValue(strFieldInTest , strInviteRespReq, 1)	
			print "---AudioValFromInviteResp = " + str(nAudioValFromInviteResp)
			nVideoValFromInviteResp = sippTestUtils.GetFieldNumValue(strFieldInTest , strInviteRespReq, 2)	
			print "---VideoValFromInviteResp = " + str(nVideoValFromInviteResp)
			nFeccValFromInviteResp = sippTestUtils.GetFieldNumValue(strFieldInTest, strInviteRespReq, 3)	
			print "---FeccValFromInviteResp = " + str(nFeccValFromInviteResp)

		#Check conditions to fail the test
		#Fail the test if the invite and 200OK values are different
		if(isTestPassed):
			if(nAudioValFromInvite != nAudioValFromInviteResp):
				isTestPassed = 0
				strErrMsg.append("InviteInd and InviteResp Audio values are different!")
			elif(nVideoValFromInvite != nVideoValFromInviteResp):
				isTestPassed = 0
				strErrMsg.append("InviteInd and InviteResp Video values are different!")
			elif(nFeccValFromInvite != nFeccValFromInviteResp):
				isTestPassed = 0
				strErrMsg.append("InviteInd and InviteResp Fecc values are different!")
	
		return isTestPassed
	#--------------------------------
	
	def CheckTestResultFunc(self, sippTestUtils, LOG, strErrMsg):
		isTestPassed = 1;
		
		#Get SIP_CS_SIG_INVITE_IND from the log
		strInviteInd		= ""
		strInviteIndArr 	= []
		strCall 			= sippTestUtils.GetAllStringLinesFromLog(LOG)
		nInviteIndLineNum	= sippTestUtils.GetOpcodeContentFromString("SIP_CS_SIG_INVITE_IND","ConfParty",strCall, strInviteIndArr,1,0)
		
		if(isTestPassed and nInviteIndLineNum != -1):
			strInviteInd = strInviteIndArr[0]
		else:
			isTestPassed = 0
			strErrMsg.append("Internal log parsing error!")


		#Get SIP_CS_SIG_INVITE_RESPONSE_REQ from the log
		strInviteRespReq		= ""
		strInviteRespReqArr		= []
		nInviteRespReqLineNum 	= -1
		if(isTestPassed):
			nInviteRespReqLineNum	= sippTestUtils.GetOpcodeContentFromString("SIP_CS_SIG_INVITE_RESPONSE_REQ","ConfParty",strCall, strInviteRespReqArr,1,nInviteIndLineNum)

		if(isTestPassed and nInviteIndLineNum != -1):
			strInviteRespReq = strInviteRespReqArr[0]
		else:
			isTestPassed = 0
			strErrMsg.append("Internal log parsing error!")

		if(isTestPassed):
			isTestPassed = self.CheckInviteWithResponseFieldValue(sippTestUtils, strErrMsg, "bIsMkiInUse" , strInviteInd, strInviteRespReq)

		if(isTestPassed):
			isTestPassed = self.CheckInviteWithResponseFieldValue(sippTestUtils, strErrMsg, "tag" , strInviteInd, strInviteRespReq)

		if(isTestPassed):
			isTestPassed = self.CheckInviteWithResponseFieldValue(sippTestUtils, strErrMsg, "cryptoSuite" , strInviteInd, strInviteRespReq)
		
		return isTestPassed
	#--------------------------------
	
	def LastActionsFunc(self, sippTestUtils):
		#sippTestUtils.SetSysFlag("MAX_TRACE_LEVEL", "n"); # Set some system flags
		return
	#--------------------------------

#---- End TruncateTest class------------------