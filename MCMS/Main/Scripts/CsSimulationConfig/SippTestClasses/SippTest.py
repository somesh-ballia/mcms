#!/mcms/python/bin/python
import os, string
import sys
import subprocess
import abc
import SippTestUtils
from SippTestUtils import *

class SippTest(object):
	
	def __init__(self, sippTestUtils, testName , authorMail):
		
		self.sippTestUtils  = sippTestUtils
		self.testName 		= testName
		self.authorMail 	= authorMail
		self.sippXmlName  	= self.GetSippXmlName()    # Without .xml
		self.confXmlName  	= self.GetConfXmlName() 	  # Without .xml 
		
		self.RunTest()
		return
	
	def RunTest(self):
		self.sippTestUtils.RunTest(self.testName , self.authorMail, self.sippXmlName, self.confXmlName, self.PreTestActionsFunc, self.CheckTestResultFunc, self.LastActionsFunc)
		return
	def InterfaceImpError(self, strFunc):
		print ""
		print "##############################################################################"
		print "##### " + strFunc + "() MUST BE IMPLEMENTED !!!"
		print "##############################################################################"
		print ""
		sys.exit(1)
		return
		
	@abc.abstractmethod
	def GetSippXmlName(self):
		self.InterfaceImpError("GetSippXmlName")
		return
		
	@abc.abstractmethod
	def GetConfXmlName(self):
		self.InterfaceImpError("GetConfXmlName")
		return

	@abc.abstractmethod
	def PreTestActionsFunc(self,sippTestUtils):
		self.InterfaceImpError("PreTestActionsFunc")
		return
		
	@abc.abstractmethod
	def CheckTestResultFunc(self,sippTestUtils, LOG, strErrMsg):
		self.InterfaceImpError("CheckTestResultFunc")
		return

	@abc.abstractmethod
	def LastActionsFunc(self, sippTestUtils):
		self.InterfaceImpError("LastActionsFunc")
		return
		
#---- End SippTest class------------------