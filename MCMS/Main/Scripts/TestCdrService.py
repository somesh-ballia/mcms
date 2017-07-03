#!/mcms/python/bin/python

# #############################################################################
# Change configuration of CDR Service
#
# Date: 25.11.14
# By  : Ofir Nissel
#
############################################################################
#
#

from McmsConnection import *
from PartyUtils.H323PartyUtils import *
import re

expected_status="Status OK"

def SetCfg(isRemoteCdrServiceEnabled,cdrServiceIp,cdrServicePort,cdrServiceUsr,cdrServicePwd):
    c = McmsConnection()
    c.Connect()

    c.LoadXmlFile('Scripts/CdrServiceScripts/SetCdrServiceConfigParms.xml')

    c.ModifyXml("CDR_SETTINGS","IS_REMOTE_CDR_SERVICE",isRemoteCdrServiceEnabled)
    c.ModifyXml("CDR_SETTINGS","CDR_SERVICE_IP",cdrServiceIp)
    c.ModifyXml("CDR_SETTINGS","CDR_SERVICE_PORT",cdrServicePort)
    c.ModifyXml("CDR_SETTINGS","CDR_SERVICE_USER",cdrServiceUsr)
    c.ModifyXml("CDR_SETTINGS","CDR_SERVICE_PASSWORD",cdrServicePwd)
    c.Send(expected_status)
    print c.xmlResponse.toprettyxml()


    print "Test Finished successfull , CDR SERVICE Configured!!!"  
    c.Disconnect()

def AddParty():
#add parties
    c = McmsConnection()
    c.Connect()

    H323PartyUtilsClass = H323PartyUtils() # H323Party utils util class

    confname = "Conf1"
    c.CreateConf(confname, 'Scripts/CheckMultiTypesOfCalls/AddConf.xml')
    confid = c.WaitConfCreated(confname,1)    

#Add Dial In H323 Party
    c.LoadXmlFile("Scripts/CheckMultiTypesOfCalls/AddDialInH323Party.xml")
    partyname = "Party"+str(1)
    partyip =  "123.123.0." + str(1)
    print "Adding H323 Party..."
    c.ModifyXml("PARTY","NAME",partyname)
    c.ModifyXml("ALIAS","NAME","")
    c.ModifyXml("PARTY","IP",partyip)
    c.ModifyXml("ADD_PARTY","ID",confid)
    c.Send()
# Add H323 party to EP Sim and connect it
    partyname = "Party"+str(1)
    c.SimulationAddH323Party(partyname, confname)
    c.SimulationConnectH323Party(partyname)
#Add Dial In SIP Party
    partyname = "Party"+str(2)
    partyip =  "123.123.0." + str(2)
    partySipAdd = partyname + '@' + partyip
    c.AddSIPParty(confid, partyname, partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialInSipParty.xml")	
# Add Sip party to EP Sim and connect it
#    partyname = "Party"+str(2)
    c.SimulationAddSipParty(partyname, confname)
    c.SimulationConnectSipParty(partyname)
    sleep(2)
    c.WaitAllOngoingConnected(confid,1)
    c.Disconnect()
def TestSetCfg():
    print
    print "Start CDR SERVICE CONFIGURATION test"
    print
#register to cdr server
    print "set enabled = true "
    SetCfg("true","10.226.118.226","8443","dev2dev","dev2dev")
    print "End of connect CDR SERVICE CONFIGURATION test"
    sleep(5)

#add parties
    AddParty()
    sleep(5)
#disconnect cdr server
    SetCfg("false","10.226.118.226","8443","dev2dev","dev2dev")
    command = "Scripts/McuCmd.sh @readcdrerrors CDR"
    ps     = subprocess.Popen(command , shell=True, stdout=subprocess.PIPE)
    output = ps.stdout.read()
    ps.stdout.close()
    ps.wait()

    print "number of errors: " + output[17]
    print "End of disconnect CDR SERVICE CONFIGURATION test"
    print

TestSetCfg()

