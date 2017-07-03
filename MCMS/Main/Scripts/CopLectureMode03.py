#!/mcms/python/bin/python
#-LONG_SCRIPT_TYPE

#############################################################################
# Create HD720 Cop conference from HD720 COP default profile
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
confId  = StartImmediatlyPartiestoCOP720ConferenceFromDefaultProfile(connection)
connection.ChangeConfLayoutType(confId, "3x3")
connection.WaitAllOngoingConnected(confId)
connection.WaitAllOngoingNotInIVR(confId)
numParties = 8   
print "~~~~~~"
partyName="COP720Conference_SIP_ENCODER_1"
isOn="true"
isTimer="true"
timerInterval="15"
isAudioActivate="false"
lectureModeType="lecture_mode"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
sleep(1)
print "~~~~~~"
for x in range(numParties-1):
    connection.ChangeDialOutVideoSpeaker(confId, x)
    sleep(1)
print "~~~~~~"
print "set auto lecture mode"
connection.ChangeDialOutVideoSpeaker(confId, 4)
partyName=""
isAudioActivate="true"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
print "~~~~~~"
for x in range(numParties-1):
    connection.ChangeDialOutVideoSpeaker(confId, x)
    sleep(1)
print "~~~~~~"
print "set fixed lecturer"
partyName="COP720Conference_H323_ENCODER_1"
isAudioActivate="false"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
print "~~~~~~"
print "change fixed lecturer"
connection.ChangeConfLayoutType(confId, "1and7")
partyName="COP720Conference_H323_ENCODER_1"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
print "~~~~~~"
print "set auto lecture mode"
connection.ChangeDialOutVideoSpeaker(confId, 4)
partyName=""
isAudioActivate="true"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
print "~~~~~~"
for x in range(numParties-1):
    connection.ChangeDialOutVideoSpeaker(confId, x)
    sleep(1)
print "~~~~~~"
print "change fixed lecturer"
connection.ChangeConfLayoutType(confId, "1and5")
partyName="COP720Conference_H323_ENCODER_3"
isAudioActivate="false"
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
partyName="COP720Conference_SIP_ENCODER_1"
#partyName="COP720Conference_H323_ENCODER_1"
isOn="true"
isTimer="true"
timerInterval="15"
isAudioActivate="false"
lectureModeType="lecture_mode"
SetLectureMode(connection,confId,partyName,isOn,isTimer,timerInterval,isAudioActivate,lectureModeType)
sleep(1)
print "~~~~~~"



sleep(5)
print "Deleting Conf..." 
connection.DeleteConf(confId)   
connection.WaitAllConfEnd()  



