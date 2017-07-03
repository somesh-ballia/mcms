#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-LONG_SCRIPT_TYPE

#############################################################################
# Test Script which test the multi move of undefined parties for full capacity 
# with parameter of minimum participants number for both EQ and target conference.
# The number of real parties in the test is equal to the full system capacity minus 
# minimum participants number.
# By  : Sergey and Michael(used infrastructure of MoveUnfef.py).
#############################################################################

from ResourceUtilities import *

def TestResWithEqAndMinUndefParty(connection,number_of_min_parties,num_retries):

    connection.SendXmlFile("Scripts/ResourceFullCapacity/ResourceMonitorCarmel.xml")
    num_of_resource = connection.GetTotalCarmelParties()
    print "RSRC_REPORT_RMX[Total] = " + str(num_of_resource)
    connection.TestFreeCarmelParties(num_of_resource)
        #add a new profile
    if num_of_resource < number_of_min_parties * 2 :  # because of 2 conferences
        sys.exit("Inconsistent number of minimum parties")
   
    print "Adding new Profile..."
    connection.LoadXmlFile("Scripts/ResourceMultMoveUndef/CreateNewProfile.xml")
    connection.ModifyXml("RESERVATION","TRANSFER_RATE",str(128))   
    connection.ModifyXml("MEET_ME_PER_CONF","ON","true")
    connection.ModifyXml("MEET_ME_PER_CONF","MIN_NUM_OF_PARTIES",int(number_of_min_parties))
    connection.Send()
    ProfId = connection.GetTextUnder("RESERVATION","ID")
    my_name = connection.GetTextUnder("RESERVATION","NAME")
    print "Profile, named: " + my_name + " ,ID = " + ProfId + ", is added"

    #send a new EQ service
    print "Adding a new Entry Queue Service...."
    connection.SendXmlFile("Scripts/ResourceMultMoveUndef/AddEqService.xml","Status OK")
        
    # send a new Reservation of EntryQueue
    eqName = "eqRsrv"
    connection.CreateAdHocEQ(eqName, ProfId, "Scripts/ResourceMultMoveUndef/CreateNewEq.xml")
   
    #os.system("usleep 200000")
    
    #Wait untill eq was added
    eqId,eqNid = connection.WaitMRCreated(eqName,num_retries)
    
    #create the target Conf and wait untill it connected
    targetConfName = "targetConf"
    targetConfID  = AddConf(connection,targetConfName,ProfId,num_retries)
    targetConfNumericId = GetConfNumericId(connection,targetConfID)
    print "Target Conf Numeric id is " + str(targetConfNumericId)

    numOfParties = int(num_of_resource) - int(number_of_min_parties)
    for i in range(numOfParties):
        partyName = "Party" +str(i+1)
        connection.SimulationAddH323Party(partyName, eqName)
        connection.SimulationConnectH323Party(partyName)
        

    #Wait untill Eq was awake
   # numOfParties = 1
        if i==0 :
           eqConfId = connection.WaitUntillEQorMRAwakes(eqName,1,num_retries,True)
        #send the TDMF to the EPsim with the numeric id of the target conf
        sleep(2)

        connection.SimulationH323PartyDTMF(partyName, targetConfNumericId)

	#Wait untill the party will be connected in the target conf
	sleep(1)

    WaitUntilPartyConnected(connection,targetConfID,numOfParties,num_retries*numOfParties)
    
#    connection.TestFreeCarmelParties(0) # equal to reserved number_of_min_parties

#    connection.TestReservedCarmelParties(number_of_min_parties) #TBD for undefined separate field

    connection.TestOccupiedCarmelParties(numOfParties)
    sleep(1)
    #delete the target conf
    connection.DeleteConf(targetConfID)
    sleep(5)
    #delete the Eq-conf
#    connection.TestFreeCarmelParties( numOfParties )
    connection.DeleteConf(eqConfId)
#    connection.DeleteCWaitUntilPartyConnectedonf(eqConfId)
    
    connection.WaitAllConfEnd(num_retries*2)
    
    connection.TestFreeCarmelParties(num_of_resource)
    #remove the EQ Resrv
    connection.DelReservation(eqId,"Scripts/ResourceMultMoveUndef/RemoveEq.xml")
    
    #remove the profile
    connection.DelProfile(ProfId, "Scripts/ResourceMultMoveUndef/RemoveNewProfile.xml")
    
    return

#------------------------ Test --------------------------
c = ResourceUtilities()
c.Connect()

TestResWithEqAndMinUndefParty(c, 3, 30)
c.Disconnect()
