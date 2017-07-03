#!/mcms/python/bin/python

#############################################################################
# Create HD1080 Cop conference from 4CIF 4x3 COP default profile
# Connect participants to all the encoders by defined rate
# Change speaker in 3 different layout types
# Date: 03/11/09
# By  : Keren
#############################################################################

from McmsConnection import *
from CopFunctions import *


connection = McmsConnection()
connection.Connect()
confId  = CreateAndConnectPartiestoCOP4CIF4x3ConferenceFromDefaultProfile(connection)
numParties = 8   
print "Start Changing Video Speaker when layout type = 1x1"
for x in range(numParties-1):
    connection.ChangeDialOutVideoSpeaker(confId, x)
    sleep(1)
    
connection.ChangeConfLayoutType(confId, "1and4Ver")
print "Start Changing Video Speaker when layout type = 1and4Ver"
for x in range(numParties-1):
    connection.ChangeDialOutVideoSpeaker(confId, x)
    sleep(1)
       
connection.ChangeConfLayoutType(confId, "1x2")
print "Start Changing Video Speaker when layout type = 1x2"
for x in range(numParties-1):
    connection.ChangeDialOutVideoSpeaker(confId, x)
    sleep(1)

connection.WaitAllOngoingConnected(confId)##check that after all the actions still all are fully connected

print "Deleting Conf..." 
connection.DeleteConf(confId)   
connection.WaitAllConfEnd()  