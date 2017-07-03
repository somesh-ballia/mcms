#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_3

#############################################################################
# Test Script which test the Creation of Names in UTF-8 & ASCII
#############################################################################


from McmsConnection import *

###---------------------------------------------------------------------------------------
num_retries=20
num_of_parties=4

###---------------------------------------------------------------------------------------
def CheckConfDisplayName(self,confDisplayName,retires = 20): 
    print "Check conf display name : " + confDisplayName.encode("utf-8")
    bFound = False
    for retry in range(retires+1):
        status = self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        ongoing_conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
        for index in range(len(ongoing_conf_list)):  
            if(confDisplayName == ongoing_conf_list[index].getElementsByTagName("DISPLAY_NAME")[0].firstChild.data):
                print
                bFound = True
                break
        if(bFound):
            break
        if (retry == retires):
            print self.xmlResponse.toprettyxml(encoding="utf-8")
            self.Disconnect()                
            ScriptAbort("Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)
    return confid

###---------------------------------------------------------------------------------------   
def WaitUtfConfCreated(self,confName,retires = 20): 
    """Monitor conferences list until 'confName' is found.
    Returns conference ID.
    
    confName - lookup conf name 
    """
    print "Wait untill Conf \'" + confName.encode("utf-8") + "\' is created...",
    bFound = False
    for retry in range(retires+1):
        status = self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        ongoing_conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
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
            print self.xmlResponse.toprettyxml(encoding="utf-8")
            self.Disconnect()                
            ScriptAbort("Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)
    print "Created Conf, ID:" + str(confid)
    return confid
#------------------------------------------------------------------------------
def SimulationAddH323Party(self, partyName, confName, capSetName = "FULL CAPSET"):
    '''
    Add H323 Party in Simulation.
    The Party will be a dial-in to confName.
    
    partyName - dial-in party name.
    confName - destination conf.   
    
    '''
    print "Adding Sim Undefined H323 Party " + partyName 
    self.LoadXmlFile("Scripts/SimAdd323Party.xml")
    self.ModifyXml("H323_PARTY_ADD","PARTY_NAME",partyName)
    self.ModifyXml("H323_PARTY_ADD","CONF_NAME",confName)
    self.ModifyXml("H323_PARTY_ADD","CAPSET_NAME",capSetName)
    self.Send()

#------------------------------------------------------------------------------
def SimulationConnectH323Party(self, partyName):
    """
    Connect H323 Party defined in Simulation.
    Party must be first defined in simulation!
    
    partyName - dial-in party name to be connected.
    """
    print "Connecting Sim H323 Party " + partyName
    self.LoadXmlFile("Scripts/SimConnect323Party.xml")
    self.ModifyXml("H323_PARTY_CONNECT","PARTY_NAME",partyName)
    self.Send()
    
       
#------------------------------------------------------------------------------
def SimulationAddSipParty(self, partyName, confName, capSetName = "FULL CAPSET"):
    """
    Add SIP Party in Simulation.
    The Party will be a dial-in to confName.
    
    partyName - dial-in party name.
    confName - destination conf.
    """
    print "Adding Sim Undefined SIP Party " + partyName 
    self.LoadXmlFile("Scripts/SimAddSipParty.xml")
    self.ModifyXml("SIP_PARTY_ADD","PARTY_NAME",partyName)
    self.ModifyXml("SIP_PARTY_ADD","CONF_NAME",confName)
    self.ModifyXml("SIP_PARTY_ADD","CAPSET_NAME",capSetName)
    self.Send()
    sleep(1)
    
#------------------------------------------------------------------------------
def SimulationConnectSipParty(self, partyName):
    """
    Connect SIP Party defined in Simulation.
    Party must be first defined in simulation!
    
    partyName - dial-in party name to be connected.
    """
    print "Connecting Sim SIP Party " + partyName
    self.LoadXmlFile("Scripts/SimConnectSipParty.xml")
    self.ModifyXml("SIP_PARTY_CONNECT","PARTY_NAME",partyName)
    self.Send()
       
#------------------------------------------------------------------------------
def AddParty(self,confid, partyname, partyIp, partyFile,expected_status="Status OK"):
    """
    Add a new party.
    
    confid - destination conference.
    partyname - party name.
    partyIp - ip address for new party
    partyFile - xml file which will be used to define the party
    """
    print "Adding Party..." + partyname
    self.LoadXmlFile(partyFile)
    self.ModifyXml("PARTY","NAME",partyname)
    self.ModifyXml("PARTY","IP",partyIp)
    self.ModifyXml("ADD_PARTY","ID",confid)
    self.Send(expected_status)    

#------------------------------------------------------------------------------
def CheckMRName(self,mrName,retires = 20): 
    """
    Monitor meeting rooms list until 'mrName' is found.
    Maybe used also to find EQ.
    Returns MR ID, and MR NID.
    
    mrName - lookup MR name 
    """
    bFound = False
    for retry in range(retires+1):
        status = self.SendXmlFile('Scripts/GetMRList.xml',"Status OK")
        mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        for index in range(len(mr_list)):  
            if(mrName == mr_list[index].getElementsByTagName("NAME")[0].firstChild.data):
                print
                bFound = True
                break
        if(bFound):
            break
        if (retry == retires):
            print self.xmlResponse.toprettyxml(encoding="utf-8")
            self.Disconnect()                
            ScriptAbort("Can not monitor MR:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()

##--------------------------------------- TEST ---------------------------------

connection = McmsConnection()
connection.Connect()

fileName = 'Scripts/CheckUnicodeNames/AddConf.xml'

confAliasName = "Conf12"
unicodeName = u"שלום"

# checking ivr fields - SERVICE_NAME - unicode
#-------------------------------------------------------
IvrService = u"סרוויס"

print "step 01: Adding Ivr Service with unicode name " + IvrService.encode("utf-8") + " ..."
connection.LoadXmlFile('Scripts/CheckUnicodeNames/AddIvrService.xml')
connection.ModifyXml("AV_COMMON","SERVICE_NAME",IvrService)
connection.Send()

sleep(2)


# Add Profile with unicode name
#-----------------------------------
profileName = u"פרופיל"

confDisplayName = "Conf13"
print "step 02: Adding profile with unicode name " + profileName.encode("utf-8") + " ..."
connection.LoadXmlFile('Scripts/CheckUnicodeNames/AddProfile.xml')
connection.ModifyXml("RESERVATION","DISPLAY_NAME", profileName)
connection.ModifyXml("RESERVATION","AV_MSG", IvrService)
connection.Send()
ProfId = connection.GetTextUnder("RESERVATION","ID")
#print "Profile, named: " + profileName.encode("utf-8") + " ,ID = " + ProfId + ", is added"
sleep(2)
#ProfId = connection.AddProfile(profileName, "Scripts/EncryConf/CreateNewProfile.xml")

#create the target Conf and wait untill it connected

connection.CreateConfFromProfile(confDisplayName, ProfId, 'Scripts/EncryConf/CreateNewConf.xml')
confId  = connection.WaitConfCreated(confDisplayName,num_retries)
sleep(2)


# Add Factory with unicode name
#-----------------------------------
print "step 03: Adding SIP Factory with unicode name "
#Adding a new Factory
factoryName = u"שלום"
print "Adding a new Factory:" + factoryName.encode("utf-8") + " ..."
connection.LoadXmlFile('Scripts/AddDeleteFactory/AddFactory.xml')
connection.ModifyXml("RESERVATION","NAME",factoryName)
connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(ProfId))
connection.Send()
sleep(2)

# Add MR with unicode name
#----------------------------------------------------------------
print "Starting test TestAwakeMrWithUndefParty"

#Adding a new Meeting room
mrName = u"ролзйц"
mrRegName = "mr1"
print "step 04: Adding MR with unicode dispaly name " + mrName.encode("utf-8")  + " and routing name " + mrRegName
connection.LoadXmlFile('Scripts/CheckUnicodeNames/AddMr.xml')
connection.ModifyXml("RESERVATION","DISPLAY_NAME",mrName)
connection.ModifyXml("RESERVATION","NAME",mrRegName)
connection.ModifyXml("RESERVATION","NUMERIC_ID",'2340')
connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(ProfId))
connection.Send()


#Get the Mr numeric id when mr was already added
mrNumericId = ""
mrId = ""
mrId,mrNumericId = connection.WaitMRCreated(mrRegName,num_retries)

# Create a new undefined party and add it to the EPSim
print "step 05: Awake MR with unicode display name " + mrName.encode("utf-8")
partyName = "Party1"
SimulationAddH323Party(connection,partyName, mrRegName) 
SimulationConnectH323Party(connection,partyName)

#Wait untill Meeting room is awake
numOfParties = 1
mrConfId = connection.WaitUntillEQorMRAwakes(mrRegName,numOfParties,num_retries)
CheckMRName(connection,mrRegName,20)
sleep(2)

# Add MR with unicode name - checking MR names
#----------------------------------------------
#Adding a new Meeting room
mrDisplayName = u"بيتقف2"

print "step 06: Adding MR with unicode dispaly name " + mrDisplayName.encode("utf-8")  + " and empty routing name"
connection.LoadXmlFile('Scripts/CheckUnicodeNames/AddMr.xml')
connection.ModifyXml("RESERVATION","DISPLAY_NAME",mrDisplayName)
connection.ModifyXml("RESERVATION","NAME","")
connection.ModifyXml("RESERVATION","NUMERIC_ID",'2341')
connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(ProfId))
connection.Send()
CheckMRName(connection,'2341',20)
sleep(2)

