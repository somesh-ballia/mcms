#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

#############################################################################
# 1. Try to create a single reservation "TestReserv" with dial-in phone number 3344 => OK
# 2. Update "TestReserv" reservation: Disable its ISDN Access => this should be OK
# 3. Update "TestReserv" reservation: Enable its ISDN Access and change dial-in phone number to be 3345 => OK
# 4. Create a repeated reservation (daily, 5 entries) with Enable ISDN Access and phone number 3344 => OK
# 5. reset MCU => all reservations should be restored 
# 6. Update the "TestReserv" reservation: try to change phone number to 3344 (but it's reserved by repeated) => FAILURE
#############################################################################

from ResourceUtilities import *
import os

connection = ResourceUtilities()
connection.Connect()


phoneNumberRsrv ="3344"
phoneNumberConf ="3345"

profId = connection.AddProfile("profile_for_reservations")

t = datetime.utcnow( ) 
deltat = timedelta(0,0,0,0,0,24,0)
t1 = t + deltat

print "-----------------------------------------------------------"
print "Adding a reservation with numeric id (1234) and dial-in phone number " + phoneNumberRsrv
res_id = connection.CreateRes("TestReserv", profId, t1, "1234", 0, 0, "Status OK", 0, "Scripts/AddRemoveReservation/StartRes.xml", phoneNumberRsrv)
sleep(2)

print "-----------------------------------------------------------"  
print "Trying to update reservation - changing \"Enable ISDN Access\" to be FALSE - this should be OK"
connection.UpdateReservation("TestReserv",res_id,"1234",profId,t1,0,0,"Status OK", "false")
sleep(2)

print "-----------------------------------------------------------"  
print "Trying to update reservation - changing it's phone number from 3344 to 3345 - this should be OK"
connection.UpdateReservation("TestReserv",res_id,"1234",profId,t1,0,0,"Status OK", "true", phoneNumberConf)
sleep(2)

print "-----------------------------------------------------------"
print "Starting recurrences, with one that should start immediately"
num_of_occurences = 5
repeated_id = connection.CreateDailyRepeatedRes("repeated_test", profId, num_of_occurences, 0, "Scripts/AddRemoveReservation/AddRepeatedRes.xml", "true", phoneNumberRsrv)  
confid = connection.WaitConfCreated("repeated_test_00001")  

sleep(10)

print "Resetting MCU in order for new configuration to take effect"
connection.Disconnect()
os.environ["CLEAN_CFG"]="NO"
os.environ["RESOURCE_SETTING_FILE"]=""
os.system("Scripts/Destroy.sh")
os.system("Scripts/Startup.sh")
connection.Connect()

sleep(10)

print "-----------------------------------------------------------"  
print "Trying to update reservation - changing it's phone number from 3345 to 3344 - this should FAIL"
expected_status = "ISDN dial-in number is already assigned to another conferencing entity"
connection.UpdateReservation("TestReserv",res_id,"1234",profId,t1,0,0, expected_status, "true", phoneNumberRsrv)
sleep(2)

print "-----------------------------------------------------------"  
print "Delete the reservation"
connection.DelConfReservation(res_id)

connection.DelRepeatedReservation(repeated_id)   
sleep(5)

#remove the profile
#connection.DelProfile(profId) #temp due the problem of profile deleting (return status is "Default Profile cannot be deleted")

connection.Disconnect()
