#!/mcms/python/bin/python

#############################################################################
# Create HD1080 Cop conference from 4CIF 16x9 COP default profile
# Connect participants to all the encoders by defined rate
# Change speaker in 3 different layout types
# Date: 03/11/09
# By  : Keren
#############################################################################

from McmsConnection import *
from CopFunctions import *


connection = McmsConnection()
connection.Connect()
confId  = CreateAndConnectPartiestoCOP4CIF16x9ConferenceFromDefaultProfile(connection)
sleep(12)
numParties = 8   
print "Start Changing Video Speaker when layout type = 1x1"
for x in range(numParties-1):
    connection.ChangeDialOutAudioSpeaker(confId, x)
    sleep(1)
    
connection.ChangeConfLayoutType(confId, "2and8")
#sleep(1000000000)
print "Start Changing Video Speaker when layout type = 2and8"
for x in range(numParties-1):
    connection.ChangeDialOutAudioSpeaker(confId, x)
    sleep(1)
       
connection.ChangeConfLayoutType(confId, "2x1")
print "Start Changing Video Speaker when layout type = 2x1"
for x in range(numParties-1):
    connection.ChangeDialOutAudioSpeaker(confId, x)
    sleep(1)
    
connection.WaitAllOngoingConnected(confId)##check that after all the actions still all are fully connected

print "Deleting Conf..." 
connection.DeleteConf(confId)   
connection.WaitAllConfEnd()   

