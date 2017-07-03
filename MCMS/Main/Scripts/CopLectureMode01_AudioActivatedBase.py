#!/mcms/python/bin/python
#-LONG_SCRIPT_TYPE

#############################################################################
# Create HD1080 Cop conference from HD1080 COP default profile
# Connect participants to all the encoders by defined rate
# Change the conference from same layout to lecture mode
# Switch between lecturers from 4 level 
# Date: 03/12/09
# By  : Keren
#############################################################################

from McmsConnection import *
from CopFunctions import *


connection = McmsConnection()
connection.Connect()
confId  = CreateAndConnectPartiestoCOP1080ConferenceFromDefaultProfile(connection)
connection.ChangeConfLayoutType(confId, "3x3")
connection.WaitAllOngoingConnected(confId)##check that after changeing layout all are still fully connected

print "~~~~~~"
partyName=""
isOn="true"
isTimer="true"
timerInterval="15"
isAudioActivate="true"
lectureModeType="lecture_mode"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
print "~~~~~~"
numParties = 8
 
for x in range(numParties-1):
    connection.ChangeDialOutVideoSpeaker(confId, x)
    sleep(3)
print "~~~~~~"


## sleep of 1 second will cause pending action
for x in range(numParties-1):
    connection.ChangeDialOutVideoSpeaker(confId, x)
    sleep(1)
print "~~~~~~"
    
## connection.ChangeConfLayoutType(confId, "2x2")
## print "Start Changing Video Speaker when layout type = 2x2"
## for x in range(numParties-1):
##     connection.ChangeDialOutVideoSpeaker(confId, x)
##     sleep(1)
       
## connection.ChangeConfLayoutType(confId, "1and7")
## print "Start Changing Video Speaker when layout type = 1and7"
## for x in range(numParties-1):
##     connection.ChangeDialOutVideoSpeaker(confId, x)
##     sleep(1)

## StartLectureMode(connection, confId, "COP1080Conference_SIP_ENCODER_1")
## connection.WaitAllOngoingConnected(confId)##check that after changeing lecturer all are still fully connected
## sleep(2)
## StartLectureMode(connection, confId, "COP1080Conference_H323_ENCODER_2")
## connection.WaitAllOngoingConnected(confId)##check that after changeing lecturer all are still fully connected
## sleep(2)
## StartLectureMode(connection, confId, "COP1080Conference_H323_ENCODER_3")
## connection.WaitAllOngoingConnected(confId)##check that after changeing lecturer all are still fully connected
## sleep(2)
## StartLectureMode(connection, confId, "COP1080Conference_SIP_ENCODER_4")
## connection.WaitAllOngoingConnected(confId)##check that after changeing lecturer all are still fully connected

sleep(2)
print "Deleting Conf..." 
connection.DeleteConf(confId)   
connection.WaitAllConfEnd()  



