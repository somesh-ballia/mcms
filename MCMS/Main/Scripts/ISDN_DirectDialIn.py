#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

#############################################################################
# 1. Try to create a reservation with dial-in phone number 3344 => OK
# 2. Try to create another one reservation with same phone number 3344 in same time interval => FAILURE
# 3. Try to create another one reservation with same phone number 3344 in other time interval => OK
# 4. Create an ongoing conference with dial-in phone number 3344 that already reserved => FAILURE
# 5. Create an ongoing conference with free dial-in phone number 3345 => OK
# 6. Create MR with dial-in phone number 3344 that already reserved => FAILURE
# 7. Create MR with dial-in phone number 3345 that already reserved by ongoing conf => FAILURE
# 8. Create MR with free dial-in phone number 3355 => OK
# 9. Direct dial-in (two participants) to the ongoing conference
# 10.Direct dial-in (two participants) to MR
#############################################################################

from McmsConnection import *
import os


#-----------------------------------------------------------------------------
def SimulationConnectISDNParty(connection, partyName):
    print "Connect participant from EPsim"
    connection.LoadXmlFile("Scripts/SimConnectEndpoint.xml")
    connection.ModifyXml("PARTY_CONNECT","PARTY_NAME",partyName)
    connection.Send()
#-----------------------------------------------------------------------------
def SimulationAddISDNParty(connection, partyName, phone):
    print "Add participant:" + partyName+", phone="+ phone
    connection.LoadXmlFile('Scripts/SimAddIsdnEP.xml')
    connection.ModifyXml("ISDN_PARTY_ADD","PARTY_NAME",partyName)
    connection.ModifyXml("ISDN_PARTY_ADD","PHONE_NUMBER",phone)
    connection.Send()

#-----------------------------------------------------------------------------
def DeletePartiesByConfID(connection, confID, numOfParties):
    party_id_list = []
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confID)
    connection.Send()
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    if len(ongoing_party_list) < numOfParties:
        sys.exit("some parties are lost...")
    for index in range(numOfParties):    
        party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)

    # delete all parties  
    for x in range(numOfParties):
        print "delete party ID = " + str(party_id_list[x]) + ", Conf ID = " + str(confID)
        connection.DeleteParty(confID,party_id_list[x])
        sleep(2)

