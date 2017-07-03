#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

# Write date = 8/8/13
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


class EqUtils:
    """This is a EQ utility class.
    It enables all sorts of EQ actions.
    GetEqListNumber
    GetEqByName
    DeleteEqById
    AddEq
    """
#------------------------------------------------------------------------------
    def GetEqListNumber(self, util, retires = 20):  
        print "Get EQ/MR list"

        for retry in range(retires+1):
            status = util.SendXmlFile('Scripts/ConfTamplates/GetMRList.xml',"Status OK")
            mr_list = util.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
            if len(mr_list) != 0:
                break
        
        print "Found EQs, len = " + str(len(mr_list))
        return len(mr_list)

#------------------------------------------------------------------------------
    def GetEqByName(self, util, EqName, retires = 20):  
        print "Get EQ/MR by name"

        found = "false"
        EqId  = 0
        status = util.SendXmlFile('Scripts/ConfTamplates/GetMRList.xml',"Status OK")
        mr_list = util.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        for index in range(len(mr_list)):
            if(EqName == mr_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                EqId = mr_list[index].getElementsByTagName("ID")[0].firstChild.data
                print "EQ was found - " + EqName
                found = "true"
                break
                
        return found, EqId
 
#------------------------------------------------------------------------------
    def DeleteEqById(self, util, EqId, retires = 20):  
        print "Remove EQ, Id:" + str(EqId)
        
        util.LoadXmlFile('Scripts/ConfTamplates/DeleteMR.xml')
        util.ModifyXml("TERMINATE_MEETING_ROOM","ID",EqId)
        util.Send("Status OK")
 
#------------------------------------------------------------------------------
    def AddEq(self, McmsUtil, IvrUtil, EqName = "EQ2", isAdHoc = "false", ProfId = 1):  
        print "Adding new EQ conf - " + EqName
        IvrService = IvrUtil.GetDefaultEqIvrService(McmsUtil) 
        #IvrService = "Entry Queue IVR Service"      
        McmsUtil.LoadXmlFile('Scripts/ConfTamplates/CreateNewEq.xml')
        McmsUtil.ModifyXml("RESERVATION","NAME",EqName)
        McmsUtil.ModifyXml("RESERVATION","AD_HOC",isAdHoc)
        McmsUtil.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",ProfId)
        McmsUtil.ModifyXml("RESERVATION","AV_MSG",IvrService)
        McmsUtil.Send()

        

