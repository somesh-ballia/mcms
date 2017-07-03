#!/mcms/python/bin/python

#############################################################################
# 
#
# Date: 05/03/06
# By  : Ron S.

#############################################################################

#-LONG_SCRIPT_TYPE

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=CSApi Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer Configurator BackupRestore McuMngr CSMngr




from McmsConnection import *
from PartyUtils.H323PartyUtils import *

###---------------------------------------------------------------------------------------
num_retries=60
num_of_parties=6

##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

H323PartyUtilsClass = H323PartyUtils() # H323Party utils util class
H323PartyUtilsClass.SimulationDeleteAllSimParties(c, "TRUE")

confname = "Conf1"
c.CreateConf(confname, 'Scripts/CheckMultiTypesOfCalls/AddConf.xml')
confid = c.WaitConfCreated(confname,num_retries)    

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

#Add Dial In SIP Party
partyname = "Party"+str(2)
partyip =  "123.123.0." + str(2)
partySipAdd = partyname + '@' + partyip
c.AddSIPParty(confid, partyname, partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialInSipParty.xml")
#c.WaitAllPartiesWereAdded(confid,2,num_retries)

# Add H323 party to EP Sim and connect it
partyname = "Party"+str(1)
c.SimulationAddH323Party(partyname, confname)
c.SimulationConnectH323Party(partyname)

sleep(2)
# Add Sip party to EP Sim and connect it
partyname = "Party"+str(2)
c.SimulationAddSipParty(partyname, confname)
c.SimulationConnectSipParty(partyname)
  
c.WaitAllOngoingConnected(confid,num_retries)

sleep(2)

# adding 1 dial in H323 undefined video call
# Add H323 undefined party to EP Sim and connect it
partyname = "Party"+str(3)
c.SimulationAddH323Party(partyname, confname)
c.SimulationConnectH323Party(partyname)

sleep(2)

# adding 1 dial in SIP undefined video call
# Add Sip party to EP Sim and connect it
partyname = "Party"+str(4)
c.SimulationAddSipParty(partyname, confname)
c.SimulationConnectSipParty(partyname)

sleep(2)
  
# Add undefined H323 audio only call to EP Sim and connect it
partyname = "Party"+str(5)
print "Adding Sim H323 Party "+partyname+"..."
c.SimulationAddH323Party(partyname, confname, "AudioOnly")
c.SimulationConnectH323Party(partyname)

sleep(2)

# Add undefined SIP audio only call to EP Sim and connect it
partyname = "Party"+str(6)
c.SimulationAddSipParty(partyname, confname, "AudioOnly")
c.SimulationConnectSipParty(partyname)
  
#c.WaitAllOngoingConnected(confid,num_retries)

sleep(2)

c.WaitAllPartiesWereAdded(confid,6,num_retries)
c.WaitAllOngoingConnected(confid,num_retries)
party_id_list = []
c.LoadXmlFile('Scripts/CreateCPConfWith4DialInParticipants/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()
ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
if len(ongoing_party_list) < 6:
    sys.exit("some parties are lost...")
for index in range(num_of_parties):    
    party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)


#adding 1 rejected H323 incoming setup.
# Add H323 party to EP Sim and connect it
partyname = "Party"+str(7)
confname2 = "conf2"
c.SimulationAddH323Party(partyname, confname2)
print "Connecting Sim H323 Party "+partyname+"..."
c.LoadXmlFile("Scripts/CheckMultiTypesOfCalls/SimConnectH323Party.xml")
c.ModifyXml("H323_PARTY_CONNECT","PARTY_NAME",partyname)
c.ModifyXml("H323_PARTY_CONNECT","CONF_NAME",confname2)
c.Send()

#adding 1 rejected SIP incoming invite
# Add Sip party to EP Sim and connect it
partyname = "Party"+str(8)
c.SimulationAddSipParty(partyname, confname2)
print "Connecting Sim Sip Party "+partyname+"..."
c.LoadXmlFile("Scripts/CheckMultiTypesOfCalls/SimConnectSipParty.xml")
c.ModifyXml("SIP_PARTY_CONNECT","PARTY_NAME",partyname)
c.ModifyXml("SIP_PARTY_CONNECT","CONF_NAME",confname2)
c.Send()
  
sleep(2)

c.WaitAllPartiesWereAdded(confid,6,num_retries)
c.WaitAllOngoingConnected(confid,num_retries)

# Check if all parties were added and save their IDs
party_id_list = []
c.LoadXmlFile('Scripts/CreateCPConfWith4DialInParticipants/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()
ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
if len(ongoing_party_list) > num_of_parties:
    sys.exit("more parties than should be...")
for index in range(num_of_parties):    
    party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)
    
# delete all parties  
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    print "delete party: " + partyname    
    c.DeleteParty(confid,party_id_list[x])
    sleep(1)
    
sleep(2)    

H323PartyUtilsClass.SimulationDeleteAllSimParties(c, "TRUE")
c.DeleteAllConf(2);
c.WaitAllConfEnd()
c.Disconnect()


##-----------------------------------------------------------------------------------------------

