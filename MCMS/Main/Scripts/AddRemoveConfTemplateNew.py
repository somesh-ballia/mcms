#!/mcms/python/bin/python

#############################################################################
# Test Script which Add and delete a conference reservation 
# The Transactions which will be tested:
#   1.Add/Update/Delete a Conference Template
#   2. Move all Conference Template to OnGoing
#   3.Get Conference Template List+ delta monitoring
#   4.Get Single Conference Template
#   5.Move all Conference Template to OnGoing
#  
# Date: 2/11/08
# By  : Romem.
#############################################################################

# For list of profiles look at RunTest.sh
#-LONG_SCRIPT_TYPE
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

from McmsConnection import *
from time import *
from datetime import *   

#------------------------------------------------------------------------------
def IsForceInPartyLevel(connection,confTemplateId,retires):
    isForceInPartyLevel=0
    for retry in range(retires+1):    
        connection.LoadXmlFile('Scripts/AddRemoveConfTemplate/GetConfTemplate.xml')
        connection.ModifyXml("GET_CONFERENCE_TEMPLATE","ID",confTemplateId)
        connection.Send()
        partyList = connection.xmlResponse.getElementsByTagName("PARTY_LIST")
        for index in range(len(partyList)):
            partyName = partyList[index].getElementsByTagName("NAME")[0].firstChild.data
            print "Party Name: "+str(partyName)
        for party in partyList:
            #currentPartyId = party.getElementsByTagName("ID")[0].firstChild.data
            cellList = party.getElementsByTagName("CELL")
            for cell in cellList:
               forceName=cell.getElementsByTagName("FORCE_NAME")[0].firstChild.data
               sourceId=cell.getElementsByTagName("SOURCE_ID")[0].firstChild.data
               print "force Name" + str(forceName)
               print "source Id" + str(sourceId)
               if str(forceName)!="":
                   isForceInPartyLevel=1
                   break 
    if isForceInPartyLevel:
        print "Recognize Force in Party Level" 
    else:
        print "Did not Recognize Force in Party Level"
    return isForceInPartyLevel
#------------------------------------------------------------------------------
def SimpleAddRemoveConfTemplateTest(connection,num_of_templates,confTempCreateFile,isConfFromProfile,timeout):

