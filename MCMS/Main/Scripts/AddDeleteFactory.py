#!/mcms/python/bin/python

#############################################################################
# Test Script which Add, Update and Remove FACTORY
#
# Date: 22/01/06
# By  : Ron S.

#############################################################################

from McmsConnection import *

###------------------------------------------------------------------------------
def AddRemoveFactoryTest(connection,AddProfile,AddFactoryFile,
                         UpdateFactoryFile,
                         RemoveFactoryFile,
                         numRetries):

    print "Starting test TestAddRemoveFactory ... "
    
 #add a new profile
    profId = connection.AddProfile("Profile1", AddProfile)

    #Adding a new Factory
    factoryName = "fac1"
    print "Adding a new Factory:" + factoryName + " ..."
    connection.LoadXmlFile(AddFactoryFile)
    connection.ModifyXml("RESERVATION","NAME",factoryName)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(profId))
    connection.Send()

  #make sure that the Factory was added
    FacId = ""
    print "Wait untill Factory: " + factoryName + " will be created..."
    for retry in range(numRetries+1):
        connection.SendXmlFile('Scripts/AddDeleteFactory/TransMrList.xml',"Status OK") 
        mr_list = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        if len(mr_list) > 0 :            
            FacId = mr_list[0].getElementsByTagName("ID")[0].firstChild.data
            print
            break
        if (retry == numRetries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not monitor conf:")
        sys.stdout.write(".")
        sys.stdout.flush()
    print "Factory Id is: "+ FacId
    
    
    #Monitor the delta and make sure it is empty
    connection.SendXmlFile('Scripts/AddRemoveMrNew/TransMrList.xml',"Status OK")
    objToken =connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    
    #send the objToken and make sure that we get it back in the same value
    responsToken=CheckObjToken(connection,objToken)
    if responsToken != objToken:
        print "Error: Monitoring the Delta of the Factory list failed"
        connection.Disconnect()                
        sys.exit("Obj token are not equal")
    print "Delta monitoring is passed, the list did not changed..."
    
    
 #Update the Factory
    print "Updating the Factory:" + factoryName + " ..."
    connection.LoadXmlFile(UpdateFactoryFile)
    connection.ModifyXml("RESERVATION","NAME",factoryName)
    connection.ModifyXml("RESERVATION","ID",FacId)
#    connection.ModifyXml("RESERVATION","SIP_FACTORY_AUTO_CONNECT","true")
    connection.Send("Status OK")
    

   #send the objtoken and make sure that we get the update changes
    responsToken=CheckObjToken(connection,objToken)        
    if ( int(responsToken) != (int(objToken)+1)):
        print "Error: the list was updated "+str(1)+" but the delta monitoring gives:"+ responsToken+",objToken was="+ objToken
        connection.Disconnect()                
        sys.exit("Error: Monitoring the Delta of the Factory list failed")
    print "Delta monitoring is passed, the list had "+str(1)+ " updates, which were viewed in the delta monitoring"
    
 
  #Remove the Factory 
    print "Remove the Factory..."
    connection.SendXmlFile(RemoveFactoryFile)

 #Remove the profile
    connection.DelProfile(profId, "Scripts/AddDeleteFactory/RemoveNewProfile.xml")

    return

#------------------------------------------------------------------------------
def CheckObjToken(connection,objToken):
    #send the objToken and make sure that we get it back in the same value
    connection.LoadXmlFile('Scripts/AddRemoveMrNew/TransMrList.xml') 
    connection.ModifyXml("GET_MEETING_ROOM_LIST","OBJ_TOKEN",objToken)
    connection.Send()
    responsToken = connection.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    return responsToken


##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

AddRemoveFactoryTest(c,                     
                     'Scripts/AddDeleteFactory/CreateNewProfile.xml',
                     'Scripts/AddDeleteFactory/AddFactory.xml', 
                     'Scripts/AddDeleteFactory/updateFactory.xml',
                     'Scripts/AddDeleteFactory/RemoveFactory.xml',                                                 
                      20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------