# checking conf fields - contact_info and billing_data - unicode
#-------------------------------------------------------
confRegistrationName = "ABC"
confDisplayName = "Conf1"

print "step 07: Adding Conf with dispaly name " + confDisplayName  + " and routing name " + confRegistrationName
print "set unicode contact info and billing data " + unicodeName.encode("utf-8")
connection.LoadXmlFile(fileName)
connection.ModifyXml("RESERVATION","NAME",confRegistrationName)
connection.ModifyXml("RESERVATION","DISPLAY_NAME",confDisplayName)
connection.ModifyXml("RESERVATION","NUMERIC_ID",'1234')
connection.ModifyXml("RESERVATION","BILLING_DATA",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_2",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_3",unicodeName)
#connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_4",unicodeName)

#connection.SendXmlFile('Scripts/CheckUnicodeNames/AddConf.xml',"Status OK")
connection.Send()

confid = WaitUtfConfCreated(connection,confRegistrationName,num_retries)   
CheckConfDisplayName(connection,confDisplayName,num_retries)  
sleep(2)


# confRegistrationName and confDisplayName - are both unicode
#-------------------------------------------------------------
confRegistrationName = u"ועידן"
confDisplayName = u"ועידג"

print "step 08: Adding Conf with unicode dispaly name " + confDisplayName.encode("utf-8")  + " and unicode routing name " + confRegistrationName.encode("utf-8")
print "test conf rejected"
connection.LoadXmlFile(fileName)
connection.ModifyXml("RESERVATION","NAME",confRegistrationName)
connection.ModifyXml("RESERVATION","DISPLAY_NAME",confDisplayName)
connection.ModifyXml("RESERVATION","NUMERIC_ID",'1235')
#connection.SendXmlFile('Scripts/CheckUnicodeNames/AddConf.xml',"Status OK")
connection.Send("Invalid character in conference routing name")

