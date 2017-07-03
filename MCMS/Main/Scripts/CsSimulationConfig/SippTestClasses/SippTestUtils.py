#!/mcms/python/bin/python

# currently we don't want valgrind this can change in phase3
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#*export CS_SIMULATION_TEST=YES

#*export GIDEONSIM_MNGMNT_CONFIG_FILE=Cfg/NetworkCfg_Management.xml

#*export USE_ALT_IP_SERVICE=Scripts/CsSimulationConfig/IPServiceList_default_udp.xml

#*export USE_ALT_MNGMNT_SERVICE=Scripts/CsSimulationConfig/NetworkCfg_Management_ipv4_only.xml

#*export ENDPOINTSSIM=NO

#*PRERUN_SCRIPTS =make_cs_simulation_env.py
#-LONG_SCRIPT_TYPE
 
 #-- SKIP_ASSERTS
from McmsTargetConnection import *
from McmsConnection import *
#from test_recovery_policy_Utilities import *
#-- SKIP_ASSERTS      




import os, string
import sys
import subprocess
import re
import traceback


from McmsConnection import *
from ConfUtils.ConfCreation import *
from ..SippUtill import *
from subprocess import call
from sys import stdout


class SippTestUtils:
	
	def __init__(self):
		self.mcmsConnection = McmsConnection()
		self.mcmsConnection.Connect()
		sleep(2)
		self.confActionsClass = ConfCreation() # conf util class
	#-----------------------
	
	def RunCommand(self, command):
		ps = subprocess.Popen(command , shell=True, stdout=subprocess.PIPE)
		output = ps.stdout.read()
		ps.stdout.close()
		ps.wait()
		return output
	#-----------------------

	def SetSysFlag(self, strFlag , strValue):

		call(["Scripts/McuCmd.sh", "set","all",strFlag,strValue])

		return
	#-----------------------

	def GetLineFromLog(self, strToFind , LOG):

		return RunCommand("egrep -e \"" + strToFind + "\" " + LOG )
	#-----------------------

	def GetLinesFromLog(self, strToFind , numOflines, LOG):

		return RunCommand("egrep -e \"" + strToFind + "\" -A" + str(numOflines) + " -m 1 " + LOG )
	#-----------------------

	def GetAllStringLinesFromLog(self , LOG):
		
		strRet = RunCommand("awk 'NR>=0 {print NR,$0}' " + LOG )
		return strRet
	#-----------------------

	def GetOpcodeContentFromString(self, strOpcodeName ,strProcessName,  strInput, returnedStringArr, showIndex=1, searchFromLine=0):
		
		bMatchFound		= 0
		nOpcodeLineNum  = -1
		search_obj 		= re.finditer('[0-9]+.(Opcode:' + strOpcodeName + ')(.)*?(D:)..(/)..(/)..', strInput , re.DOTALL)
		
		if(search_obj and showIndex >= 1):
			for match in search_obj:
				strOpcodeContent = str(match.group())
				nOpcodeLineNum   = int(self.GetLineNumber(strOpcodeContent))
				bProcessMatch 	 = self.IsProccess( strOpcodeContent, strInput , strProcessName)
				
				if(bProcessMatch and (nOpcodeLineNum >= searchFromLine)):
					if(showIndex == 1):
						bMatchFound = 1;
						break;
					else:
						showIndex = showIndex - 1
		
		if(bMatchFound):
			returnedStringArr.append(strOpcodeContent);
		else:
			nOpcodeLineNum = -1
			
		return nOpcodeLineNum
	#-----------------------

	def GetLineNumber(self, strInput):
		
		strLineNumber = "-1"
		search_obj 	= re.search('[0-9]+', strInput , re.DOTALL)
		if(search_obj):
			strLineNumber = str(search_obj.group(0))
		
		return strLineNumber
	#-----------------------

	def IsProccess(self, strOpcodeContent , strBigInput , strProcessName):
		
		bProcessMatch = 0

		nLineNumber = int(self.GetLineNumber(strOpcodeContent))
		nLineNumber = nLineNumber - 2
		search_process = re.search('\n' + str(nLineNumber) +'.*?\n', strBigInput)
		if(search_process):
			strProcessLine = search_process.group(0)
			search_process 	= re.search('(.)*(P:' + strProcessName + ')(.)*(\n)', strProcessLine)
			if(search_process):
				bProcessMatch = 1
		
		return bProcessMatch
	#-----------------------

	def GetFieldNumValue(self, strToFind , strFrom , showIndex=1):
				
		bMatchFound = 0;
		search_obj 	= re.finditer(strToFind +'.*?([0-9]+)', strFrom)
		
		if(search_obj and showIndex >= 1):
			for match in search_obj:
				strFound = str(match.group())
				if(showIndex == 1):
					bMatchFound = 1;
					search_obj 	= re.search('([0-9]+)', strFound)
					if(search_obj):
						nRetVal = int(search_obj.group(0))
					break;
				else:
					showIndex = showIndex - 1
						
		return nRetVal
	#-----------------------
	
	def GetLog(self):
		
		LOG=RunCommand("ls -latr LogFiles/*.txt | tail -1  |  awk '{print $9}'")
		print "sippTest: log = " + str(LOG)

		return LOG
	#-----------------------


	def EndTest(self, sippTest, isTestPassed, strErrMsg, isException=0):
		
		sippTest.KillAllSippProcess()
		self.HandleResults(sippTest , isTestPassed, strErrMsg, isException)
		return
	#-----------------------

	def StartTest(self, sippTest, sippXmlName):
		
		
		confName = sippXmlName
		mcmsConn = self.mcmsConnection
		ConfActionsClass = self.confActionsClass
		
		#Init test params
		num_of_parties=1
		
		#Start the test
		confid = ConfActionsClass.WaitConfCreated(mcmsConn, confName)
		print "sippTest: conference ID = " + str(confid)
		sippTest.Run()
		mcmsConn.WaitUntilAllPartiesConnected(confid,num_of_parties,30)
		sleep(2)
		mcmsConn.DeleteConf(confid)
		os.system("Scripts/Flush.sh")
		
		return
	#-----------------------

	def PrepareTest(self, testName, authorMail,  sippXmlName, confXmlName):
		
		mcmsConn = self.mcmsConnection
		ConfActionsClass = self.confActionsClass
		sleep(2)
		os.system("Scripts/Flush.sh")
		
		sippTest 			= SippUtill() # create sipp EP obj
		sippTest.testName 	= testName
		sippTest.authorMail	= authorMail
		confName 			= sippXmlName
		sippTest.SetCallParams('Scripts/CsSimulationConfig/SippXmlFiles/'+ sippXmlName +'.xml',sippXmlName)
		ConfActionsClass.CreateConf(mcmsConn, confName, 'Scripts/CsSimulationConfig/ConfTemplates/'+ confXmlName +'.xml', "NONE")
		
		return sippTest
	#-----------------------
	
	def HandleResults(self, sippTest, isTestPassed, strErrMsg, isException=0):
		if (isTestPassed == 0):
			# If we get here it means - test FAILED
			self.HandleTestFailed(strErrMsg, sippTest.testName, sippTest.authorMail, isException)
		#------------------------------------------
		else:
			# If we get here it means - test PASSED
			self.HandleTestPassed()
		return
	#------------------------------------------
		
	def HandleTestFailed(self, errorMsg, testName, authorMail, isException=0):
		
		strErrorMsg = "Unknown ERROR!"
		if(errorMsg):
			strErrorMsg = errorMsg[0]

		print ""
		print "##############################################################################"
		print "##### SippTest: TEST FAILED !!!"
		print "##### SippTest: Test Name	: " + testName
		print "##### SippTest: Author		: " + authorMail		
		print "##### SippTest: Fail reason	: " + strErrorMsg
		print "##############################################################################"
		print ""
		if(isException):
			sys.exit(1)
		return
	#-----------------------
	
	def HandleTestPassed(self):
		print ""
		print "===================================="
		print "====  SippTest: TEST PASSED  =======" 
		print "===================================="
		print ""
		return
	#-----------------------
	
	def RunTest(self, testName , authorMail, sippXmlName, confXmlName, PreTestActionsFunc , CheckTestResultFunc, LastActionsFunc ):
		
		print ""
		print "================================================================="
		print "=== SippTest: START : " + testName
		print "=== SippTest: Author: " + authorMail
		print "================================================================="
		print ""
		
		try:
			sippTest = self.PrepareTest(testName, authorMail, sippXmlName, confXmlName)
			
			try:
				PreTestActionsFunc(self);
				
				self.StartTest(sippTest , sippXmlName)
				
				LOG 	 	 	= self.GetLog()
				
				strErrMsg 		= []
		
				isTestPassed	= CheckTestResultFunc(self, LOG, strErrMsg);
				
				LastActionsFunc(self);
				
			except:
				traceback.print_exc(file=sys.stdout)
				strErrMsg 	 = []
				isTestPassed = 0
				strErrMsg.append("Unexpected exception 1 !")			
				self.EndTest(sippTest, isTestPassed, strErrMsg, 1)
			
			self.EndTest(sippTest, isTestPassed, strErrMsg)

		except:
			traceback.print_exc(file=sys.stdout)
			strErrMsg 	 = []
			isTestPassed = 0
			strErrMsg.append("Unexpected exception 2 !")	
			self.HandleTestFailed(strErrMsg, testName, authorMail, 1)
			
		return
	#-----------------------
	
#---- End SippTestUtils class------------------
	