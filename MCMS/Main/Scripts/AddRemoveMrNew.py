#!/mcms/python/bin/python

#############################################################################
# Test Script which Add and delete a Meetin rooms reservation 
# The Transactions which will be tested:
#   1.Add Meeting Room
#   2.Update Meeting Room
#   3.Delete Meeting Room
#   4.Get Meeting Room List+ delta monitoring
#   5.Get Single Meeting Room
#  
# Date: 16/01/05
# By  : Udi B.
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

from McmsConnection import *

#------------------------------------------------------------------------------
def SimpleAddRemoveMrTest(connection,num_of_mr,mrCreateFile,timeout):

#    connection.ClearAllDefaultRsrv()
    
    mrIdArray = [0]*num_of_mr
    mrNumericIdArray = [0]*num_of_mr

    #add a new profile
    profId = connection.AddProfile("profile")
        
    #Add The Meeting Room Reservations
    for mr_num in range(num_of_mr):
        mrName = "MR"+str(mr_num+1)
        connection.CreateMR(mrName,profId)
        mrIdArray[mr_num],mrNumericIdArray[mr_num]=connection.WaitMRCreated(mrName)
    print "mrIdArray" + str(mrIdArray)
    print "mrNumericIdArray" + str(mrNumericIdArray)

    #Make Sure All Mr's alive
    #MakeSureAllMrIsAlive(connection,num_of_mr)
    MakeSureMeetingRoomsIsAlive(connection, mrIdArray)
    print "mrIdArray" + str(mrIdArray)

    #Monitor the delta and make sure it is empty
    connection.SendXmlFile('Scripts/AddRemoveMrNew/TransMrList.xml',"Status OK")
    objToken =connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    
    #send the objToken and make sure that we get it back in the same value
    responsToken=CheckObjToken(connection,objToken)
    if responsToken != objToken:
        print "Error: Monitoring the Delta of the Mr list failed"
        connection.Disconnect()                
        sys.exit("Obj token are not equal")
    print "Delta monitoring is passed, the list did not changed..."

    #Update all Meeting Rooms
    passValue=1234
    for mr_num in range(num_of_mr):  
        mrName = "MR" +str(mr_num+1)
        filedValue = str(passValue+1)
        UpdateMrField(connection,mrName,str(mrIdArray[mr_num]),profId,mrNumericIdArray[mr_num],"ENTRY_PASSWORD",filedValue) 
    
    if (connection.IsProcessUnderValgrind("ConfParty")):
        sleep(4)
    #send the objtoken and make sure that we get the update changes
    responsToken=CheckObjToken(connection,objToken)        
    if ( int(responsToken) != (int(objToken)+int(num_of_mr))):
        print "Error: the list was updated "+str(num_of_mr)+" but the delta monitoring gives:"+ responsToken+",objToken was="+ objToken
        connection.Disconnect()                
        sys.exit("Error: Monitoring the Delta of the Mr list failed")
    print "Delta monitoring is passed, the list had "+str(num_of_mr)+ " updates, which were viewed in the delta monitoring"
        
    #make sure all mr's are still in the list
    #MakeSureAllMrIsAlive(connection,num_of_mr)
    MakeSureMeetingRoomsIsAlive(connection, mrIdArray)

    #Get the updated element only and make sure the the value has been changed
    passValue=1234
    for mr_num in range(num_of_mr):
        filedValue = str(passValue+1)
        MonitorSingleMr(connection,str(mrIdArray[mr_num]),"ENTRY_PASSWORD",filedValue,timeout)
        
    #delta monitor the list and make sure that we get only the chnged mr's

    #print "delete all Meeting Rooms"
    #DeleteAllMeetingRooms(connection)
    DeleteMeetingRooms(connection, mrIdArray)
    if (profId != -1):
        connection.DelProfile(profId)

    #print "waiting until all Meeting Rooms end"
    #WaitAllMrEnd(connection,timeout)
    WaitMrEnd(connection, mrIdArray, timeout)
    
    return

#------------------------------------------------------------------------------
def CheckObjToken(connection,objToken):
    #send the objToken and make sure that we get it back in the same value
    connection.LoadXmlFile('Scripts/AddRemoveMrNew/TransMrList.xml') 
    connection.ModifyXml("GET_MEETING_ROOM_LIST","OBJ_TOKEN",objToken)
    connection.Send()
    responsToken = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    return responsToken

#------------------------------------------------------------------------------
def MonitorSingleMr(connection,mrId,fieldName,fieldValue,retires):
    print "Monitoring mr id " + mrId+" and checks if "+fieldName+" is equal to "+fieldValue
    for retry in range(retires+1):    
        connection.LoadXmlFile('Scripts/AddRemoveMrNew/GetMr.xml')
        connection.ModifyXml("GET_MEETING_ROOM","ID",mrId)
        connection.Send()
        #get the changed value
        mrRsrvList = connection.xmlResponse.getElementsByTagName("RESERVATION")
        mrFiledNameVal = mrRsrvList[0].getElementsByTagName(fieldName)[0].firstChild.data
        if mrFiledNameVal == fieldValue:
            break
        if retry == retires:
            print "Error: Field " +fieldName +" got: " +mrFiledNameVal + " while we expected " +fieldValue 
            connection.Disconnect()
            sys.exit("Monitoring singke Mr Failed")
    print "Monitoring of single Mr passed and "+fieldName+" = " + fieldValue
