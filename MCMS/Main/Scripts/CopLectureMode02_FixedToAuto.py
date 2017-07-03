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
connection.ChangeConfLayoutType(confId, "2x2")
connection.WaitAllOngoingConnected(confId)##check that after changeing layout all are still fully connected

print "~~~~~~~"
partyName=""
isOn="true"
isTimer="true"
timerInterval="15"
isAudioActivate="false"
lectureModeType="lecture_mode"
print "set auto lecture mode"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
connection.ChangeDialOutVideoSpeaker(confId, 2)
print "~~~~~~"
print "set fixed lecturer"
partyName="COP1080Conference_SIP_ENCODER_1"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
print "~~~~~~"
print "change fixed lecturer"
connection.ChangeConfLayoutType(confId, "1and7")
partyName="COP1080Conference_H323_ENCODER_1"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
print "~~~~~~"
print "set auto lecture mode"
connection.ChangeDialOutVideoSpeaker(confId, 4)
partyName=""
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
print "~~~~~~"
print "stop lecture mode"
partyName=""
isOn="false"
isTimer="false"
timerInterval="15"
isAudioActivate="false"
lectureModeType="lecture_none"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
print "~~~~~~"
print "re-start lecture mode"
partyName=""
isOn="true"
isTimer="true"
timerInterval="15"
isAudioActivate="false"
lectureModeType="lecture_mode"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
print "~~~~~~"



sleep(2)
print "Deleting Conf..." 
connection.DeleteConf(confId)   
connection.WaitAllConfEnd()  



