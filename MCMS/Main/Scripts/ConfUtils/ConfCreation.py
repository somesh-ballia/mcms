#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

# Write date = 1/8/13
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
from PartyUtils.H323PartyUtils import *


class ConfCreation(H323PartyUtils):
    """This is a conf utility class.
    It enables all sorts of conf actions.
    AddHdProfile
    DelProfile
    CreateConf
    CreateConfNameAndDisplayName
    CreateConferences
    DeleteConf
    DeleteAllConferences
    WaitAllConferencesAreDeleted
    WaitConfCreated
    WaitAllOngoingChangedLayoutType
    """
    
#------------------------------------------------------------------------------
    def AddHdProfile(self, util, profileName, transfer_rate, fileName="Scripts/HD/XML/AddHdProfile.xml", hd_resolution="None"):
        """Create new Profile.
        
        profileName - name of profile to be created.
        fileName - XML file
        """
        print "Adding new Profile..."
        util.LoadXmlFile(fileName)
        util.ModifyXml("RESERVATION","NAME", profileName)
        util.ModifyXml("RESERVATION","TRANSFER_RATE", transfer_rate)
        if hd_resolution != "None" :
            util.ModifyXml("RESERVATION","HD_RESOLUTION", hd_resolution)
        
        util.Send()
        ProfId = util.GetTextUnder("RESERVATION","ID")
        print "Profile, named: " + profileName + " ,ID = " + ProfId + ", is added"
        return ProfId

#------------------------------------------------------------------------------
    def DelProfile(self, util, profId, fileName="Scripts/RemoveNewProfile.xml"):
        """Remove Profile.
        
        profId - ID of profile to be deleted.
        fileName - XML file
        """
        #print "Deleting profile: " + profId
        util.LoadXmlFile(fileName)
        util.ModifyXml("TERMINATE_PROFILE","ID",profId)
        util.Send("Status OK")
    
#------------------------------------------------------------------------------
    def CreateConf(self, util, confName, fileName, setMedia, profileID = "NONE"):  
        """Create a new conference.
        
        confName - conference name
        fileName - the xml file which will be used to define the conference.
        """
        print "Adding Conf " + confName + " ..."
        util.LoadXmlFile(fileName)
        if(setMedia == "AUDIO"):
            util.ModifyXml("RESERVATION", "MEDIA", "audio")
        if(profileID != "NONE"):
            util.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileID) 
        util.ModifyXml("RESERVATION","NAME",confName)
        util.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
        util.Send()

#------------------------------------------------------------------------------
    def CreateConfNameAndDisplayName(self, util, confName, displayName, fileName, setMedia, profileID = "NONE"):
        """Create a new conference.

        confName - conference name
	displayName - conference display name
        fileName - the xml file which will be used to define the conference.
        """
        print "Adding Conf " + confName + "\n"
        print "Conf Display Name " + displayName + " ..."
        util.LoadXmlFile(fileName)
        if(setMedia == "AUDIO"):
            util.ModifyXml("RESERVATION", "MEDIA", "audio")
        if(profileID != "NONE"):
            util.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileID)
        util.ModifyXml("RESERVATION","NAME",confName)
        util.ModifyXml("RESERVATION","DISPLAY_NAME",displayName)
        util.Send()

#------------------------------------------------------------------------------
    def CreateConferences(self, util, num_of_conf, confFile):
        
        for conf_num in range(num_of_conf):
            confname = "Conf"+str(conf_num+1)
            self.CreateConf(util, confname, confFile, "NONE")

        if (util.IsProcessUnderValgrind("Resource")):
            sleeptime = 5
        else:    
            sleeptime = 1
            
        util.Sleep(sleeptime)
            
#------------------------------------------------------------------------------
    def DeleteConf(self,util, confId, exp_sts = "In progress"):
        """Delete a given conference.
        """
        print "Delete conference ID: "+confId        
        util.LoadXmlFile('Scripts/ConfTamplates/DeleteConf.xml')
        util.ModifyXml("TERMINATE_CONF","ID",confId)
        util.Send(exp_sts)
    
