#!/mcms/python/bin/python

#############################################################################
#Script which tests the repeated reservations feature
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-- SKIP_ASSERTS

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

profId = c.AddProfile("profile_for_recurrence")
print "-----------------------------------------------------------"
c.CreateAndThenDeleteRepeated(profId, 10)
print "-----------------------------------------------------------"
c.CreateAndThenDeleteRepeated(profId, 10)
print "-----------------------------------------------------------"
c.CreateAndThenDeleteRepeated(profId, 10)
print "-----------------------------------------------------------"
c.CreateAndThenDeleteRepeated(profId, 10)
print "-----------------------------------------------------------"
c.CreateAndThenDeleteRepeated(profId, 10)
print "-----------------------------------------------------------"
print "Starting recurrences, with one that should start immediately"
num_of_occurences = 10
repeated_id = c.CreateDailyRepeatedRes("repeated_test_immediately", profId, num_of_occurences, 0)  
confid = c.WaitConfCreated("repeated_test_immediately_00001")  
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list, (after adding the repeated reservations) is " + str(res_number)
if res_number != num_of_occurences - 1 :
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Not all reservations were created")
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list, (after adding the repeated reservations) is " + str(conf_number)
if conf_number != 1 :
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
c.DelRepeatedReservation(repeated_id)   
sleep(5)
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list, (after deleting the repeated reservations) is " + str(res_number)
if res_number != 0 :
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Not all reservations were deleted")
c.DeleteConf(confid)
c.WaitConfEnd(confid)
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list, (after deleting the repeated reservations) is " + str(conf_number)
if conf_number != 0 :
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
print "-----------------------------------------------------------"
print "Starting recurrences, with one that should start in a minute"
num_of_occurences = 10
repeated_id = c.CreateDailyRepeatedRes("repeated_test_in_a_minute", profId, num_of_occurences, 1)
sleep(5)
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list, (after adding the repeated reservations) is " + str(res_number)
if res_number != num_of_occurences :
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Not all reservations were created")
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list, (after adding the repeated reservations) is " + str(conf_number)
if conf_number != 0 :
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
print "Waiting a little more than a minute for the reservation to start"   
sleep(70)
c.Connect()
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != num_of_occurences - 1:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list is " + str(conf_number)
if conf_number != 1 :
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
print "-----------------------------------------------------------"

c.Disconnect()