## ------------------------------------------------
def TestDialInParty(connection,profileId,numOfParties,num_retries):    

    phoneNumberRsrv ="3344"
    phoneNumberConf ="3345"
    phoneNumberMR = "3355"

    profId = c.AddProfile("profile_for_reservations")

    t = datetime.utcnow( ) 
    deltat1 = timedelta(0,0,0,0,30,0,0)
    deltat2 = timedelta(0,0,0,0,30,1,0)
    t1 = t + deltat1
    t2 = t + deltat2

    print "-----------------------------------------------------------"
    print "Adding a reservation with numeric id (1234) and dial-in phone number " + phoneNumberRsrv
    connection.CreateRes("TestReserv1", profId, t1, "1234", 0, 0, "Status OK", 0, "Scripts/AddRemoveReservation/StartRes.xml", phoneNumberRsrv)

    print "-----------------------------------------------------------"
    print "Trying to add another reservation with the same dial-in phone number " + phoneNumberRsrv
    connection.CreateRes("TestReserv2", profId, t1, "1235", 0, 0, "ISDN dial-in number is already assigned to another conferencing entity", 0, "Scripts/AddRemoveReservation/StartRes.xml", phoneNumberRsrv)

    print "-----------------------------------------------------------"
    print "Trying to add another reservation with the same dial-in phone number " + phoneNumberRsrv + " but for another time interval"
    connection.CreateRes("TestReserv2", profId, t2, "1235", 0, 0, "Status OK", 0, "Scripts/AddRemoveReservation/StartRes.xml", phoneNumberRsrv)

    print "-----------------------------------------------------------"
    confName = "TargetConf"
    print "Adding a conference " + confName + " with dial-in phone number " + phoneNumberRsrv + "..."
    connection.LoadXmlFile("Scripts/UndefinedDialIn/AddCpConf_enableISDN.xml")
    connection.ModifyXml("RESERVATION","NAME",confName)
    connection.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileId) 
    connection.ModifyXml("SERVICE","PHONE1",phoneNumberRsrv)
    expected_status = "ISDN dial-in number is already assigned to another conferencing entity"
    connection.Send(expected_status)
    if expected_status != "Status OK":
       print "Indeed received required status of - " + expected_status

    print "-----------------------------------------------------------"
    print "Adding a conference " + confName + " with different dial-in phone number " + phoneNumberConf + "..."
    connection.ModifyXml("SERVICE","PHONE1",phoneNumberConf)
    connection.Send()

    targetConfID = connection.WaitConfCreated(confName,num_retries)
    
    targetConfNumericId = connection.GetConfNumericId(targetConfID)
    print "Target Conf Numeric ID is " + str(targetConfNumericId)

    print "-----------------------------------------------------------"
    mrName = "TargetMR"
    print "Adding a MR with dial-in phone " + phoneNumberRsrv + " that belongs to a reservation TestReserv..."
    connection.LoadXmlFile("Scripts/UndefinedDialIn/CreateMR_enableISDN.xml")
    connection.ModifyXml("RESERVATION","NAME",mrName)
    connection.ModifyXml("RESERVATION","DISPLAY_NAME",mrName)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(profileId))
    connection.ModifyXml("SERVICE","PHONE1",phoneNumberRsrv)
    expected_status = "ISDN dial-in number is already assigned to another conferencing entity"
    connection.Send(expected_status)

    if expected_status != "Status OK":
    	print "Indeed received required status of - " + expected_status

    print "-----------------------------------------------------------"
    print "Adding a MR " + mrName + " with dial-in phone number " + phoneNumberConf + " that belongs to a Ongoing Conf..."
    connection.ModifyXml("SERVICE","PHONE1",phoneNumberConf)
    connection.Send(expected_status)
    if expected_status != "Status OK":
    	print "Indeed received required status of - " + expected_status

    print "-----------------------------------------------------------"
    print "Adding a MR " + mrName + " with free dial-in phone number " + phoneNumberMR + "..."
    connection.ModifyXml("SERVICE","PHONE1", phoneNumberMR)
    connection.Send()
    mrId, mrNumericId = connection.WaitMRCreated(mrName)


    #Add Dial-in undefined parties
    for x in range(numOfParties*2):
        print "-----------------------------------------------------------"
        partyname = "IsdnParty"+str(x+1)
	if x < 2:
            SimulationAddISDNParty(connection,partyname,phoneNumberConf)
	else:
	    SimulationAddISDNParty(connection,partyname,phoneNumberMR)
       	SimulationConnectISDNParty(connection,partyname)
        sleep(2)
        
    print "-----------------------------------------------------------"
    #Make sure the parties are connected in the target Conf.
    connection.WaitAllPartiesWereAdded(targetConfID,numOfParties,numOfParties*num_retries)
    connection.WaitAllOngoingConnected(targetConfID,numOfParties*num_retries)
    #Make sure the parties are connected in the MR.
    mrConfId = connection.WaitUntillEQorMRAwakes(mrName,numOfParties,num_retries)
    connection.WaitAllOngoingConnected(mrConfId,numOfParties*num_retries)

    #Sleeping for IVR messages
    print "Sleeping 10 sec..."
    sleep(10)

    print "-----------------------------------------------------------"
    print "Disconnect the parties from EMA"     
    # Check if all parties were added and save their IDs      
    DeletePartiesByConfID(connection, targetConfID, numOfParties)

    # Check if all MR parties were added and save their IDs      
    DeletePartiesByConfID(connection, mrConfId, numOfParties)


    #Delete The Target Conference
    connection.DeleteConf(targetConfID)
    connection.WaitConfEnd(targetConfID)

    #Delete The Target MR
    connection.DeleteConf(mrConfId)
    connection.WaitConfEnd(mrConfId)

    print "Delete Meeting Room..."
    connection.DelReservation(mrId, 'Scripts/AwakeMrByUndef/RemoveMr.xml')



## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

#add a new profile
ProfId = c.AddProfile("profile")

TestDialInParty(c,
                ProfId,#Profile ID
                2,     #Num of parties
                20)    #Num of retries


#remove the profile
c.DelProfile(ProfId)
c.Disconnect()
