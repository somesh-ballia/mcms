#!/mcms/python/bin/python
import os, string
import sys
import subprocess
import abc
import SippTestUtils
import SippTest
from SippTestUtils import *
from SippTest import *

#### Name   	: Truncate SIP domain from site name #6
#### Description: Truncate domain SIP from site name feature test
#### Test is simulating ordinary call scenario with diffrent SIP headers names values when the SIP_OMIT_DOMAIN_FROM_PARTY_NAME flag set to YES or NO 
#### 1. SIP_OMIT_DOMAIN_FROM_PARTY_NAME=NO.
#### 2. Contact display  contains sitename="TestSiteName@domain.com".
#### 3. verify site name is TestSiteName@domain.com.
#### Author		: Arbel M.
#### Reviewer   : Arbel M.
#### Date   	: 15/09/14
class TruncateSipDomainTest_6(SippTest):
	
	def GetSippXmlName(self):
		return "truncateSipDomain_6"
	#--------------------------------
	
	def GetConfXmlName(self):
		return "AddConfTemplate"
	#--------------------------------
	
	def PreTestActionsFunc(self, sippTestUtils):
			sippTestUtils.SetSysFlag("SIP_OMIT_DOMAIN_FROM_PARTY_NAME", "NO"); # Set some system flags
			return
	#--------------------------------
	
	def CheckTestResultFunc(self, sippTestUtils, LOG, strErrMsg):
		isTestPassed = 1;
		
		sitenameline=RunCommand("egrep -e \"setting site name to visual name\" " + LOG )
		print "sitenameline = " + str(sitenameline)
		siteNameExists=string.find(str(sitenameline),"TestSiteName@domain.com")
		
		shtrudelExists=-1
		
		if siteNameExists == -1:
			strErrMsg.append("Test FAILED - party visual name should be = TestSiteName@domain.com")
			isTestPassed = 0;
		
		else:
			if siteNameExists != -1:
			    shtrudelExists=string.find(str(sitenameline),"@")
			
			if shtrudelExists == -1:
				strErrMsg.append("Test FAILED - party visual name should contain the domain name \"@ \" ")
				isTestPassed = 0;
        
		return isTestPassed
	#--------------------------------
	
	def LastActionsFunc(self, sippTestUtils):
		sippTestUtils.SetSysFlag("SIP_OMIT_DOMAIN_FROM_PARTY_NAME", "YES"); # Set some system flags
		return
	#--------------------------------

#---- End TruncateTest class------------------
