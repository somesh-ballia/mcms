#!/mcms/python/bin/python

###################################
# Re-Write date =03/10/13
# Re-Write name = Uri A.
###################################

#-LONG_SCRIPT_TYPE
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

from McmsConnection import *

#------------------------------------------------------------------------------
def AddMr(connection, mr_num, mrCreateFile, timeout):
    #Add The Meeting Room Reservations
    mrName = "mr1"
    connection.LoadXmlFile(mrCreateFile)
    connection.ModifyXml("RESERVATION","NAME",mrName)
    print "Adding New Meeting Room : " + mrName + "  ..."
    connection.Send()
        
    #make sure all Meeting rooms were added
    print "Wait untill MR  will be created..."
    for retry in range(timeout+1):
        connection.SendXmlFile('Scripts/SipDialInToMR/TransMrList.xml',"Status OK") 
        mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        if len(mr_list) == mr_num :
            mrIdArray = mr_list[mr_num-1].getElementsByTagName("ID")[0].firstChild.data
            print 
            break
        if (retry == timeout):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not monitor conf")
            sys.stdout.write(".")
            sys.stdout.flush()

    IsConfPartyUnderValgrind = False
    if (connection.IsProcessUnderValgrind("ConfParty")):
        IsConfPartyUnderValgrind = True
    if (IsConfPartyUnderValgrind):
    	sleep(2)

    #Make Sure All Mr's alive
    MakeSureAllMrIsAlive(connection,mr_num)

    #Monitor the delta and make sure it is empty
    connection.SendXmlFile('Scripts/SipDialInToMR/TransMrList.xml',"Status OK")
    objToken =connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    #send the objToken and make sure that we get it back in the same value
    responsToken=CheckObjToken(connection,objToken)
    if responsToken != objToken:
        print "Error: Monitoring the Delta of the Mr list failed"
        connection.Disconnect()                
        sys.exit("Obj token are not equal")
    print "Delta monitoring is passed, the list did not change..."

    return 

#----------------------------------------------------------------------------------
def TestStatusAndRemoveMR(connection, num_of_mr, num_of_parties, timeout):
    print '1'
    #Monitor the delta and make sure it is empty
    connection.SendXmlFile('Scripts/SipDialInToMR/TransMrList.xml',"Status OK")
    objToken =connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    #send the objtoken and make sure that we get the update changes
    responsToken=CheckObjToken(connection,objToken)   
    #list was updated: num_of_mr + connection of parties
    if responsToken != objToken:
        print "Error: the list was updated "+str(num_of_mr+num_of_parties)+" but the delta monitoring gives:"+ responsToken+",objToken was="+ objToken
        connection.Disconnect()                
        sys.exit("Error: Monitoring the Delta of the Mr list failed")
    print "Delta monitoring is passed, the list had "+str(num_of_mr)+ " updates, which were viewed in the delta monitoring"
        
    #make sure all mr's are still in the list
    MakeSureAllMrIsAlive(connection,num_of_mr)
 
    print "delete all Meeting Rooms"
    DeleteAllMeetingRooms(connection)
    IsConfPartyUnderValgrind = False
    if (connection.IsProcessUnderValgrind("ConfParty")):
        IsConfPartyUnderValgrind = True
    if (IsConfPartyUnderValgrind):
        sleep(2)

    print "waiting until all Meeting Rooms end"
    WaitAllMrEnd(connection,timeout)

#------------------------------------------------------------------------------
def CheckObjToken(connection,objToken):
    #send the objToken and make sure that we get it back in the same value
    connection.LoadXmlFile('Scripts/SipDialInToMR/TransMrList.xml') 
    connection.ModifyXml("GET_MEETING_ROOM_LIST","OBJ_TOKEN",objToken)
    connection.Send()
    responsToken = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    return responsToken

#------------------------------------------------------------------------------
def MakeSureAllMrIsAlive(connection,num_of_mr):
    #get the mr list and make sure all mr's are there
    connection.SendXmlFile('Scripts/SipDialInToMR/TransMrList.xml',"Status OK") 
    mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    if len(mr_list) != num_of_mr :
        print "Error: Not all Mr's were found in the list, found only "+ str(len(mr_list))+" Mr's"
        connection.Disconnect()                
        sys.exit("Can not find Mrs")
    print "All " + str(num_of_mr) + " Mr's are still alive in monitoring list"
