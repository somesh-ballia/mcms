#!/mcms/python/bin/python

#############################################################################
# Create 4CIF 4x3 Cop conference from HD720 COP default profile
# Connect participants to all the encoders by defined rate
# Change all layout types 
# Date: 03/11/09
# By  : Keren
#############################################################################

from McmsConnection import *
from CopFunctions import *


connection = McmsConnection()
connection.Connect()
confId  = CreateAndConnectPartiestoCOP4CIF4x3ConferenceFromDefaultProfile(connection)
for layoutType in availableLayoutTypes:
        connection.ChangeConfLayoutType(confId, layoutType)
        WaitConfLayoutUpdated(connection,confId,layoutType)

connection.WaitAllOngoingConnected(confId)##check that after all the actions still all are fully connected

print "Deleting Conf..." 
connection.DeleteConf(confId)   
connection.WaitAllConfEnd()          
