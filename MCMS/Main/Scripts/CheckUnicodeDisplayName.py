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
#    return confid

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
        
        


##--------------------------------------- TEST ---------------------------------

connection = McmsConnection()
connection.Connect()

fileName = 'Scripts/CheckUnicodeNames/AddConf.xml'

confAliasName = "Conf12"
confName = "שלום"

# confRegistrationName -ascii != confDisplayName -ascii
#-------------------------------------------------------
confRegistrationName = "ABC"
confDisplayName = "Conf1"

print "Adding Conf " + confDisplayName + " ..."
connection.LoadXmlFile(fileName)
connection.ModifyXml("RESERVATION","NAME",confRegistrationName)
connection.ModifyXml("RESERVATION","DISPLAY_NAME",confDisplayName)
connection.ModifyXml("RESERVATION","NUMERIC_ID",'1234')
#connection.SendXmlFile('Scripts/CheckUnicodeNames/AddConf.xml',"Status OK")
connection.Send()

confid1 = WaitUtfConfCreated(connection,confRegistrationName,num_retries)   
CheckConfDisplayName(connection,confDisplayName,num_retries)  
sleep(2)


# confRegistrationName and confDisplayName - are both empty
#----------------------------------------------------------
confRegistrationName = ""
confDisplayName = ""

print "Adding Conf " + confDisplayName + " ..."
connection.LoadXmlFile(fileName)
connection.ModifyXml("RESERVATION","NAME",confRegistrationName)
connection.ModifyXml("RESERVATION","DISPLAY_NAME",confDisplayName)
connection.ModifyXml("RESERVATION","NUMERIC_ID",'1235')
#connection.SendXmlFile('Scripts/CheckUnicodeNames/AddConf.xml',"Status OK")
connection.Send("Field value is too short")

#confid = WaitUtfConfCreated(connection,confRegistrationName,num_retries)     
sleep(2)

# confRegistrationName - ascii and confDisplayName - unicode
#----------------------------------------------------------
confRegistrationName = "Conf2"
confDisplayName = u"ועידן"

print "Adding Conf " + confDisplayName.encode("utf-8") + " ..."
connection.LoadXmlFile(fileName)
connection.ModifyXml("RESERVATION","NAME",confRegistrationName)
connection.ModifyXml("RESERVATION","DISPLAY_NAME",u"ועידן")
connection.ModifyXml("RESERVATION","NUMERIC_ID",'1236')
#connection.SendXmlFile('Scripts/CheckUnicodeNames/AddConf.xml',"Status OK")
connection.Send()

confid2 = WaitUtfConfCreated(connection,confRegistrationName,num_retries)    
CheckConfDisplayName(connection,u"ועידן",num_retries) 
sleep(2)

# confRegistrationName - ascii and confDisplayName - empty
#----------------------------------------------------------
confRegistrationName = "Conf3"
confDisplayName = ""

print "Adding Conf " + confDisplayName + " ..."
connection.LoadXmlFile(fileName)
connection.ModifyXml("RESERVATION","NAME",confRegistrationName)
connection.ModifyXml("RESERVATION","DISPLAY_NAME",confDisplayName)
connection.ModifyXml("RESERVATION","NUMERIC_ID",'1237')
#connection.SendXmlFile('Scripts/CheckUnicodeNames/AddConf.xml',"Status OK")
connection.Send()

confid3 = WaitUtfConfCreated(connection,confRegistrationName,num_retries) 
CheckConfDisplayName(connection,confRegistrationName,num_retries)     
sleep(2)


# confRegistrationName - empty and confDisplayName - ascii
#----------------------------------------------------------
confRegistrationName = ""
confDisplayName = "Conf4"

print "Adding Conf " + confDisplayName + " ..."
connection.LoadXmlFile(fileName)
connection.ModifyXml("RESERVATION","NAME",confRegistrationName)
connection.ModifyXml("RESERVATION","DISPLAY_NAME",confDisplayName)
connection.ModifyXml("RESERVATION","NUMERIC_ID",'1238')
#connection.SendXmlFile('Scripts/CheckUnicodeNames/AddConf.xml',"Status OK")
connection.Send()

confid4 = WaitUtfConfCreated(connection,confDisplayName,num_retries) 
CheckConfDisplayName(connection,confDisplayName,num_retries)     
sleep(2)


# confRegistrationName - empty and confDisplayName - unicode
#----------------------------------------------------------
confRegistrationName = ""
confDisplayName = u"3ועידה"
numericID = '2222'
print "Adding Conf " + confDisplayName.encode("utf-8") + " ..."
connection.LoadXmlFile(fileName)
connection.ModifyXml("RESERVATION","NAME",confRegistrationName)
connection.ModifyXml("RESERVATION","DISPLAY_NAME",confDisplayName)
connection.ModifyXml("RESERVATION","NUMERIC_ID",numericID)
#connection.SendXmlFile('Scripts/CheckUnicodeNames/AddConf.xml',"Status OK")
connection.Send()

confid5 = WaitUtfConfCreated(connection,numericID,num_retries) 
CheckConfDisplayName(connection,confDisplayName,num_retries)     
sleep(2)

