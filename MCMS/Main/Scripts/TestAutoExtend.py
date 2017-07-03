#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#-LONG_SCRIPT_TYPE

#############################################################################
# Test Script which test the AutoExtension feature
#  
# Date: 16/07/06
# By  : Udi B.
#############################################################################

from McmsConnection import *

def SleepForXMinutes(connection,minutes,confName,num_retries):
    print "Sleeping for a total of: " + str(minutes) + " minutes"
    for i in range(minutes) :
        connection.WaitConfCreated(confName,num_retries)
        sleep(60)
        print "Sleeping already " + str(i+1) + " minutues"


            
def TestAutoExtend(connection,num_retries):

    #Set the debug flag fro 1 minute
    jumpingTime="2"
    os.popen("Bin/McuCmd set ConfParty EXTENSION_TIME_INTERVAL "+jumpingTime)
    
    #add a new profile
    profName = "Prof1"
    beforeFirstJoin = "1"
    timeAfterLastQuit = "3"
    connection.LoadXmlFile("Scripts/MoveUndef/CreateNewProfile.xml")
    connection.ModifyXml("RESERVATION","NAME",profName)
    connection.ModifyXml("AUTO_TERMINATE","TIME_BEFORE_FIRST_JOIN",beforeFirstJoin)
    connection.ModifyXml("AUTO_TERMINATE","TIME_AFTER_LAST_QUIT",timeAfterLastQuit)
    connection.Send()
    profId = connection.GetTextUnder("RESERVATION","ID")

    
    confName = "Conf1"
    hour="0"
    minutes="6"
    seconds="0"
    
    connection.LoadXmlFile("Scripts/MoveUndef/CreateNewConf.xml")
    connection.ModifyXml("RESERVATION","NAME",confName)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profId)
    connection.ModifyXml("DURATION","HOUR",hour)
    connection.ModifyXml("DURATION","MINUTE",minutes)
    connection.ModifyXml("DURATION","SECOND",seconds)
    connection.Send()

    confid = connection.WaitConfCreated(confName,num_retries)
    
    
    #Add 2 parties
    numOfParties = 2
    connection.AddVideoParty(confid,"Party1","1.1.1.1")
    connection.AddVideoParty(confid,"Party2","2.2.2.2")
    
    connection.WaitAllPartiesWereAdded(confid,numOfParties,num_retries)
    connection.WaitAllOngoingConnected(confid,num_retries)
    party1Id = connection.GetPartyId(confid,"Party1")
    party2Id = connection.GetPartyId(confid,"Party2")
        
    #Sleep for duration time 
    SleepForXMinutes(connection,int(minutes),confName,num_retries)
        
    #Make sure the conference is in the air and the duration was updated
    print "Make sure the conf is still alive"
    confid = connection.WaitConfCreated(confName,num_retries)

    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()

    newMinutesDuration = connection.GetTextUnder("DURATION","MINUTE")
    print
    print "New minutes duration from the reservation is: " + newMinutesDuration
    confExpectedDuration = int(minutes)+ int(jumpingTime) * 2
    print "Expected udration is: " + minutes+ " + " + jumpingTime + " + " + jumpingTime + " = " + str(confExpectedDuration)

    if newMinutesDuration != str(confExpectedDuration) :
        print "Error: Conf Duration is: " + newMinutesDuration + " ,while it should be: " + str(confExpectedDuration)
        sys.exit("Error: Conf Duration is: " + newMinutesDuration + " ,while it should be: " + str(confExpectedDuration))
    
    
    #disconnect the parties
    connection.DisconnectParty(confid,party1Id)
    connection.DisconnectParty(confid,party2Id)
    connection.WaitAllOngoingDisConnected(confid)

    #Sleep for  timeAfterLastQuit minutes
    print "Sleeping for: " + timeAfterLastQuit 
    SleepForXMinutes(connection,int(timeAfterLastQuit),confName,num_retries)
    
    #Make sure the Conference was deleted
    print "Party should be deleted by Auto terminate!"
    connection.WaitConfEnd(confid)

    #remove the profile
    connection.DelProfile(profId, "Scripts/MoveUndef/RemoveNewProfile.xml")

    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestAutoExtend(c,
                    30)# retries

c.Disconnect()


