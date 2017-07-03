#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

# Write date = 11/8/13
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
from CapabilitiesSetsDefinitions import *

class Party:
    """This is a Party base class that activate all general party actions.
    It enables all sorts of party actions.
    WaitAllPartiesWereAdded
    WaitAllOngoingConnected
    WaitAllOngoingNotInIVR
    DeleteAllParties
    GetPartyId
    DisconnectParty
    WaitUntillPartyDeleted
    SetDelayBetweenParties
    SimulationDeleteAllSimParties
    SimulationDeleteCapsSets
    ConnectParty
    WaitPartyConnected
    """
#------------------------------------------------------------------------------
    def WaitAllPartiesWereAdded(self, util, confid, num_of_parties, num_retries, delayBetweenXmlReq = 1):
        """Monitor conference until it has 'num_of_parties' defined parties
        """
        util.LoadXmlFile('Scripts/ConfTamplates/TransConf2.xml')
        util.ModifyXml("GET","ID",confid)
        print "Wait Untill All parties are added..",
        util.Send()
        last_ongoing = 0
        for retry in range(num_retries+1):
            ongoing_parties = util.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            num_ongoing_parties = len(ongoing_parties)
            if last_ongoing != num_ongoing_parties:
                con_string =  "["+str(num_ongoing_parties)+"/"+str(num_of_parties)+"]"
                sys.stdout.write(con_string)
            last_ongoing = num_ongoing_parties
            if num_ongoing_parties == num_of_parties:
                print
                break
             
            sys.stdout.write(".")
            sys.stdout.flush()
            if (retry == num_retries):
                print util.xmlResponse.toprettyxml(encoding="utf-8")
                util.Disconnect()                
                ScriptAbort(str(num_of_parties - num_ongoing_parties)+" parties are not connected")
            util.Send()
            sleep(delayBetweenXmlReq)             

#------------------------------------------------------------------------------
    def WaitAllOngoingConnected(self, util, confid, num_retries=30, delayBetweenParticipants=1):
        """Monitor conference until all ongoing parties are connected.
        """
        print "Wait until all ongoing parties connect",
        util.LoadXmlFile('Scripts/ConfTamplates/TransConf2.xml')
        util.ModifyXml("GET","ID",confid)
        util.Send()
        last_num_connected = 0
        for retry in range(num_retries+1):
            ongoing_parties = util.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            num_ongoing_parties = len(ongoing_parties)
            num_connected = 0
            ongoing_parties = util.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            for party in ongoing_parties:
                status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
                if status != "connected":
                    if (retry == num_retries):
                        print util.xmlResponse.toprettyxml(encoding="utf-8")
                        util.Disconnect()                
                        ScriptAbort("party not connected :" + status)
                    
                else:
                    num_connected=num_connected+1;
            sys.stdout.write(".")
            if last_num_connected != num_connected:
                con_string =  "["+str(num_connected)+"/"+str(len(ongoing_parties))+"]"
                sys.stdout.write(con_string)
            sys.stdout.flush()                
            last_num_connected = num_connected                        
            if num_connected == len(ongoing_parties):
                print
                break
            sleep(delayBetweenParticipants)            
            util.Send()
            
#------------------------------------------------------------------------------
    def WaitAllOngoingNotInIVR(self, util, confid, num_retries=30):
        """Monitor conference until all ongoing parties are not in IVR.
        """
        print "Wait until all ongoing parties are no longer in IVR",
        util.LoadXmlFile('Scripts/ConfTamplates/TransConf2.xml')
        util.ModifyXml("GET","ID",confid)
        util.Send()
        last_num_connected = 0
        for retry in range(num_retries+1):
            ongoing_parties = util.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            num_ongoing_parties = len(ongoing_parties)
            num_connected = 0
            for party in ongoing_parties:
                status = party.getElementsByTagName("ATTENDING_STATE")[0].firstChild.data
                if status != "inconf":
                    if (retry == num_retries):
                        print util.xmlResponse.toprettyxml(encoding="utf-8")
                        util.Disconnect()                
                        ScriptAbort("party not inconf :" + status)
                    
                else:
                    num_connected=num_connected+1;
            sys.stdout.write(".")
            if last_num_connected != num_connected:
                con_string =  "["+str(num_connected)+"/"+str(len(ongoing_parties))+"]"
                sys.stdout.write(con_string)
            sys.stdout.flush()                
            last_num_connected = num_connected                        
            if num_connected == len(ongoing_parties):
                print
                break
            sleep(1)            
            util.Send()