#    connection.ClearAllDefaultRsrv()
    
    ConfTemplateIdArray = [0]*num_of_templates

    profId=-1
    rate=384
    #add a new profile
    if(isConfFromProfile):
        profId = connection.AddProfile("profile")
        print "New profile ID = " + profId
    else:
        rate=768
        
    #Add The Conf Template
    for temp_num in range(num_of_templates):
        tempName = "CONFTEMP"+str(temp_num+1)
        print "Conf Template Name:" + tempName
        connection.CreateConfTemplate(tempName,profId,"Scripts/AddRemoveConfTemplate/StartConfTemplate.xml",rate)
        ConfTemplateIdArray[temp_num]=connection.WaitConfTemplateCreated(tempName)
        sleep(1)

    #Monitor the delta and make sure it is empty
    connection.SendXmlFile('Scripts/AddRemoveConfTemplate/TransConfTemplateList.xml',"Status OK")
    objToken =connection.xmlResponse.getElementsByTagName("CONFERENCE_TEMPLATE_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    
    #send the objToken and make sure that we get it back in the same value
    responsToken=CheckObjToken(connection,objToken)
    if responsToken != objToken:
        print "Error: Monitoring the Delta of the Reservation list failed"
        connection.Disconnect()                
        sys.exit("Obj token are not equal")
    print "Delta monitoring is passed, the list did not changed..."

    #Update all Conf Templates
    passValue=1234
    for temp_num in range(num_of_templates):
        newTempName = "CONFTEMP"+str(temp_num+1)
        filedValue = str(passValue+1)
        UpdateConfTemplateField(connection,newTempName,str(ConfTemplateIdArray[temp_num]),profId,"ENTRY_PASSWORD",filedValue)         
      
    #send the objtoken and make sure that we get the update changes
    responsToken=CheckObjToken(connection,objToken)        
    if ( int(responsToken) != (int(objToken)+int(num_of_templates))):
        print "Error: the Conf Template list was updated "+str(num_of_templates)+" but the delta monitoring gives:"+ responsToken+",objToken was="+ objToken
        connection.Disconnect()                
        sys.exit("Error: Monitoring the Delta of the Conf Template list failed")
    print "Delta monitoring is passed, the Conf Template list had "+str(num_of_templates)+ " updates, which were viewed in the delta monitoring"
        
    #make sure all Teplates are still in the list
    MakeSureAllConfTemplateIsAlive(connection,num_of_templates)
    #Get the updated element only and make sure the the value has been changed
    print "update Entry Password"
    passValue=1234
    for temp_num in range(num_of_templates):
        filedValue = str(passValue+1)
        MonitorSingleConfTemplate(connection,str(ConfTemplateIdArray[temp_num]),"ENTRY_PASSWORD",filedValue,timeout)
        
    #check if Confparty is under valgrind
    IsConfPartyUnderValgrind = False
    if (connection.IsAnyProcessUnderValgrind(['ConfParty','CDR','Resource'])):
    	print "Process is under Valgrind"
        IsConfPartyUnderValgrind = True
        
    #delta monitor the list and make sure that we get only the changed Reservations
    # Move all templates to OnGoing
    print " Move all templates to OnGoing"
    for temp_num in range(num_of_templates): 
    	sleep(3)
    	if (IsConfPartyUnderValgrind): 
        	sleep(10)
        MoveSingleConfTemplateToOngoing(connection,ConfTemplateIdArray[temp_num],20)
    sleep(3)    
    if (IsConfPartyUnderValgrind):        	    
    	sleep(10)    
    print "delete all Conf Templates"
    DeleteAllConfTemplates(connection)
    #print "waiting until all ConfTemplates  end"
    WaitAllConfTemplatesEnd(connection,timeout)
    sleep(3)
    print "Delete all OnGoing Conf Templates"
    connection.DeleteAllConf(3)
    if (IsConfPartyUnderValgrind):        	    
    	sleep(3)
    #print "waiting until all conferences end"
    connection.WaitAllConfEnd(timeout)
    
    if (profId != -1):
        connection.DelProfile(profId)
    return
#------------------------------------------------------------------------------
def CheckObjToken(connection,objToken):
    #send the objToken and make sure that we get it back in the same value
    connection.LoadXmlFile('Scripts/AddRemoveConfTemplate/TransConfTemplateList.xml') 
    connection.ModifyXml("GET_CONFERENCE_TEMPLATE_LIST","OBJ_TOKEN",objToken)
    connection.Send()
    responsToken = connection.xmlResponse.getElementsByTagName("CONFERENCE_TEMPLATE_SUMMARY_LS")[0].getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    return responsToken

#------------------------------------------------------------------------------
def MoveSingleConfTemplateToOngoing(connection,confTemplateId,retires):    
    print "Move To Ongoing Template ID: "+str(confTemplateId)
    connection.LoadXmlFile('Scripts/AddRemoveConfTemplate/GetConfTemplate.xml')
    connection.ModifyXml("GET_CONFERENCE_TEMPLATE","ID",confTemplateId)
    connection.Send()
    confTemplateList = connection.xmlResponse.getElementsByTagName("RESERVATION")
    entryPasswordFromXml = confTemplateList[0].getElementsByTagName("ENTRY_PASSWORD")[0].firstChild.data
    confTemplateName   = confTemplateList[0].getElementsByTagName("NAME")[0].firstChild.data
    print "Move Template to OnGoing, Name:  " + confTemplateName
    connection.LoadXmlFile('Scripts/AddRemoveConfTemplate/StartConfTemplate.xml')
    connection.ModifyXml("RESERVATION","ID",confTemplateId)
    connection.ModifyXml("RESERVATION","NAME",confTemplateName)
    connection.ModifyXml("RESERVATION","ENTRY_PASSWORD",entryPasswordFromXml)
    connection.loadedXml.getElementsByTagName("CONFERENCE_TEMPLATE")[0].getElementsByTagName("ON")[0].firstChild.data = "false"
    connection.Send()
    connection.WaitConfCreated(confTemplateName,retires)
       
#------------------------------------------------------------------------------
def MonitorSingleConfTemplate(connection,confTemplateId,fieldName,fieldValue,retires):
    print "Monitoring ConfTemplate id " + confTemplateId+" and checks if "+fieldName+" is equal to "+fieldValue
    for retry in range(retires+1):    
        connection.LoadXmlFile('Scripts/AddRemoveConfTemplate/GetConfTemplate.xml')
        connection.ModifyXml("GET_CONFERENCE_TEMPLATE","ID",confTemplateId)
        connection.Send()
        #get the changed value
        confTemplateList = connection.xmlResponse.getElementsByTagName("RESERVATION")
        confTemplateFiledNameVal = confTemplateList[0].getElementsByTagName(fieldName)[0].firstChild.data
        if confTemplateFiledNameVal == fieldValue:
            break
        if retry == retires:
            print "Error: Field " +fieldName +" got: " +confTemplateFiledNameVal + " while we expected " +fieldValue 
            connection.Disconnect()
            sys.exit("Monitoring single ConfTemplate Failed")
    print "Monitoring of single ConfTemplate passed and "+fieldName+" = " + fieldValue
#------------------------------------------------------------------------------
def GetFieldOfSingleConfTemplate(connection,confTemplateId,fieldName,retires):
    print "Get Field of  Conf Template id " + confTemplateId+ " Field Name: "+fieldName
    for retry in range(retires+1): 
       connection.LoadXmlFile('Scripts/AddRemoveConfTemplate/GetConfTemplate.xml')
       connection.ModifyXml("GET_CONFERENCE_TEMPLATE","ID",confTemplateId)
       connection.Send()
        #get the changed value
       confTemplateList = connection.xmlResponse.getElementsByTagName("RESERVATION")
       confTemplateIdFromXML=confTemplateList[0].getElementsByTagName("ID")[0].firstChild.data
       if confTemplateIdFromXML == confTemplateId:
           print "Filed name: "+fieldName
           confTemplateFileNameVal = confTemplateList[0].getElementsByTagName(fieldName)[0].firstChild.data
           break       
       if retry == retires:
           print "Error: Can not Get From ConfTemplate, Field " +fieldName  
           connection.Disconnect()
           sys.exit("Monitoring single ConfTemplate Failed")
    print "Get a field from single ConfTemplate passed and "+fieldName+" = " + confTemplateIdFromXML
    return confTemplateIdFromXML
#------------------------------------------------------------------------------
def UpdateConfTemplateField(connection,templateName,templateId,profileID,fieldName,fieldValue,status="Status OK"):
    print "Updating the field: " + fieldName+" to "+ fieldValue + " in Conf Template id " + templateId #+ " with numericID = " + templateNumericId
    retries=20
    #startValueFromXml = GetFieldOfSingleConfTemplate(connection,templateId,"START_TIME",retries)
    #print "Start Time from Res: " + startValueFromXml + "\n"
    connection.LoadXmlFile('Scripts/AddRemoveConfTemplate/UpdateConfTemplate.xml')
    connection.ModifyXml("RESERVATION","NAME",templateName)
    connection.ModifyXml("RESERVATION","ID",templateId)
    #connection.ModifyXml("RESERVATION","NUMERIC_ID",templateNumericId)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileID) 
    connection.ModifyXml("RESERVATION",fieldName,fieldValue)
    connection.loadedXml.getElementsByTagName("DURATION")[0].getElementsByTagName("HOUR")[0].firstChild.data = 2
    #connection.ModifyXml("RESERVATION","START_TIME",startValueFromXml)
    connection.Send(status)
#------------------------------------------------------------------------------
def MakeSureAllConfTemplateIsAlive(connection,num_of_confTemplates):
    #get the Conf Template list and make sure all templates are there
    connection.SendXmlFile('Scripts/AddRemoveConfTemplate/TransConfTemplateList.xml',"Status OK") 
    conf_template_list = connection.xmlResponse.getElementsByTagName("CONFERENCE_TEMPLATE_SUMMARY")
    if len(conf_template_list) != num_of_confTemplates :
        print "Error: Not all conference templates were found in the list, found only "+ str(len(conf_template_list))+" Conf Templates"
        connection.Disconnect()                
        sys.exit("Can not find Conf Templates")
    print "All " + str(num_of_confTemplates) + " Conf Templates are still alive in monitoring list"
#------------------------------------------------------------------------------
def DeleteAllConfTemplates(connection):
    print "Delete all Conf Templates..."
    connection.SendXmlFile('Scripts/AddRemoveConfTemplate/TransConfTemplateList.xml',"Status OK")
    conf_template_list = connection.xmlResponse.getElementsByTagName("CONFERENCE_TEMPLATE_SUMMARY")
    for x in range(len(conf_template_list)):
        conf_template_id = conf_template_list[x].getElementsByTagName("ID")[0].firstChild.data
        connection.DelConfTemplate(conf_template_id,'Scripts/AddRemoveConfTemplate/DelConfTemplate.xml')
    return

#------------------------------------------------------------------------------
def WaitAllConfTemplatesEnd(connection,retires = 20):
    print "Waiting until all Conference Template end",
    for retry in range(retires+1):
        sleep(1)
        sys.stdout.write(".")
        sys.stdout.flush()
        connection.SendXmlFile('Scripts/AddRemoveConfTemplate/TransConfTemplateList.xml',"Status OK")
        if connection.GetTextUnder("CONFERENCE_TEMPLATE_SUMMARY","ID") == "":
            print
            break
        if retry == retires:
            connection.Disconnect()
            sys.exit("Failed delete Conference Template!!!")
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
isConfFromProfile=1
numOfConferenceTemplates=7
run_num=1
for using_profile in range(run_num):
    if using_profile ==0:
              print "Test Conf Templates with profile" 
              
    else:
              print "Test Conf Templates with profile id =-1"
              isConfFromProfile=0
    SimpleAddRemoveConfTemplateTest(c, #the connection class
                      numOfConferenceTemplates, # num ofConference Templates
                      'Scripts/AddRemoveConfTemplate/StartConfTemplate.xml',isConfFromProfile,
                      20) 
    sleep(3)
c.Disconnect()