#confid = WaitUtfConfCreated(connection,confRegistrationName,num_retries)     
sleep(2)

# checking party fields - contact_info and billing_data - unicode
#-----------------------------------------------------------------
confRegistrationName = "ABC"
confDisplayName = "Conf1"


#Add Dial In H323 Party
# 1st party
connection.LoadXmlFile("Scripts/CheckUnicodeNames/AddDialInH323Party.xml")
partyname = "Party"+str(1)
partyip =  "124.124.0." + str(1)
print "step 09: Party with unicode name " + (u"1משתתף").encode("utf-8")
print "set party unicode contact info " + unicodeName.encode("utf-8")
print "Adding H323 Party..."
connection.ModifyXml("PARTY","NAME",u"1משתתף")
connection.ModifyXml("PARTY","IP",partyip)
connection.ModifyXml("ADD_PARTY","ID",confid)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_2",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_3",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_4",unicodeName)
connection.Send()

sleep(2)

#-----------------------------------------------------------------
# 2nd party
print "step 10: Party with unicode ip alias " + (u"2משתתף").encode("utf-8")
print "party should connect, alias should be empty"

print 'adding  dial out H323 video call'     
partyname = u"2рошпа"
partyip =  "1.2.3." + str(2)
print "Adding Party..." + partyname.encode("utf-8")
connection.LoadXmlFile("Scripts/CheckUnicodeNames/AddDialOutH323Party.xml")
connection.ModifyXml("PARTY","NAME",u"2рошпа")
connection.ModifyXml("PARTY","IP",partyip)
connection.ModifyXml("ALIAS","NAME",u"2משתתף")
connection.ModifyXml("ADD_PARTY","ID",confid)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_2",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_3",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_4",unicodeName)
connection.Send()
##connection.Send("STATUS_NODE_VALUE_NOT_ASCII")    
sleep(2) 


# adding  dial out Sip video call
print 'adding  dial out Sip video call'

partyname = u"3بلانم"
partyip =  "1.2.3." + str(3)
#partySipAdd = partyname + '@' + partyip

partySipAdd = u"3משתתף"
print "step 11: Party with unicode SIP address " + partySipAdd.encode("utf-8")
print "party shoul rejected"
connection.LoadXmlFile("Scripts/CheckUnicodeNames/AddDialOutSipParty.xml")
connection.ModifyXml("PARTY","NAME",partyname)
connection.ModifyXml("PARTY","IP",partyip)
connection.ModifyXml("ADD_PARTY","ID",confid)
connection.ModifyXml("PARTY","SIP_ADDRESS",partySipAdd)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_2",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_3",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_4",unicodeName)
connection.Send()
## connection.Send("STATUS_NODE_VALUE_NOT_ASCII")    
sleep(2)    

