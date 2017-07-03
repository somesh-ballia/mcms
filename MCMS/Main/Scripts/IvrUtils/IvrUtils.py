#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

# Write date = 10/8/13
# Write name = Uri A.


import sys
import os
import httplib, urllib
import xml.dom.minidom
from time import *
import socket
import string
#from datetime import date
from datetime import *

from xml.dom.minidom import parse, parseString, Document

#The following redirects error stream to the standard stream, 
#to allow capture of interpreter errors in script logs
sys.stderr = sys.stdout

from McmsConnection import *


class IvrUtils:
    """This is a IVR utility class.
    It enables all sorts of IVR actions.
    UpdateIvrService
    GetNoOfIvrServices
    AddNewIvrService
    GetDefaultEqIvrService
    IsNumberOfServicesEqual
    CheckIvrServiceDeleted
    CheckNewServiceAdded
    GetIvrServiceByName
    PrintAndCompareIvrServiceParam
    """

#------------------------------------------------------------------------------
#Wait until the new IVR service is added
    def DeleteIvrServiceByName(self, McmsUtil, ServiceName="IVR2"):  
        print "Deleting IVR service - " + ServiceName
        McmsUtil.LoadXmlFile('Scripts/IvrTamplates/DeleteNewIVR.xml')
        McmsUtil.ModifyXml("DELETE","SERVICE_NAME",ServiceName)
        McmsUtil.Send()

#------------------------------------------------------------------------------
#Wait until the new IVR service is added
    def UpdateIvrService(self, McmsUtil, RetriesNumber_updateParam, ServiceName="IVR2"):  
        print "updating IVR service - change " + ServiceName
        McmsUtil.LoadXmlFile('Scripts/IvrTamplates/UpdateIvrService.xml')
        McmsUtil.ModifyXml("AV_COMMON","SERVICE_NAME",ServiceName)
        McmsUtil.ModifyXml("IVR_PARAMS","INPUT_RETRIES_NUMBER",RetriesNumber_updateParam)
        McmsUtil.Send()

#------------------------------------------------------------------------------
    def GetNoOfIvrServices(self, McmsUtil):  
        print "Monitoring IVR service list..."
        McmsUtil.SendXmlFile('Scripts/IvrTamplates/GetIvrList.xml')
        ivr_services = McmsUtil.xmlResponse.getElementsByTagName("IVR_SERVICE")
        num_ivr_services = len(ivr_services)
        print "num_ivr_services = ", num_ivr_services
        return num_ivr_services

#------------------------------------------------------------------------------
    def AddNewIvrService(self, McmsUtil, ServiceName="IVR2"):  
        print "Adding new IVR service - " + ServiceName
        McmsUtil.LoadXmlFile('Scripts/IvrTamplates/AddIVR.xml')
        McmsUtil.ModifyXml("AV_COMMON","SERVICE_NAME",ServiceName)
        McmsUtil.Send()

#------------------------------------------------------------------------------
    def GetDefaultEqIvrService(self, McmsUtil):  
        print "Get default IVR service for EQ"
        McmsUtil.SendXmlFile('Scripts/IvrTamplates/GetIvrList.xml')
        ivr_services = McmsUtil.xmlResponse.getElementsByTagName("IVR_SERVICE")
        for index in range(len(ivr_services)):
            if("true" == ivr_services[index].getElementsByTagName("IS_ENTRY_QUEUE_SERVICE")[0].firstChild.data):
                IvrServiceForEq = ivr_services[index].getElementsByTagName("SERVICE_NAME")[0].firstChild.data
                print "Service Name - " + IvrServiceForEq
                return IvrServiceForEq

#------------------------------------------------------------------------------
#Wait until the services number is equal
    def IsNumberOfServicesEqual(self, McmsUtil, num_ivr_services, retries):  
        for retry in range (retries):
            sleep(1)
            McmsUtil.SendXmlFile('Scripts/IvrTamplates/GetIvrList.xml')
            ivr_services = McmsUtil.xmlResponse.getElementsByTagName("IVR_SERVICE")
            num_ivr_services_current = len(ivr_services)
            if num_ivr_services == num_ivr_services_current:
                print "Number Of IVR services is OK"
                break
            if retry == retries:
                McmsUtil.Disconnect()
                sys.exit("Failed - number of IVR services is not equal!!!")

#------------------------------------------------------------------------------
#check if new IVR service is deleted
    def CheckIvrServiceDeleted(self, McmsUtil, num_ivr_services, retries):  
        self.IsNumberOfServicesEqual(McmsUtil, num_ivr_services, retries)

#------------------------------------------------------------------------------
#check if the new IVR service is added
    def CheckNewServiceAdded(self, McmsUtil, num_ivr_services, retries):
        self.IsNumberOfServicesEqual(McmsUtil, num_ivr_services, retries)

#------------------------------------------------------------------------------
#Get IVR service by name
    def GetIvrServiceByName(self, McmsUtil, ServiceName = "IVR2"):
        print "Get IVR service by name"
        McmsUtil.SendXmlFile('Scripts/IvrTamplates/GetIvrList.xml')
        sleep(2)
        Ivr_List = McmsUtil.xmlResponse.getElementsByTagName("IVR_SERVICE")
        for index in range(len(Ivr_List)):
            if(ServiceName == Ivr_List[index].getElementsByTagName("SERVICE_NAME")[0].firstChild.data):
                return Ivr_List[index]
 
#------------------------------------------------------------------------------
#print and compare IVR service param (we check number of retries
    def PrintAndCompareIvrServiceParam(self, McmsUtil, RetriesNumber_updateParam, ServiceName = "IVR2"):
        print "Compare IVR service Params"
        McmsUtil.SendXmlFile('Scripts/IvrTamplates/GetIvrList.xml')
        sleep(2)
        IvrService = self.GetIvrServiceByName(McmsUtil, ServiceName)
        inputRetries = IvrService.getElementsByTagName("INPUT_RETRIES_NUMBER")[0].firstChild.data
        print "INPUT_RETRIES_NUMBER = ",inputRetries
        if inputRetries == str(RetriesNumber_updateParam):
            print "Success update the IVR service"
        else:
            McmsUtil.Disconnect()
            sys.exit("Failed to update IVR service!!!")
        