#------------------------------------------------------------------------------
# delete all parties  
    def DeleteAllParties(self, util, confid, numParties, num_retries=30):
        """Delete all parties from a conf (identify by confId.
        """
        for x in range(numParties):
            partyname = confName+"_(00" + str(x) +")"
            print "disconnecting party: " + partyname
            partyId = self.GetPartyId(util, confid, partyname) 
            self.DisconnectParty(util, confid, partyId)        
            sleep(1)

 #------------------------------------------------------------------------------
    def GetPartyId(self, util, confid, partyname):
        """Monitor party in conference, and return it's id.
        
        confid - target conference id
        partyname - name of the party to monitor
        """
        util.LoadXmlFile('Scripts/ConfTamplates/TransConf2.xml')
        util.ModifyXml("GET","ID",confid)
	num_retries = 3 
	for retry in range(num_retries):
        	util.Send()
        	ongoing_party_list = util.xmlResponse.getElementsByTagName("ONGOING_PARTY")
       		print "the num of ongoing parties are: " + str(len(ongoing_party_list))
		if (len(ongoing_party_list) > 0):
			break
    
        if (len(ongoing_party_list) == 0):
            ScriptAbort("the party was not connected...")
        else:  
            for index in range(len(ongoing_party_list)):  
                if(partyname == ongoing_party_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                    partyid = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data   
                    return partyid
 
#------------------------------------------------------------------------------
    def DisconnectParty(self, util, confId, partyId): 
        """Disconnect a certain party from a certain conference.
        
        confId - the conference id in which the party is defined
        partyId - the id of the party to be disconnected
        """
        # disconnect participant 
        print "Disconnecting Party"
        util.LoadXmlFile('Scripts/PartyTamplates/DisconnectIPParty.xml')
        util.ModifyXml("SET_CONNECT", "ID",confId)
        util.ModifyXml("SET_CONNECT", "PARTY_ID", partyId)
        util.Send()

#------------------------------------------------------------------------------
    def WaitUntillPartyDeleted(self, util, confid, num_retries):
        print "Monitor Party list until empty in Conf: "+confid
        util.LoadXmlFile('Scripts/ConfTamplates/TransConf2.xml')
        util.ModifyXml("GET","ID",confid)
        util.Send()
        for retry in range(num_retries+1):
            ongoing_parties = util.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            if len(ongoing_parties) == 0:
                break
            
            if (retry == num_retries):
                print "Timeout for Monitor Party list until empty"
                print util.xmlResponse.toprettyxml(encoding="utf-8")
                util.Disconnect()                
                ScriptAbort("There are still" + str(len(ongoing_parties)) + " connected parties")
                
            sys.stdout.write(".")
            sys.stdout.flush()
            util.Send()
            sleep(1)

        print
        print "No Parties in Conference: "+confid
        

#------------------------------------------------------------------------------
    def SetDelayBetweenParties(self, util):

        delayBetweenParticipants = 1
        if(McmsUtilClass.IsProcessUnderValgrind("ConfParty")):
            delayBetweenParticipants = 3
        elif(McmsUtilClass.IsProcessUnderValgrind("EndpointsSim")):
            delayBetweenParticipants = 2
        
        return delayBetweenParticipants
    
 #------------------------------------------------------------------------------
    def SimulationDeleteAllSimParties(self, util, resetSimIpAddress="FALSE"): 
        """Delete All SIM partiesH323 party from simulation.
        
        """
        print "Deleting All SIM parties"
        util.LoadXmlFile("Scripts/PartyTamplates/SimDel323Party.xml")
        if(resetSimIpAddress == "TRUE"):
            util.ModifyXml("H323_PARTY_DEL","PARTY_NAME","ALL_SIM_EPS_AND_RESET_IP")#"ALL_SIM_EPS" indicate the simulation to delete all EPs
        else:
            util.ModifyXml("H323_PARTY_DEL","PARTY_NAME","ALL_SIM_EPS")#"ALL_SIM_EPS" indicate the simulation to delete all EPs
        util.Send()
         
#------------------------------------------------------------------------------
#adding parties with manualy defined capSets
    def SimulationDeleteCapsSets(self, util):  
       """Delete All SIM undefined caps sets from capabilities py."""
       print "Deleting All SIM undefined caps sets"
       for key, value in UndefinedCapSets.items():
           CapSet = str(key)    
           util.SimulationDelCaps(CapSet)   #create the new capset and send it to simulation

##------------------------------------------------------------------------------
    def ConnectParty(self, util, confid, partyName, setProtocol, partyNumber, fileName = "Scripts/PartyTamplates/AddParty1.xml", setMedia = "AUDIO", waitForPartyToConnect = "TRUE", expected_status="Status OK"):
        """Add Party and connect it"
        """
#        print "Connecting " + setProtocol + " Party " + partyName
        util.LoadXmlFile(fileName)
        if(setProtocol == "H323"):
            util.ModifyXml("PARTY", "INTERFACE", "h323")  
        elif(setProtocol == "SIP"):
            util.ModifyXml("PARTY", "INTERFACE", "sip") 
            
        if(setMedia == "AUDIO"):
            util.ModifyXml("PARTY", "CALL_CONTENT", "voice") 
        if(setMedia == "HD_1080"):
            util.ModifyXml("PARTY", "MAX_RESOLUTION", "hd_1080") 

        partyip =  "1.2.3." + str(partyNumber)
        util.ModifyXml("PARTY","IP",partyip)
        util.ModifyXml("PARTY", "NAME", partyName)  
        util.ModifyXml("ADD_PARTY","ID",confid)
        util.Send(expected_status)
        
        if (waitForPartyToConnect == "TRUE"):
            #self.WaitAllOngoingConnected(confid,num_retries)
            if (util.IsProcessUnderValgrind("ConfParty")):
                sleep(2)
            partyid = self.GetPartyId(util, confid, partyName)
            self.WaitPartyConnected(util, confid, partyid)


#------------------------------------------------------------------------------
    def WaitPartyConnected(self, util, confid, partyid, num_retries=30):
        """Monitor conference until the party is 'connected'.
        """
        print "Wait until party is connected",
        util.LoadXmlFile('Scripts/ConfTamplates/TransConf2.xml')
        util.ModifyXml("GET","ID",confid)
        util.Send()
        for retry in range(num_retries+1):
            ongoing_parties = util.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
            ongoing_party_list = util.xmlResponse.getElementsByTagName("ONGOING_PARTY")
            i = 0;
            for party in ongoing_parties:
                if(partyid == ongoing_party_list[i].getElementsByTagName("ID")[0].firstChild.data):
                    status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
                    if status != "connected":
                        if (retry == num_retries):
                            print util.xmlResponse.toprettyxml(encoding="utf-8")
                            util.Disconnect()                
                            ScriptAbort("party is not connected : status = " + status)
                    else:
                        return
                i=i+1
            
            sys.stdout.write(".")
            sys.stdout.flush()
            sleep(1)
            util.Send()
            