#------------------------------------------------------------------------------
    def DeleteAllConferences(self, util, delayBetweenConfDel=0):
        """Delete all on-going conferences.
        """
        print "Delete all conferences..."
        util.SendXmlFile('Scripts/ConfTamplates/TransConfList.xml',"Status OK")
        conf_list = util.xmlResponse.getElementsByTagName("CONF_SUMMARY")
        for conf in conf_list:
            conf_id = conf.getElementsByTagName("ID")[0].firstChild.data
            self.DeleteConf(util, conf_id)
            sleep(delayBetweenConfDel)


#------------------------------------------------------------------------------
    def WaitAllConferencesAreDeleted(self, util, retires = 20):
        """Monitor conferences list until empty
        """
        print "Waiting until all conferences are deleted",
        for retry in range(retires+1):
            sys.stdout.write(".")
            sys.stdout.flush()
            util.SendXmlFile('Scripts/ConfTamplates/TransConfList.xml',"Status OK")
            if util.GetTextUnder("CONF_SUMMARY","ID") == "":
                print "\nAll conferences have been deleted."
                break
            if retry == retires:
                util.Disconnect()
                util.ScriptAbort("\nFailed to delete all conferences!!! Abort!")
            sleep(1)                

#------------------------------------------------------------------------------
    def WaitConfCreated(self, util, confName, retires = 20): 
        """Monitor conferences list until 'confName' is found.
        Returns conference ID.
        
        confName - lookup conf name 
        """
        print "Wait untill Conf \'" + confName + "\' is created...",
        bFound = False
        for retry in range(retires):
            status = util.SendXmlFile('Scripts/ConfTamplates/TransConfList.xml',"Status OK")
            ongoing_conf_list = util.xmlResponse.getElementsByTagName("CONF_SUMMARY")
            for index in range(len(ongoing_conf_list)):  
                if(confName == ongoing_conf_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                    confid = ongoing_conf_list[index].getElementsByTagName("ID")[0].firstChild.data
                    if confid != "":
                        print
                        bFound = True
                        break
            if(bFound):
                break
            if (retry == retires):
                print util.xmlResponse.toprettyxml(encoding="utf-8")
                util.Disconnect()                
                ScriptAbort("Can not monitor conf:" + status)
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)
        print "Created Conf, ID:" + str(confid)
        return confid
    
#------------------------------------------------------------------------------
    def WaitAllOngoingChangedLayoutType(self, util, confid, newConfLayout, num_retries=30, party_name_who_shouldnt_see_conf_layout = ""):
        print "Wait until all ongoing parties see new layout type:" + newConfLayout + "\n",
        util.LoadXmlFile('Scripts/ConfTamplates/TransConf2.xml')
        util.ModifyXml("GET","ID",confid)
        for retry in range(num_retries+1):
            util.Send()
            all_layouts = ""
            wanted_all_layouts = ""
            ongoing_parties = util.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            party_index = 0
            for party in ongoing_parties:
                if (party_name_who_shouldnt_see_conf_layout != ""):
                    party_name = ongoing_parties[party_index].getElementsByTagName("NAME")[0].firstChild.data
                    if (party_name == party_name_who_shouldnt_see_conf_layout):
                        party_index = party_index + 1
                        continue
                force = util.getElementsByTagNameFirstLevelOnly(ongoing_parties[party_index],"FORCE")[0]
                layouts = force.getElementsByTagName("LAYOUT")
                force_num = 0
                if(len(layouts) == 2):
                    force_num = 1
                layout = layouts[force_num].firstChild.data 
                all_layouts += (layout + " ")
                wanted_all_layouts += (newConfLayout +" ")
                if layout != newConfLayout:
                    if (retry == num_retries):
                        print util.xmlResponse.toprettyxml(encoding="utf-8")
                        util.Disconnect()
                        ScriptAbort("Party is not in conf new layout: "+ newConfLayout +", But in layout : " + layout)
                party_index = party_index + 1            
            if all_layouts == wanted_all_layouts:
                break
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)
        
