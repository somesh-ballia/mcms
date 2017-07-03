#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

from McmsConnection import *

c = McmsConnection()
c.Connect()


#Add maximum profiles (20)
profileMaxNumber=19
profIdList= list()

for x in range(profileMaxNumber):
    profIdList.append(c.AddProfile("profile"+str(x)))
    sleep(1)

delProfileCounter = 4
print "Deleting: " + str(delProfileCounter) + " for testing"
for x in range(delProfileCounter):
    c.DelProfile(profIdList[0])
    profIdList.remove(profIdList[0])
    sleep(1)

print "Adding the " + str(delProfileCounter) + " deleteed profiles"
for x in range(delProfileCounter):
    profIdList.append(c.AddProfile("profile2_"+str(x)))
    sleep(1)
    
#Deleting all profiles
for x in range(profileMaxNumber):
    c.DelProfile(profIdList[0])
    profIdList.remove(profIdList[0])
    sleep(1)
    
c.Disconnect()

