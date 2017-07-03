#!/mcms/python/bin/python

#############################################################################
#Script which tests the read DB of reservations feature
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-- SKIP_ASSERTS

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

profId = c.AddProfile("profile_for_reservations")
print "-----------------------------------------------------------"
num_of_occurences = 10
print "Starting recurrences " + str(num_of_occurences) 
repeated_id = c.CreateDailyRepeatedRes("repeated_test", profId, num_of_occurences) 
sleep(5) 
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list, (after adding the repeated reservations) is " + str(res_number)
if res_number != num_of_occurences:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Not all reservations were created")

print "Adding one additional reservation"
t = datetime.utcnow( ) 
deltat = timedelta(0,0,0,0,30,1,0)
t = t + deltat
print t   
c.CreateRes("test1", profId, t)   
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list, (after adding the additional reservation) is " + str(res_number)
if res_number != num_of_occurences + 1:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Not all reservations were created")
   
print "-----------------------------------------------------------"
c.Disconnect()
print "Resetting"
os.system("Scripts/Destroy.sh")
os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")
print "-----------------------------------------------------------"
c.Connect()
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list, (after reset) is " + str(res_number)
if res_number != res_number:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Not all reservations were created")
print "-----------------------------------------------------------"
c.Disconnect()

