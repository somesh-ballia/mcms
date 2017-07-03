#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE

#############################################################################
# Test Script which Add and delete a conference reservation 
# The Transactions which will be tested:
#   1.Add a future Reservation
#   2.Update a future Reservation
#   3.Delete a future reservation
#   4.Get Reservations List+ delta monitoring
#   5.Get Single Reservation
#  
# Date: 4/08/08
# By  : Romem.
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

from McmsConnection import *
from time import *
from datetime import *


#------------------------------------------------------------------------------
def SimpleAddRemoveResTest(connection,num_of_res,isReservationWithProfile,resCreateFile,timeout):

    #connection.ClearAllDefaultRsrv()
    
    resIdArray = [0]*num_of_res
    resNumericIdArray = [0]*num_of_res

    newProfileID = -1
    profId=-1
    if isReservationWithProfile != 0:
        #add a new profile
        profId = connection.AddProfile("profile")
        newProfileID = profId
        
    #Add The Conf Reservations
    for res_num in range(num_of_res):
        resName = "RESROMEM"+str(res_num+1)
        print "Reservation Name:" + resName
        #t = datetime.now()
        t = datetime.utcnow( ) 
        print t  
        deltat = timedelta(res_num,0,0,0,30,1,0)
        t = t + deltat
        print t
        connection.CreateRes(resName,profId,t)
        resIdArray[res_num],resNumericIdArray[res_num]=connection.WaitResCreated(resName)
        sleep(1)
    #Monitor the delta and make sure it is empty
    connection.SendXmlFile('Scripts/AddRemoveReservation/TransResList.xml',"Status OK")
    objToken =connection.xmlResponse.getElementsByTagName("RES_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    
    #send the objToken and make sure that we get it back in the same value
    responsToken=CheckObjToken(connection,objToken)
    if responsToken != objToken:
        print "Error: Monitoring the Delta of the Reservation list failed"
        connection.Disconnect()                
        sys.exit("Obj token are not equal")
    print "Delta monitoring is passed, the list did not changed..."

    isResourceUnderValgrind = connection.IsProcessUnderValgrind("Resource")

    #Update all Reservations
    passValue=1234
    for res_num in range(num_of_res):  
        #resName = "ResNewName" +str(res_num+1)
        resName = "RESROMEM"+str(res_num+1)
        filedValue = str(passValue+1)
        UpdateResField(connection,resName,str(resIdArray[res_num]),profId,resNumericIdArray[res_num],"ENTRY_PASSWORD",filedValue)
    	if(isResourceUnderValgrind):
	    sleep(1)
      
    #send the objtoken and make sure that we get the update changes
    responsToken=CheckObjToken(connection,objToken)        
    if ( int(responsToken) != (int(objToken)+int(num_of_res))):
        print "Error: the Res list was updated "+str(num_of_res)+" but the delta monitoring gives:"+ responsToken+",objToken was="+ objToken
        connection.Disconnect()                
        sys.exit("Error: Monitoring the Delta of the Res list failed")
    print "Delta monitoring is passed, the Res list had "+str(num_of_res)+ " updates, which were viewed in the delta monitoring"
        
    #make sure all Reservations are still in the list
    MakeSureAllResIsAlive(connection,num_of_res)

    #Get the updated element only and make sure the the value has been changed
    passValue=1234
    print "update Entry Password"
    for res_num in range(num_of_res):
        filedValue = str(passValue+1)
        MonitorSingleRes(connection,str(resIdArray[res_num]),"ENTRY_PASSWORD",filedValue,timeout)
        
    #delta monitor the list and make sure that we get only the changed Reservationss

    #print "delete all Reservations"
    DeleteAllRes(connection)
    
    print "waiting until all Reservations  end"
    WaitAllReservationsEnd(connection,timeout)
    
    if (newProfileID != -1):
        print "Delete profile: ID = " + str(newProfileID)
        connection.DelProfile(newProfileID)

    return
#------------------------------------------------------------------------------
def CheckObjToken(connection,objToken):
    #send the objToken and make sure that we get it back in the same value
    connection.LoadXmlFile('Scripts/AddRemoveReservation/TransResList.xml') 
    connection.ModifyXml("GET_RES_LIST","OBJ_TOKEN",objToken)
    connection.Send()
    responsToken = connection.xmlResponse.getElementsByTagName("RES_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    return responsToken

#------------------------------------------------------------------------------
def MonitorSingleRes(connection,resId,fieldName,fieldValue,retires):
    print "Monitoring res id " + resId+" and checks if "+fieldName+" is equal to "+fieldValue
    for retry in range(retires+1):    
        connection.LoadXmlFile('Scripts/AddRemoveReservation/GetRes.xml')
        connection.ModifyXml("GET_RES","ID",resId)
        connection.Send()
        #get the changed value
        resList = connection.xmlResponse.getElementsByTagName("RESERVATION")
        resFiledNameVal = resList[0].getElementsByTagName(fieldName)[0].firstChild.data
        if resFiledNameVal == fieldValue:
            break
        if retry == retires:
            print "Error: Field " +fieldName +" got: " +resFiledNameVal + " while we expected " +fieldValue 
            connection.Disconnect()
            sys.exit("Monitoring singke Res Failed")
    print "Monitoring of single Res passed and "+fieldName+" = " + fieldValue
#------------------------------------------------------------------------------
def GetFieldOfSingleRes(connection,resId,fieldName,retires):
    print "Get Field of  res id " + resId+ " Field Name: "+fieldName
    for retry in range(retires+1):    
        connection.LoadXmlFile('Scripts/AddRemoveReservation/GetRes.xml')
        connection.ModifyXml("GET_RES","ID",resId)
        connection.Send()
        #get the changed value
        resList = connection.xmlResponse.getElementsByTagName("RESERVATION")
        resIdFromXML=resList[0].getElementsByTagName("ID")[0].firstChild.data
        if resIdFromXML == resId:
                resFiledNameVal = resList[0].getElementsByTagName(fieldName)[0].firstChild.data
                break       
        if retry == retires:
            print "Error: Can not Get From Reservation, Field " +fieldName  
            connection.Disconnect()
            sys.exit("Monitoring singke Res Failed")
    print "Get a field from single Res passed and "+fieldName+" = " + resFiledNameVal
    return resFiledNameVal
#------------------------------------------------------------------------------
def UpdateResField(connection,resName,resId,profileID,resNumericId,fieldName,fieldValue,status="Status OK"):
    print "Updating the field: " + fieldName+" to "+ fieldValue + " in res id " + resId + " with numericID = " + resNumericId
    retries=20
    startValueFromXml = GetFieldOfSingleRes(connection,resId,"START_TIME",retries)
    print "Start Time from Res: " + startValueFromXml + "\n"
    connection.LoadXmlFile('Scripts/AddRemoveReservation/UpdateRes.xml')
    connection.ModifyXml("RESERVATION","NAME",resName)
    connection.ModifyXml("RESERVATION","ID",resId)
    connection.ModifyXml("RESERVATION","NUMERIC_ID",resNumericId)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileID) 
    connection.ModifyXml("RESERVATION",fieldName,fieldValue)
    connection.ModifyXml("RESERVATION","START_TIME",startValueFromXml)
    connection.Send(status)
#------------------------------------------------------------------------------
def MakeSureAllResIsAlive(connection,num_of_res):
    #get the res list and make sure all reservations are there
    connection.SendXmlFile('Scripts/AddRemoveReservation/TransResList.xml',"Status OK") 
    res_list = connection.xmlResponse.getElementsByTagName("RES_SUMMARY")
    if len(res_list) != num_of_res :
        print "Error: Not all reservations were found in the list, found only "+ str(len(res_list))+" Reservations"
        connection.Disconnect()                
        sys.exit("Can not find Reservations")
    print "All " + str(num_of_res) + " Reservations are still alive in monitoring list"
#------------------------------------------------------------------------------
def DeleteAllRes(connection):
    print "Delete all Reservations..."
    connection.SendXmlFile('Scripts/AddRemoveReservation/TransResList.xml',"Status OK")
    res_list = connection.xmlResponse.getElementsByTagName("RES_SUMMARY")
    for x in range(len(res_list)):
        res_id = res_list[x].getElementsByTagName("ID")[0].firstChild.data
        connection.DelConfReservation(res_id, 'Scripts/AddRemoveReservation/DeleteRes.xml')
    return

#------------------------------------------------------------------------------
def WaitAllReservationsEnd(connection,retires = 20):
    print "Waiting until all Rservations end",
    for retry in range(retires+1):
        sleep(1)
        sys.stdout.write(".")
        sys.stdout.flush()
        connection.SendXmlFile('Scripts/AddRemoveReservation/TransResList.xml',"Status OK")
        if connection.GetTextUnder("RES_SUMMARY","ID") == "":
            print
            break
        if retry == retires:
            connection.Disconnect()
            sys.exit("Failed delete reservation!!!")
    return

##------------------------------------------------------------------------

def FormatDate(stringDate): 
    # stringDate is in the format of 2006-01-23T17:15:05 and split it to year, month, day, hour, min, second
      ss = stringDate.split('-')
      Year = ss[0]
      Month = ss[1]
      
      ss2 = ss[2].split('T')
      Day = ss2[0]
      
      ss3 = ss2[1].split(':')
      Hour =ss3[0] 
      Minute = ss3[1]
      Sec = ss3[2]
      
      return Year,  Month,  Day, Hour, Minute,  Sec

## ---------------------- Test --------------------------

c = McmsConnection()
c.Connect()
print "Invoke SimpleAddRemoveResTest"

isReservationWithProfile=1
run_num=2
numOfReservations=7
for using_profile in range(run_num):
         if using_profile ==0:
              print "Create and Remove Reservations with profile" 
              
         else:
              print "Create and Remove Reservations with profile id =-1"
              isReservationWithProfile=0
              
         SimpleAddRemoveResTest(c, #the connection class
                     numOfReservations, # num of Rservations
                     isReservationWithProfile,
                     'Scripts/AddRemoveReservation/StartRes.xml', #Reservation Script
                      20) #num of retries
         sleep(3)
c.Disconnect()

