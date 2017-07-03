#!/mcms/python/bin/python
import os, string
import sys
import subprocess
import abc
import SippTestUtils
import SippTest
from SippTestUtils import *
from SippTest import *

#### Name   	: Truncate SIP domain from site name #2
#### Description: Truncate domain SIP from site name feature test
#### Truncate domain SIP from site name feature test
#### Test is simulating ordinary call scenario with diffrent SIP headers names values when the SIP_OMIT_DOMAIN_FROM_PARTY_NAME flag set to YES or NO 
#### 1. SIP_OMIT_DOMAIN_FROM_PARTY_NAME=YES.
#### 2. From header contains sitename="TestSiteName" with "@.x.x.x.x" From Display is missing.
#### 3. verify site name is TestSiteName.
#### Author		: Arbel M.
#### Reviewer   : Arbel M.
#### Date   	: 15/09/14
class TruncateSipDomainTest_2(SippTest):
	
	def GetSippXmlName(self):
		return "truncateSipDomain_2"
	#--------------------------------
	
	def GetConfXmlName(self):
		return "AddConfTemplate"
	#--------------------------------
	
	def PreTestActionsFunc(self, sippTestUtils):
			sippTestUtils.SetSysFlag("SIP_OMIT_DOMAIN_FROM_PARTY_NAME", "YES"); # Set some system flags
			return
	#--------------------------------
	
	def CheckTestResultFunc(self, sippTestUtils, LOG, strErrMsg):
		isTestPassed = 1;
		
		sitenameline=RunCommand("egrep -e \"setting site name to visual name\" " + LOG )
		print "sitenameline = " + str(sitenameline)
		siteNameExists=string.find(str(sitenameline),"TestSiteName")
		
		shtrudelExists=-1
		
		if siteNameExists == -1:
			strErrMsg.append("party visual name should be = TestSiteName")
			isTestPassed = 0
		
		else:
			if siteNameExists != -1:
			    shtrudelExists=string.find(str(sitenameline),"@")
			
			if shtrudelExists != -1:
				strErrMsg.append("party visual name should not contain the domain name \"@ \" ")
				isTestPassed = 0

		return isTestPassed
	#--------------------------------
	
	def LastActionsFunc(self, sippTestUtils):
		sippTestUtils.SetSysFlag("SIP_OMIT_DOMAIN_FROM_PARTY_NAME", "YES"); # Set some system flags
		return
	#--------------------------------

#---- End TruncateTest class------------------
