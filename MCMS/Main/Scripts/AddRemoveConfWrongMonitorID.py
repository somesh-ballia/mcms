#!/mcms/python/bin/python

#############################################################################
# Eror handling Script which Try to Delete Conf with several cases of wrong MonitorID
#
# 1.Wrong MonitorID
# 3.FFFFFFFF for MonitorID
# 2.Empty MonitorID 
#
#Also try to Add conf with an existing MonitorID
#
# Date: 30/01/06
# By  : Yoella.

#############################################################################

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

from McmsConnection import *

###------------------------------------------------------------------------------
def AddRemoveConfWrongID(connection,confFile,numRetries):
    
    confname = "ResistConf"
    connection.LoadXmlFile(confFile)
    connection.ModifyXml("RESERVATION","NAME",confname)
    print "Adding Conf " + confname + "  ..."
    connection.Send()
    
    print "Wait untill Conf  " + confname + " will be createtd...",
    sleep(2)
    confid = MonitorConf(connection,numRetries,1)
    
    print "Try to Add Conf With Same Monitor ID " + confid
    newName = "ConfShouldBeAddWithNewID"
    connection.LoadXmlFile(confFile)
    connection.ModifyXml("RESERVATION","NAME",newName)
    connection.ModifyXml("RESERVATION","ID",confid)
    print "Adding Conf " + newName + "  ..."
    connection.Send()
                
    confidfollow = "9" 
    print "Trying to Delete Conf With Wrong Monitor ID " + confidfollow
    DeleteConfStatusConfNotExists(connection,confidfollow)
    
    confidHuge="4294967295"
    print "Trying to Delete Conf With Monitor ID FFFFFFFF"
    DeleteConfStatusConfNotExists(connection,confidHuge)
    
    confidEmpty=""
    print "Trying to Delete Conf With Empty Monitor ID "
    DeleteConfStatusEnumValueInvalid(connection,confidEmpty)
   
    print "Checking that " + confname + " Still Exist",
    confID1 = MonitorConf(connection,numRetries,1)
    
    print "Checking that NewConf got New ID",
    confID2 = MonitorConf(connection,numRetries,2)
    
    if confID1 != confID2:
        print "The new conf got a new confId !!!"
        
    if confID1 == confID2:
       print "Conf with the same ID had been created !!!"
       print connection.xmlResponse.toprettyxml()
       connection.Disconnect()                
       sys.exit("Different confrences hold the same MonitorID")
   
    connection.DeleteConf(confID1)
    connection.DeleteConf(confID2)
    connection.WaitAllConfEnd(50)
    
    return
###------------------------------------------------------------------------------
def MonitorConf(connection,numRetries,index):
    
    for retry in range(numRetries+1):
        connection.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        
        confList = connection.xmlResponse.getElementsByTagName("CONF_SUMMARY")
        confid=confList[index-1].getElementsByTagName("ID")[0].firstChild.data
        
        if confid != "":
            print
            break

        if (retry == numRetries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not monitor conf")
            
        sys.stdout.write(".")
        sys.stdout.flush()
        
    print "Conf with id " + str(confid) + " exist in the list"
 
    return confid

###------------------------------------------------------------------------------
def DeleteConfStatusConfNotExists(connection,confId):
        print "Delete conference ID: "+confId        
        connection.LoadXmlFile('Scripts/DeleteVoipConf.xml')
        connection.ModifyXml("TERMINATE_CONF","ID",confId)
        connection.Send("Conference name or ID does not exist")

def DeleteConfStatusEnumValueInvalid(connection,confId):
        print "Delete conference ID: "+confId        
        connection.LoadXmlFile('Scripts/DeleteVoipConf.xml')
        connection.ModifyXml("TERMINATE_CONF","ID",confId)
        connection.Send("Invalid ENUM value")

##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

AddRemoveConfWrongID(c,
                  'Scripts/AddRemoveConfWrongMonitorID/AddConf.xml',
                  20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------
