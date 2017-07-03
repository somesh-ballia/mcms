#!/mcms/python/bin/python

#############################################################################
#Script which tests the reservation capacity
#NOTE: this script should NOT run with valgrind (it will take too long to make all reservations)
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#-- SKIP_ASSERTS
#-LONG_SCRIPT_TYPE

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

profId = c.AddProfile("profile")

c.DeleteAllMR()
c.CheckNumOfMRsInList(0)

print "============================================================"
print "Creating 900 meeting rooms"
num_of_mr = 900
for mr_num in range(num_of_mr):
    mrName = "MR"+str(mr_num+1)
    NID = str(4000+mr_num+1)
    c.LoadXmlFile("Scripts/CreateMR.xml")
    c.ModifyXml("RESERVATION","NAME",mrName)
    c.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(profId))
    c.AddXML("RESERVATION","NUMERIC_ID",NID)   
    c.Send()
    print "MR, named: " + mrName + ", is added"
c.CheckNumOfMRsInList(num_of_mr)
print "============================================================"
print "Creating 2 repeated reservations with 1000 occurrences (total of 2000 reservations)"
num_of_occurences = 1000
repeated_id1 = c.CreateDailyRepeatedRes("repeated_test1", profId, num_of_occurences, 30)
sleep(20)
c.Connect()
c.CheckNumOfReservationsInList(1000)
repeated_id2 = c.CreateDailyRepeatedRes("repeated_test2", profId, num_of_occurences, 30)
sleep(20)
c.Connect()
c.CheckNumOfReservationsInList(2000)
print "============================================================"
print "Creating another 100 meeting rooms"
num_of_mr = 100
for mr_num in range(num_of_mr):
    mrName = "MR2_"+str(mr_num+1)
    NID = str(6000+mr_num+1)
    c.LoadXmlFile("Scripts/CreateMR.xml")
    c.ModifyXml("RESERVATION","NAME",mrName)
    c.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(profId))  
    c.AddXML("RESERVATION","NUMERIC_ID",NID)   
    c.Send()
    print "MR, named: " + mrName + ", is added"
c.CheckNumOfMRsInList(1000)
print "============================================================"
print "Check that no other MR can be added"   
mrName = "MR_test"
c.LoadXmlFile("Scripts/CreateMR.xml")
c.ModifyXml("RESERVATION","NAME",mrName)
c.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(profId))  
c.AddXML("RESERVATION","NUMERIC_ID","1000")   
c.Send("Maximum number of conferences exceeded")
print "Indeed didn't succeed to add another MR"
c.CheckNumOfMRsInList(1000)
print "============================================================"
print "Check that no other reservation can be added"   
t = datetime.utcnow( ) 
deltat = timedelta(0,0,0,0,30,1,0)
t = t + deltat
c.CreateRes("test", profId, t, "1234", 0, 0, "Maximum number of conferences exceeded")
print "Indeed didn't succeed to add another reservation"
c.CheckNumOfReservationsInList(2000)
print "============================================================"
print "Reset and check that everything is as expected"
c.Disconnect()
os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Destroy.sh")
os.system("Scripts/Startup.sh")
#c.Connect()
#sleep a while because reading of reservations is not a pre-requisite for getting out of startup
sleep(30)
c.Connect()
c.CheckNumOfMRsInList(1000)
c.CheckNumOfReservationsInList(2000)
print "============================================================"
print "Check again that no other MR can be added"   
mrName = "MR_test"
c.LoadXmlFile("Scripts/CreateMR.xml")
c.ModifyXml("RESERVATION","NAME",mrName)
c.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(profId))  
c.AddXML("RESERVATION","NUMERIC_ID","1000")   
c.Send("Maximum number of conferences exceeded")
print "Indeed didn't succeed to add another MR"
c.CheckNumOfMRsInList(1000)
print "============================================================"
print "Check again that no other reservation can be added"   
t = datetime.utcnow( ) 
deltat = timedelta(0,0,0,0,30,1,0)
t = t + deltat
c.CreateRes("test", profId, t, "1234", 0, 0, "Maximum number of conferences exceeded")
print "Indeed didn't succeed to add another reservation"
c.CheckNumOfReservationsInList(2000)
print "============================================================"








c.Disconnect()
