#!/mcms/python/bin/python
import os, string
import sys
import subprocess
import abc
import SippTestUtils
import SippTest
from SippTestUtils import *
from SippTest import *

#### Name   	: Test Name 				# <- CHANGE 
#### Description: Test Description			# <- CHANGE 
#### Author		: Developer Name			# <- CHANGE 
#### Reviewer   : Reviewer Name				# <- CHANGE 
#### Date   	: D/M/Y						# <- CHANGE 
class YourTestName(SippTest):
	
	#Returns a string of your SIPP scenario XML file without the “.Xml“.
	def GetSippXmlName(self):
		return "YourSippXmlFileName" 		 # <- CHANGE : Set your external SIPP scenario XML file name (without the .XML)
	#--------------------------------
	
	#Returns a string of your Conference template XML file , without the “.Xml”
	def GetConfXmlName(self):
		return "YourConfTemplateXmlFileName" # <- CHANGE : Set your Conference Template XML file name (without the .XML)
	#--------------------------------
	
	#Add some code to do before the test starts to run, like set system flags you need for your test.
	#The function gets:
	#SippTestUtils object as a parameter called sippTestUtils which you can use.
	def PreTestActionsFunc(self, sippTestUtils):
		# <- CHANGE : Do some work before the test starts , like setting system flags
		return
	#--------------------------------
	
	#Add some code here to test the outcome of your SIPP call.
	#The function returns: isTestPassed which you should set to 1 for success or 0 for failure.
	#This function gets:
	#SippTestUtils - object as a parameter called sippTestUtils which you can use.
	#String – Called LOG , which is the Log file name after the SIPP scenario ended.
	#Array [] – Called  strErrMsg, which is the error message you should set if your test failed.
	def CheckTestResultFunc(self, sippTestUtils, LOG, strErrMsg):
		isTestPassed = 1;

		# <- CHANGE : Add you code here to check the LOG if your test has passed (isTestPassed = 1) or failed (isTestPassed = 0)
		
		return isTestPassed
	#--------------------------------
	
	#Add some code to do after the test has ended, like set system flags you changed in PreTestActionsFunc  back to their default values.
	#The function gets:
	#SippTestUtils - object as a parameter called sippTestUtils which you can use.
	def LastActionsFunc(self, sippTestUtils):
		# <- CHANGE : Do some work after the test ended , like setting back the system flags you previously changed
		return
	#--------------------------------
#---- End class------------------