#------------------------------------------------------------------------------
def DeleteAllMeetingRooms(connection):
    print "Delete all Meeting Rooms..."
    connection.SendXmlFile('Scripts/SipDialInToMR/TransMrList.xml',"Status OK")
    mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    for x in range(len(mr_list)):
        mr_id = mr_list[x].getElementsByTagName("ID")[0].firstChild.data
        connection.DelReservation(mr_id, 'Scripts//AddRemoveMrNew/DeleteMR.xml')
        
    return

#------------------------------------------------------------------------------
def WaitAllMrEnd(connection,retires = 20):
    print "Waiting until all conferences MR end",
    for retry in range(retires+1):
        sleep(1)
        sys.stdout.write(".")
        sys.stdout.flush()
        connection.SendXmlFile('Scripts/SipDialInToMR/TransMrList.xml',"Status OK")
        if connection.GetTextUnder("MEETING_ROOM_SUMMARY","ID") == "":
            print
            break
        if retry == retires:
            connection.Disconnect()
            sys.exit("Failed delete conference!!!")
    return

#------------------------------------------------------------------------------
def TestUndefinedDailIn(connection, num_of_parties, num_retries):
    confName = "mr1"

    ### Add parties to EP Sim and connect them
    for x in range(num_of_parties):
        partyname = "Party"+str(x+1)
        connection.SimulationAddSipParty(partyname, confName)
        connection.SimulationConnectSipParty(partyname)
    IsConfPartyUnderValgrind = False
    if (connection.IsProcessUnderValgrind("ConfParty")):
        IsConfPartyUnderValgrind = True

    if (IsConfPartyUnderValgrind):
    	sleep(2)
   
    #Wait untill Meeting room is awake
    ConfId = connection.WaitUntillEQorMRAwakes(confName, num_of_parties,num_retries)
    IsConfPartyUnderValgrind = False
    if (connection.IsProcessUnderValgrind("ConfParty")):
        IsConfPartyUnderValgrind = True
    if (IsConfPartyUnderValgrind):
    	sleep(2)

    # Check if all parties were added and save their IDs
    party_id_list = [0]*num_of_parties 
    connection.LoadXmlFile('Scripts/SipDialInToMR/TransConf2.xml')
    connection.ModifyXml("GET","ID",ConfId)
    connection.Send()
    ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    if len(ongoing_party_list) < num_of_parties:
        errMsg= "some parties are lost, find only " +str(len(ongoing_party_list)) + " parties in conf"
        sys.exit(errMsg )
    for index in range(num_of_parties):
        party_id_list[(num_of_parties - index) - 1 ]  = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data

    sleep(3)

    for index in range(num_of_parties):
        partyName = "Party"+str(index+1)
        connection.SimulationDisconnectSipParty(partyName)
        connection.SimulationDeleteSipParty(partyName)
    IsConfPartyUnderValgrind = False
    if (connection.IsProcessUnderValgrind("ConfParty")):
        IsConfPartyUnderValgrind = True
    if (IsConfPartyUnderValgrind):
    	sleep(2)

    connection.WaitUntillPartyDeleted(ConfId,num_of_parties*num_retries)    
    connection.DeleteConf(ConfId)
    
    return

## ---------------------- Test --------------------------

c = McmsConnection()
c.Connect()

num_of_mr = 1
num_of_parties = 3

c.ClearAllDefaultRsrv()
AddMr(c, #the connection class
                      num_of_mr, # num of Meeting Rooms
                      'Scripts/SipDialInToMR/CreateMR.xml', #Meeting Room Script
                      20) #num mof retries
                      
TestUndefinedDailIn(c,
                    num_of_parties, # num of parties
                    20)# retries
        
sleep(3)

TestStatusAndRemoveMR(c,
                      num_of_mr,
                      num_of_parties,
                      20)
                    
c.Disconnect()

