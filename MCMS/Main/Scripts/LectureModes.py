#!/mcms/python/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

from McmsConnection import *


#------------------------------------------------------------------------------
def ChangeLecturer(connection,confid, partyName):
    print "Conference ID: "+ confid + " New Lecturer: " + partyName
    connection.LoadXmlFile('Scripts/PresentationMode/UpdateLectureMode.xml')
    connection.ModifyXml("SET_LECTURE_MODE","ID",confid)
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_NAME",partyName) 
    connection.ModifyXml("SET_LECTURE_MODE","ON","true")  
    connection.ModifyXml("SET_LECTURE_MODE","TIMER","true")  
    connection.ModifyXml("SET_LECTURE_MODE","INTERVAL","15")  
    connection.ModifyXml("SET_LECTURE_MODE","AUDIO_ACTIVATED","false")
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_MODE_TYPE","lecture_mode")
    connection.Send()
    return

#------------------------------------------------------------------------------
def StartLectureMode(connection,confid, partyName):
    print "Conference ID: "+ confid + " New Lecturer: " + partyName
    connection.LoadXmlFile('Scripts/PresentationMode/UpdateLectureMode.xml')
    connection.ModifyXml("SET_LECTURE_MODE","ID",confid)
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_NAME",partyName)  
    connection.ModifyXml("SET_LECTURE_MODE","ON","true")  
    connection.ModifyXml("SET_LECTURE_MODE","TIMER","true")  
    connection.ModifyXml("SET_LECTURE_MODE","INTERVAL","15")  
    connection.ModifyXml("SET_LECTURE_MODE","AUDIO_ACTIVATED","false")
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_MODE_TYPE","lecture_mode")
    connection.Send()
    return

#------------------------------------------------------------------------------
def StopLectureMode(connection,confid):
    print "Conference ID: "+ confid + " Stopping Lecture Mode "
    connection.LoadXmlFile('Scripts/PresentationMode/UpdateLectureMode.xml')
    connection.ModifyXml("SET_LECTURE_MODE","ID",confid)
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_ID","-1") 
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_NAME","")  
    connection.ModifyXml("SET_LECTURE_MODE","ON","false")  
    connection.ModifyXml("SET_LECTURE_MODE","TIMER","false")  
    connection.ModifyXml("SET_LECTURE_MODE","INTERVAL","15")  
    connection.ModifyXml("SET_LECTURE_MODE","AUDIO_ACTIVATED","false")
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_MODE_TYPE","lecture_none")
    connection.Send()
    return

 #------------------------------------------------------------------------------ 
c = McmsConnection()
c.Connect()

delay = 2; 
if(c.IsProcessUnderValgrind("ConfParty")):
    delay = 5;    

print "Adding Conf..."
status = c.SendXmlFile('Scripts/PresentationMode/AddVideoCpConfLectureMode.xml')
        
print "Wait untill Conf create...",
num_retries = 50
for retry in range(num_retries+1):
    status = c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
    confid = c.GetTextUnder("CONF_SUMMARY","ID")
    if confid != "":
        print
        break
    if (retry == num_retries):
        print c.xmlResponse.toprettyxml()
        c.Disconnect()                
        c.exit("Can not monitor conf:" + status)
    sys.stdout.write(".")
    sys.stdout.flush()

#connect parties
print "Start connecting Parties..."
num_of_parties = 4   
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    partyip =  "1.2.3." + str(x+1)
    c.AddVideoParty(confid, partyname, partyip)
    print "current number of connected parties = " + str(x+1)
    sleep(3)

c.WaitAllOngoingNotInIVR(confid,num_retries)
sleep(delay)

for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    iPartyId = int(c.GetPartyId(confid, partyname))
    print "Party Name = " + partyname + ";  Id = " + str(iPartyId)
    sleep(5)

confLayoutType = "2x2"
currentLecturer = 0
c.CheckValidityOfLectureMode(confid, currentLecturer, confLayoutType, num_retries)

print
print "Start Test in ConfLayout 1and5..."    
confLayoutType = "1and5"
c.ChangeConfLayoutType(confid, confLayoutType)
c.CheckValidityOfLectureMode(confid, currentLecturer, confLayoutType, num_retries)

print
print "Test the Lecture Mode Timer Changing Video Speaker to be lecturer"
c.ChangeDialOutVideoSpeaker(confid, currentLecturer)
for x in range(num_of_parties): 
    c.WaitPartySeesPartyInCell(confid, currentLecturer, num_of_parties-x, 0)
    sleep(15);

print
print "Test Update Different Lecturer"
currentLecturer = 2
currentLecturerName = "Party"+str(currentLecturer)
ChangeLecturer(c, confid, currentLecturerName)
c.CheckValidityOfLectureMode(confid, currentLecturer, confLayoutType, num_retries)

print
print "Test Stop Lecture Mode"
StopLectureMode(c, confid)
c.WaitAllOngoingChangedLayoutType(confid, confLayoutType)

print
print "Test Start Lecture Mode"
currentLecturer = 1
currentLecturerName = "Party"+str(currentLecturer)
StartLectureMode(c, confid, currentLecturerName)
c.CheckValidityOfLectureMode(confid, currentLecturer, confLayoutType, num_retries)
    
print    
print "Start disconnecting Parties..."
for x in range(num_of_parties-1):
    c.DeleteParty(confid, x)
   
print "Deleting Conf..." 
c.DeleteConf(confid)   
c.WaitAllConfEnd()

c.Disconnect()
