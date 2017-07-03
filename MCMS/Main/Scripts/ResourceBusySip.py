#!/mcms/python/bin/python

from McmsConnection import *
import os

#SEND_SIP_BUSY_UPONRESOURCE_THRESHOLD="YES"
#export CLEAN_CFG=NO

print "#############################################################################"
print "###                     Script ResourceBusySip.py                         ###"
print "#############################################################################"

#############################################################################
# Script For Testing SIP dial-in busy (according to port gauge %).
#############################################################################
#-----------------------------------------------------------------------

def SetPortGauge(connection, portGauge):
   connection.LoadXmlFile("Scripts/ResourcePortGauge.xml")
   connection.ModifyXml("SET_PORT_GAUGE","PORT_GAUGE_VALUE", portGauge)
   connection.Send()

#-----------------------------------------------------------------------

def GetPortGaugeFromRsrcRep(connection):
    connection.SendXmlFile("Scripts/Resource2Undef3RealParties/ResourceMonitorCarmel.xml")
    portGauge = connection.GetTextUnder("RSRC_REPORT_RMX_LIST","PORT_GAUGE_VALUE",0)
    print "GetPortGaugeFromRsrcRep portGauge value= " + portGauge
    return portGauge
         
#------------------------------------------------------------------------------ 
 
connection = McmsConnection()
connection.Connect()

confName = "confTest"
connection.CreateConf (confName)
confid  = connection.WaitConfCreated(confName,1)

portGauge1="10"
SetPortGauge(connection, portGauge1)
portGauge = GetPortGaugeFromRsrcRep(connection)
print "portGauge value from resource report is " + portGauge
sleep(3)

partyname1 = "SIPdialInParty1"
connection.SimulationAddSipParty(partyname1, confName,"AudioOnly")
connection.SimulationConnectSipParty(partyname1)
connection.WaitAllOngoingConnected(confid)
connection.WaitAllOngoingNotInIVR(confid)
            
portGauge1=2
SetPortGauge(connection, portGauge1)
#pass the % with lot parties -- its not pass. gives the status...
portGauge = GetPortGaugeFromRsrcRep(connection)
print "portGauge value from resource report is " + portGauge

partyname2 = "SIPdialInParty2"
connection.SimulationAddSipParty(partyname2, confName,"AudioOnly") 
connection.SimulationConnectSipParty(partyname2)
connection.WaitAllOngoingConnected(confid)
connection.WaitAllOngoingNotInIVR(confid)

partyname3 = "SIPdialInParty3"
connection.SimulationAddSipParty(partyname3, confName,"AudioOnly")
connection.SimulationConnectSipParty(partyname3)
connection.WaitAllOngoingConnected(confid)
connection.WaitAllOngoingNotInIVR(confid)

sleep(3)
connection.DeleteConf(confid)
sleep(2)
connection.Disconnect()

