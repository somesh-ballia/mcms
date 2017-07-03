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
from Party import *

class H323PartyUtils(Party):
    """This is a H323 Party utility class.
    It enables all sorts of H323 Party actions.
    AddH323DefineCapSetParties
    AddH323UndefineCapSetParties
    SimulationAddH323Party
    SimulationConnectH323Party
    SimulationConnectH323PartyExpStatus
    SimulationDeleteH323Party
    """
#------------------------------------------------------------------------------
#adding parties with capsets that are already defined by EP-SIM
    def AddH323DefineCapSetParties(self, util, confName, confid, PartyNameMold = "PartyDef"):  
        "Add H323 parties with defined caps set"
        numParties = 0
        for key in DefinedCapSets.keys():
#        for key in range(1):
            numParties = numParties + 1
            partyname = PartyNameMold+str(numParties)
            CapSet = str(key)
            self.SimulationAddH323Party(util, partyname, confName, CapSet)
            self.SimulationConnectH323Party(util, partyname)

            sleepDelay = 0
            if (util.IsProcessUnderValgrind("ConfParty")):
                sleepDelay = 3
            sleep(sleepDelay)
        self.WaitAllPartiesWereAdded(util, confid, numParties, 20)
        self.WaitAllOngoingConnected(util, confid)
        self.WaitAllOngoingNotInIVR(util, confid)
        sleep(1) 
        return numParties


#------------------------------------------------------------------------------
#adding parties with manualy defined capSets
    def AddH323UndefineCapSetParties(self, util, confName, confid, numParties, PartyNameMold = "PartyUndef"):  
        "Add H323 parties with manualy defined cap Sets"
        for key, value in UndefinedCapSets.items():
            numParties = numParties + 1 
            partyname = PartyNameMold+str(numParties)
            CapSet = str(key)    
            util.SimulationSetCaps(CapSet, value)   #create the new capset and send it to simulation
            self.SimulationAddH323Party(util, partyname, confName, CapSet)    #add the party
            self.SimulationConnectH323Party(util, partyname)                 #connect the party

            sleepDelay = 0
            if (util.IsProcessUnderValgrind("ConfParty")):
                sleepDelay = 1
            sleep(sleepDelay)
        self.WaitAllPartiesWereAdded(util, confid, numParties, 20)
        self.WaitAllOngoingConnected(util, confid)
        self.WaitAllOngoingNotInIVR(util, confid)
        return numParties

#------------------------------------------------------------------------------
    def SimulationAddH323Party(self, util, partyName, confName, capSetName="FULL CAPSET", ipVer=0, manuName="EndpointsSim", sourcePartyAliasName="HDX"):
        """Add H323 Party in Simulation.
        The Party will be a dial-in to confName.
        
        partyName - dial-in party name.
        confName - destination conf.
        """
        print "Adding Sim H323 Party " + partyName 
        util.LoadXmlFile("Scripts/PartyTamplates/SimAdd323Party.xml")
        util.ModifyXml("H323_PARTY_ADD","PARTY_NAME",partyName)
        util.ModifyXml("H323_PARTY_ADD","CONF_NAME",confName)
        util.ModifyXml("H323_PARTY_ADD","CAPSET_NAME",capSetName)
        util.ModifyXml("H323_PARTY_ADD","IP_VER",ipVer)
        util.ModifyXml("H323_PARTY_ADD","MANUFUCTURER_NAME",manuName)
        util.ModifyXml("H323_PARTY_ADD","SOURCE_PARTY_ALIAS",sourcePartyAliasName)
        util.Send()
        
 #------------------------------------------------------------------------------
    def SimulationDeleteH323Party(self, partyName): 
        """Delete H323 party from simulation.
        
        partyName - dial-in party name.
        """
        print "Deleting SIM H323 party "+partyName+"..."
        self.LoadXmlFile("Scripts/PartyTamplates/SimDel323Party.xml")
        self.ModifyXml("H323_PARTY_DEL","PARTY_NAME",partyName)
        self.Send()
        
#------------------------------------------------------------------------------
    def SimulationConnectH323Party(self, util, partyName):
        """Connect H323 Party defined in Simulation.
        Party must be first defined in simulation!
        
        partyName - dial-in party name to be connected.
        """
        print "Connecting Sim H323 Party " + partyName
        util.LoadXmlFile("Scripts/PartyTamplates/SimConnect323Party.xml")
        util.ModifyXml("H323_PARTY_CONNECT","PARTY_NAME",partyName)
        util.Send()

##------------------------------------------------------------------------------
    def SimulationConnectH323PartyExpStatus(self, util, partyName, expected_status="Status OK"):
        """Connect H323 Party defined in Simulation.
        Party must be first defined in simulation!
        
        partyName - dial-in party name to be connected.
        """
        print "Connecting Sim H323 Party " + partyName
        util.LoadXmlFile("Scripts/PartyTamplates/SimConnect323Party.xml")
        util.ModifyXml("H323_PARTY_CONNECT","PARTY_NAME",partyName)
        util.Send(expected_status)
        
        
##------------------------------------------------------------------------------
    def ConnectH323Parties(self, util, confid, numberOfParties, partyName, setMedia = "HD_1080", fileName = "Scripts/PartyTamplates/AddParty1.xml",expected_status="Status OK"):
        """Add H323 Parties"
        
        """
        for x in range(numberOfParties):
            party_name = partyName + str(x)
            print "Connecting H323 Party " + party_name
            self.ConnectParty(util, confid, party_name, "H323", x, fileName, setMedia, "TRUE")

