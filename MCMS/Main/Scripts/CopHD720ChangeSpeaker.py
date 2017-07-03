#!/mcms/python/bin/python

#############################################################################
# Create HD1080 Cop conference from HD720 COP default profile
# Connect participants to all the encoders by defined rate
# Change speaker in 3 different layout types
# Date: 03/11/09
# By  : Keren
#############################################################################

from McmsConnection import *
from CopFunctions import *


connection = McmsConnection()
connection.Connect()
confId  = CreateAndConnectPartiestoCOP720ConferenceFromDefaultProfile(connection)
print "ConfID = ", confId
numParties = 8   
print "Start Changing Video Speaker when layout type = 1x1"
for x in range(numParties-1):
    connection.ChangeDialOutVideoSpeaker(confId, x)
    sleep(1)
    
connection.ChangeConfLayoutType(confId, "3x3")
print "Start Changing Video Speaker when layout type = 3x3"
for x in range(numParties-1):
    connection.ChangeDialOutVideoSpeaker(confId, x)
    sleep(1)
       
connection.ChangeConfLayoutType(confId, "1and8Central")
print "Start Changing Video Speaker when layout type = 1and8Central"
for x in range(numParties-1):
    connection.ChangeDialOutVideoSpeaker(confId, x)
    sleep(1)
    
connection.WaitAllOngoingConnected(confId)##check that after all the actions still all are fully connected

print "Deleting Conf..." 
connection.DeleteConf(confId)   
connection.WaitAllConfEnd()  