#------------------------------------------------------------------------------
def UpdateMrField(connection,mrName,mrId,profileID,mrNumericId,fieldName,fieldValue,status="Status OK"):
    print "Updating the field: " + fieldName+" to "+ fieldValue + " in mr id " + mrId + " with numericID = " + mrNumericId
    #connection.LoadXmlFile('Scripts/AddRemoveMrNew/UpdateMr.xml')
    connection.LoadXmlFile('Scripts/UpdateMR.xml')
    connection.ModifyXml("RESERVATION","NAME",mrName)
    connection.ModifyXml("RESERVATION","ID",mrId)
    connection.ModifyXml("RESERVATION","NUMERIC_ID",mrNumericId)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileID) 
    connection.ModifyXml("RESERVATION",fieldName,fieldValue)
    connection.Send(status)
    
#------------------------------------------------------------------------------
def MakeSureAllMrIsAlive(connection,num_of_mr):
    #get the mr list and make sure all mr's are there
    connection.SendXmlFile('Scripts/AddRemoveMrNew/TransMrList.xml',"Status OK") 
    mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    if len(mr_list) != num_of_mr :
        print "Error: Not all Mr's were found in the list, found only "+ str(len(mr_list))+" Mr's"
        connection.Disconnect()                
        sys.exit("Can not find Mrs")
    print "All " + str(num_of_mr) + " Mr's are still alive in monitoring list"
#------------------------------------------------------------------------------
def MakeSureMeetingRoomsIsAlive(connection, mr_IDs_list):
    mrIDsList = mr_IDs_list[:]
    #get the mr list and make sure all mr's are there
    print "Delete Meeting Rooms..."
    connection.SendXmlFile('Scripts/AddRemoveMrNew/TransMrList.xml',"Status OK") 
    mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    for x in range(len(mr_list)):
        mr_id = mr_list[x].getElementsByTagName("ID")[0].firstChild.data
        if mr_id in mrIDsList:
            mrIDsList.remove(mr_id)
        if len(mrIDsList) == 0 :
            break
    if len(mrIDsList) != 0 :
        print "Error: Not all Mr's were found in the list: "+ str(mrIDsList)+" Mr's"
        connection.Disconnect()                
        sys.exit("Can not find Mrs")
    print "All " + str(len(mrIDsList)) + " Mr's are still alive in monitoring list"
    
#------------------------------------------------------------------------------
def DeleteAllMeetingRooms(connection):
    print "Delete all Meeting Rooms..."
    connection.SendXmlFile('Scripts/AddRemoveMrNew/TransMrList.xml',"Status OK")
    mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    for x in range(len(mr_list)):
        mr_id = mr_list[x].getElementsByTagName("ID")[0].firstChild.data
        connection.DelReservation(mr_id, 'Scripts//AddRemoveMrNew/DeleteMR.xml')
    return
#------------------------------------------------------------------------------
def DeleteMeetingRooms(connection, mr_IDs_list):
    print "Delete Meeting Rooms..."
    connection.SendXmlFile('Scripts/AddRemoveMrNew/TransMrList.xml',"Status OK")
    mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
    for x in range(len(mr_list)):
        mr_id = mr_list[x].getElementsByTagName("ID")[0].firstChild.data
        if mr_id in mr_IDs_list:
            connection.DelReservation(mr_id, 'Scripts//AddRemoveMrNew/DeleteMR.xml')
    return

#------------------------------------------------------------------------------
def WaitAllMrEnd(connection,retires = 20):
    print "Waiting until all conferences MR end",
    for retry in range(retires+1):
        sleep(1)
        sys.stdout.write(".")
        sys.stdout.flush()
        connection.SendXmlFile('Scripts/AddRemoveMrNew/TransMrList.xml',"Status OK")
        if connection.GetTextUnder("MEETING_ROOM_SUMMARY","ID") == "":
            print
            break
        if retry == retires:
            connection.Disconnect()
            sys.exit("Failed delete conference!!!")
    return

#------------------------------------------------------------------------------
def WaitMrEnd(connection, mr_IDs_list, retires = 20):
    bExists = 0
    print "Waiting until conferences MR end",
    print "mr_IDs_list" + str(mr_IDs_list)
    for retry in range(retires+1):
        sleep(1)
        sys.stdout.write(".")
        sys.stdout.flush()
        bExists = 0
        connection.SendXmlFile('Scripts/AddRemoveMrNew/TransMrList.xml',"Status OK")
        mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        for x in range(len(mr_list)):
            mr_id = mr_list[x].getElementsByTagName("ID")[0].firstChild.data
            if mr_id in mr_IDs_list:
                bExists = 1
                break
        if bExists == 0 :
            print
            break
        if retry == retires:
            connection.Disconnect()
            sys.exit("Failed delete conference!!!")
    return

## ---------------------- Test --------------------------

c = McmsConnection()
c.Connect()
SimpleAddRemoveMrTest(c, #the connection class
                      7, # num of Meeting Rooms
                      'Scripts/AddRemoveMrNew/CreateMR.xml', #Meeting Room Script
                      20) #num mof retries
c.Disconnect()

