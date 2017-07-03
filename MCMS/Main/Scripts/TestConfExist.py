#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1


from McmsConnection import *


## ---------------------- Test --------------------------
def AddProfile(connection,profileName,status='Status OK'):
    print "Adding new Profile..."
    connection.LoadXmlFile("Scripts/CreateNewProfile.xml")
    connection.ModifyXml("RESERVATION","NAME", profileName)
    connection.Send(status)
    ProfId = connection.GetTextUnder("RESERVATION","ID")
    print "Profile, named: " + profileName + " ,ID = " + ProfId + ", is added"
    return ProfId

def CreateConf(connection,confName,status='Status OK'):
    print "Adding Conf " + confName + " ..."
    connection.LoadXmlFile('Scripts/AddCpConf.xml')
    connection.ModifyXml("RESERVATION","NAME",confName)
    connection.Send(status)
    
def CreateMR(connection, mrName, ProfId,status,fileName):
    print "Adding a new MR-" + mrName + " Reservation..."
    connection.LoadXmlFile(fileName)
    connection.ModifyXml("RESERVATION","NAME",mrName)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(ProfId))
    connection.Send(status)
    
def TestConfExist(connection,num_retries):
    #Add a new EQ service
    print "Adding a new Entry Queue Service...."
    connection.SendXmlFile("Scripts/CreateAdHoc3BlastUndefParties/AddEqService.xml","Status OK")


    #Add a profile with name Prof1
    porfile1Name = "Profile1"
    profileId = AddProfile(connection,porfile1Name)

    #try to add the same profile
    AddProfile(connection,porfile1Name,"Profile name already exists")
    
    #create a conference with the same name as profile name
    CreateConf(connection,porfile1Name)
    confId=connection.WaitConfCreated(porfile1Name)
    connection.DeleteConf(confId)
    connection.WaitConfEnd(confId)
    
    #Create a MR with the name MR1
    MR1Name ="MR1"
    CreateMR(connection, MR1Name,profileId,'Status OK',"Scripts/CreateMR.xml")
    mrId,numericId = connection.WaitMRCreated(MR1Name)
    
    #Try Create a conference with the name MR1
    CreateConf(connection,MR1Name,'Conference name already exists in Meeting Rooms list')

    #Try to create an EQ with the smae name
    CreateMR(connection ,MR1Name ,profileId,'Conference name already exists',"Scripts/CreateAdHoc3BlastUndefParties/CreateNewEq.xml")
    
    #Create an EQ  with the name EQ1
    EQ1Name="EQ1"
    CreateMR(connection ,EQ1Name ,profileId,'Status OK',"Scripts/CreateAdHoc3BlastUndefParties/CreateNewEq.xml")
    eqId,numericId = connection.WaitMRCreated(EQ1Name)
    
    #Try to create a conference with EQ1 name
    CreateConf(connection,EQ1Name,'Conference name already exists in Meeting Rooms list')

    #create another conference with the name Conf1
    CreateConf(connection,'Conf1')
    confId1=connection.WaitConfCreated('Conf1')

    #Try to create a conference with the same name
    CreateConf(connection,'Conf1','Conference name already exists')
    
    #Create a conference with the name MrOrEQ
    CreateConf(connection,'MrOrEQ')
    confId2=connection.WaitConfCreated('MrOrEQ')
    
    #try to create a MR with the name of MrOrEQ
    CreateMR(connection,'MrOrEQ',profileId,'Conference name already exists',"Scripts/CreateMR.xml")
             
    #try to create an EQ with the name of MrOrEQ
    CreateMR(connection ,'MrOrEQ' ,profileId,'Conference name already exists',"Scripts/CreateAdHoc3BlastUndefParties/CreateNewEq.xml")

    #Create a profile with the name MrOrEQ
    profileId2 = AddProfile(connection,'MrOrEQ')
    
    connection.DelReservation(mrId, "Scripts/AwakeMrByUndef/RemoveMr.xml")
    connection.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")
        
    connection.DeleteConf(confId1)
    connection.DeleteConf(confId2)
    connection.DelProfile(profileId)
    connection.DelProfile(profileId2)
    connection.WaitAllConfEnd(num_retries)
    
    
c = McmsConnection()
c.Connect()

TestConfExist(c,
                    30)# retries

c.Disconnect()
