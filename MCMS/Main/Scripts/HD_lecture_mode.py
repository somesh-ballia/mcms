#!/mcms/python/bin/python

#############################################################################
# Test Script For HD
# In The Test:

# Date: 07/12/06
# By  : Ron
#############################################################################
from McmsConnection import *
from HDFunctions import *
import string 

#*PROCESSES_NOT_FOR_VALGRIND=ConfParty

#------------------------------------------------------------------------------

def TestHD_lecture_mode(connection):

	num_of_dial_out_parties = 10
	num_retries = 10
	conf_name = "HD_conf_1920"
	num_of_speaker_change = 3
	
	## start conference
	prof_id = AddHdProfile(connection,"HD_profile_1920","1920", "Scripts/HD/XML/AddHdProfile.xml")
	connection.CreateConfFromProfile(conf_name, prof_id)
	conf_id = connection.WaitConfCreated(conf_name,num_retries)
	
	## connect participants
	AddDialOutHDParticipants(connection,conf_id,num_of_dial_out_parties)
	sleep(5)
	connection.WaitAllOngoingNotInIVR(conf_id)
	# set speaker
	parties_list = GetPartiesInfo(connection,conf_id)
	next_speaker_id = int(parties_list[0][0])
	print "next_speaker_id " + str(next_speaker_id)
	connection.ChangeDialOutVideoSpeaker(conf_id, next_speaker_id)
	
	# (1) set first lecture mode (timer off)
	lecturer_id = int(parties_list[4][0])
	StartLectureMode(connection, conf_id, parties_list[4][1],"false","15")
	sleep(3)
	CheckConfVswLayout(connection,conf_id,lecturer_id,next_speaker_id)
	
	speaker_index = 0
	next_speaker_index = 1
	
	# (2) change speakers and check all parties sees lecturer, lecturer sees last speaker
	for change_layout_index in range (num_of_speaker_change):
		speaker_index = next_speaker_index
		next_speaker_index += 1
		if(next_speaker_index >= len(parties_list)):
			next_speaker_index = 0
		current_speaker_id = next_speaker_id
		next_speaker_id = int(parties_list[next_speaker_index][0])
		print "\n"+"change speaker to party: " + parties_list[next_speaker_index][1] + ", monitoring id: " + str(next_speaker_id)
		connection.ChangeDialOutVideoSpeaker(conf_id, next_speaker_id)
		sleep(3)
		if(lecturer_id!=next_speaker_id):
			CheckConfVswLayout(connection,conf_id,lecturer_id,next_speaker_id)
		else:
			CheckConfVswLayout(connection,conf_id,lecturer_id,current_speaker_id)
		
	# (3) set conf level force and check all parties sees lecturer, lecturer sees forced party
	force_party_index = 2
	force_party_id = int(parties_list[force_party_index][0])
	print "\n"+"change force to party: " + parties_list[force_party_index][1] + ", monitoring id: " + str(force_party_id)
	SetConfLevelForce(connection, conf_id, "1x1", force_party_id, 0)
	sleep(3)
	print "lecturer_id = " + str(lecturer_id) + ", force_party_id = " + str(force_party_id) + ", next_speaker_id = " + str(next_speaker_id) + ", current_speaker_id = " + str(current_speaker_id) 
	if (lecturer_id!=force_party_id):
		CheckConfVswLayout(connection,conf_id,lecturer_id,force_party_id)
	elif (lecturer_id!=next_speaker_id):
		CheckConfVswLayout(connection,conf_id,lecturer_id,next_speaker_id)
	else:
		CheckConfVswLayout(connection,conf_id,lecturer_id,current_speaker_id)
	
	# (4) remove conf level force and check all parties sees lecturer, lecturer sees last speaker
	print "\n"+"Removing conf level force"
	RemoveConfLevelForce(connection, conf_id, "1x1", 0)	
	sleep(3)
	print "lecturer_id = " + str(lecturer_id) + ", force_party_id = " + str(force_party_id) + ", next_speaker_id = " + str(next_speaker_id) + ", current_speaker_id = " + str(current_speaker_id) 
	if(lecturer_id!=next_speaker_id):
		CheckConfVswLayout(connection,conf_id,lecturer_id,next_speaker_id)
	else:
		CheckConfVswLayout(connection,conf_id,lecturer_id,current_speaker_id)




	connection.DelProfile(prof_id)
	connection.DeleteConf(conf_id)
	connection.WaitAllConfEnd()
	
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestHD_lecture_mode(c)
c.Disconnect()
#------------------------------------------------------------------------------  