# confRegistrationName - unicode and confDisplayName - ascii
#----------------------------------------------------------
#confRegistrationName = u"4ועידה"
#confDisplayName = "Conf6"
#
#print "Adding Conf " + confDisplayName + " ..."
#connection.LoadXmlFile(fileName)
#connection.ModifyXml("RESERVATION","NAME",confRegistrationName)
#connection.ModifyXml("RESERVATION","DISPLAY_NAME",confDisplayName)
#connection.ModifyXml("RESERVATION","NUMERIC_ID",'1240')
##connection.SendXmlFile('Scripts/CheckUnicodeNames/AddConf.xml',"Status OK")
#connection.Send("Invalid character in conference Routing Name")

#confid = WaitUtfConfCreated(connection,numericID,num_retries) 
#CheckConfDisplayName(connection,confDisplayName,num_retries)     
#sleep(2)

# confRegistrationName - unicode and confDisplayName - ascii
#----------------------------------------------------------
confRegistrationName = ""
confDisplayName = '2222'

print "Adding Conf " + confDisplayName + " ..."
connection.LoadXmlFile(fileName)
connection.ModifyXml("RESERVATION","NAME",confRegistrationName)
connection.ModifyXml("RESERVATION","DISPLAY_NAME",confDisplayName)
connection.ModifyXml("RESERVATION","NUMERIC_ID",'1242')
connection.Send("Display name already exists")

#confid = WaitUtfConfCreated(connection,numericID,num_retries) 
#CheckConfDisplayName(connection,confDisplayName,num_retries)     
sleep(3)
#----------------------------------------------------------


'''
connection.ModifyXml("RESERVATION","NAME",u"ועידן")
connection.ModifyXml("RESERVATION","DISPLAY_NAME",u"ועידן")
#connection.SendXmlFile('Scripts/CheckUnicodeNames/AddConf.xml',"Status OK")
connection.Send("Invalid character in conference name")
sleep(5)
#connection.CreateConf(confname, 'Scripts/CheckMultiTypesOfCalls/AddConf.xml')
#confid = WaitUtfConfCreated(connection,u"ועידן",num_retries)   
'''
print "Adding Conf " + confAliasName + " ..."
connection.LoadXmlFile(fileName)
connection.ModifyXml("RESERVATION","NAME",confAliasName)
connection.ModifyXml("RESERVATION","DISPLAY_NAME","")
connection.ModifyXml("RESERVATION","NUMERIC_ID",'1241')
#connection.SendXmlFile('Scripts/CheckUnicodeNames/AddConf.xml',"Status OK")
connection.Send()

confid6 = WaitUtfConfCreated(connection,confAliasName,num_retries)     
sleep(2)
confName = "ועידן"
#Add Dial In H323 Party
connection.LoadXmlFile("Scripts/CheckMultiTypesOfCalls/AddDialInH323Party.xml")
partyname = "Party"+str(1)
partyip =  "123.123.0." + str(1)
print "Adding H323 Party..."
connection.ModifyXml("PARTY","NAME",partyname)
connection.ModifyXml("PARTY","IP",partyip)
connection.ModifyXml("ADD_PARTY","ID",confid6)
connection.Send()

'''
#Add Dial In H323 Party
partyname = "Party"+str(1)
partyip =  "123.123.0." + str(1)
connection.AddParty(confid, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialInH323Party.xml")
'''
#Add Dial In SIP Party
partyname = "Party"+str(2)
partyip =  "123.123.0." + str(2)
partySipAdd = partyname + '@' + partyip
connection.AddSIPParty(confid6, partyname, partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialInSipParty.xml")
#c.WaitAllPartiesWereAdded(confid,2,num_retries)
sleep(2)

print 'adding  dial out H323 video call'     
partyname = "Party" + str(3) 
partyip =  "1.2.3." + str(3)
connection.AddParty(confid6, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323Party.xml")

sleep(2) 

# adding  dial out Sip video call
print 'adding  dial out Sip video call'

partyname = "Party" + str(4) 
partyip =  "1.2.3." + str(4)
partySipAdd = partyname + '@' + partyip
connection.AddSIPParty(confid6, partyname, partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialOutSipParty.xml")
    
sleep(2)    


connection.WaitAllPartiesWereAdded(confid6,4,num_retries)
connection.WaitAllOngoingDialOutConnected(confid6,num_retries)

# Check if all parties were added and save their IDs
party_id_list = []
connection.LoadXmlFile('Scripts/CreateCPConfWith4DialInParticipants/TransConf2.xml')
connection.ModifyXml("GET","ID",confid6)
connection.Send()
ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
if len(ongoing_party_list) > num_of_parties:
    sys.exit("more parties than should be...")
for index in range(num_of_parties):    
    party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)
    
# delete all parties  
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    print "delete party: " + partyname    
    connection.DeleteParty(confid6,party_id_list[x])
    sleep(1)
    
sleep(2)    

connection.DeleteConf(confid6);
connection.DeleteConf(confid5);
connection.DeleteConf(confid4);
connection.DeleteConf(confid3);
connection.DeleteConf(confid2);
connection.DeleteConf(confid1);


connection.WaitAllConfEnd()
connection.Disconnect()

    