connection.WaitAllPartiesWereAdded(confid,3,num_retries)


# Add H323 party to EP Sim and connect him
#partyname = u"משתתף"

partyname = u'\u05de\u05e9\u05ea\u05ea\u05e3'
print "step 11: Adding more participants with other unicode names " + partyname.encode("utf-8") 
connection.LoadXmlFile("Scripts/SimAdd323Party.xml")
connection.ModifyXml("H323_PARTY_ADD","PARTY_NAME",partyname)
connection.ModifyXml("H323_PARTY_ADD","CONF_NAME",confRegistrationName)
connection.ModifyXml("H323_PARTY_ADD","CAPSET_NAME","FULL CAPSET")
connection.Send()

print "Connecting Sim H323 Party " + partyname.encode("utf-8")
connection.LoadXmlFile("Scripts/SimConnect323Party.xml")
connection.ModifyXml("H323_PARTY_CONNECT","PARTY_NAME",partyname)
connection.Send()

# Add Sip party to EP Sim and connect him
partyname = u"משתתף6"
print "Adding Sim Undefined SIP Party " + partyname.encode("utf-8") 
connection.LoadXmlFile("Scripts/SimAddSipParty.xml")
connection.ModifyXml("SIP_PARTY_ADD","PARTY_NAME",partyname)
connection.ModifyXml("SIP_PARTY_ADD","CONF_NAME",confRegistrationName)
connection.ModifyXml("SIP_PARTY_ADD","CAPSET_NAME","FULL CAPSET")
connection.Send()

print "Connecting Sim SIP Party " + partyname.encode("utf-8")
connection.LoadXmlFile("Scripts/SimConnectSipParty.xml")
connection.ModifyXml("SIP_PARTY_CONNECT","PARTY_NAME",partyname)
connection.Send()
  
#c.WaitAllOngoingConnected(confid,num_retries)

sleep(10)


print 'adding  dial out H323 video call'     
partyname = "party12"
print "step 12: Test party visual name: add party " + partyname
partyip =  "1.2.3." + str(12)
print "Adding Party..." + partyname
connection.LoadXmlFile("Scripts/CheckUnicodeNames/AddDialOutH323Party.xml")
connection.ModifyXml("PARTY","NAME",partyname)
connection.ModifyXml("PARTY","IP",partyip)
connection.ModifyXml("ALIAS","NAME",partyname)
connection.ModifyXml("ADD_PARTY","ID",confid)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_2",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_3",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_4",unicodeName)
connection.Send()    
sleep(2) 

connection.WaitAllPartiesWereAdded(confid,6,num_retries)

partyid = connection.GetPartyId(confid, partyname)
##connection.WaitAllOngoingConnected(confid,num_retries)  


# Set party name to unicode visual name
visualName = u"ρτλκψ"
print "Set party visual name: " + visualName.encode("utf-8")
#visualName = u"ועידן"
print "Set party name to : " + visualName.encode("utf-8")
connection.LoadXmlFile("Scripts/CheckUnicodeNames/SetVisualName.xml")
connection.ModifyXml("SET_PARTY_VISUAL_NAME","ID",confid)
connection.ModifyXml("SET_PARTY_VISUAL_NAME","PARTY_ID",partyid)
connection.ModifyXml("SET_PARTY_VISUAL_NAME","NAME",visualName)
connection.Send()

sleep(20)
print 'adding  dial out H323 video call'     
partyname = u"апзщй"
partyAlias = u"йцукен"
partyip =  "1.2.3." + str(13)
print "Adding Party..." + partyname.encode("utf-8")
connection.LoadXmlFile("Scripts/CheckUnicodeNames/AddDialOutH323Party.xml")
connection.ModifyXml("PARTY","NAME",partyname)
connection.ModifyXml("PARTY","IP",partyip)
connection.ModifyXml("ALIAS","NAME",partyAlias)
connection.ModifyXml("ADD_PARTY","ID",confid)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_2",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_3",unicodeName)
connection.ModifyXml("CONTACT_INFO_LIST","CONTACT_INFO_4",unicodeName)
connection.Send()    
sleep(2) 


# Check if all parties were added and save their IDs
party_id_list = []
connection.LoadXmlFile('Scripts/CreateCPConfWith4DialInParticipants/TransConf2.xml')
connection.ModifyXml("GET","ID",confid)
connection.Send()
ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
num_of_parties = (len(ongoing_party_list))
print "num of parties: " + str(num_of_parties)
for index in range(num_of_parties):    
    party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)
    
# delete all parties  
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    print "delete party: " + partyname    
    connection.DeleteParty(confid,party_id_list[x])
    sleep(1)
    
sleep(2)    

connection.DeleteAllConf()
#connection.DeleteConf(confid)
connection.WaitAllConfEnd()
connection.Disconnect()
    
