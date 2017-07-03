#!/mcms/python/bin/python

#############################################################################
# Test Script For HD
# In The Test:

# Date: 14/02/06
# By  : Ron
#############################################################################
from McmsConnection import *
from HDFunctions import *
import string 
#------------------------------------------------------------------------------

def TestHD_disconnect_lecturer(connection):
	
	print "TestHD_disconnect_lecturer"
	num_of_dial_out_parties = 3
	num_retries = 10
	conf_name = "HD_conf_1920"
	num_of_speaker_change = 3
	speaker_id_interval = 0
	
	## start conference
	prof_id = AddHdProfile(connection,"HD_profile_1920","1920", "Scripts/HD/XML/AddHdProfile.xml")
	connection.CreateConfFromProfile(conf_name, prof_id)
	conf_id = connection.WaitConfCreated(conf_name,num_retries)
	
	## connect participants
	AddDialOutHDParticipants(connection,conf_id,num_of_dial_out_parties)
	sleep(5)
	
	connection.WaitAllOngoingNotInIVR(conf_id)
	
	# set speaker to first party
	parties_list = GetPartiesInfo(connection,conf_id)
	next_speaker_id = int(parties_list[0][0])
	print "next_speaker_id " + str(next_speaker_id)
	connection.ChangeDialOutVideoSpeaker(conf_id, (next_speaker_id + speaker_id_interval))
	sleep(1)
	# change speaker to second party
	next_speaker_id = int(parties_list[1][0])
	print "next_speaker_id " + str(next_speaker_id)
	connection.ChangeDialOutVideoSpeaker(conf_id, (next_speaker_id + speaker_id_interval))
	sleep(1)
	CheckConfVswLayout(connection,conf_id,int(parties_list[1][0]),int(parties_list[0][0]))
	sleep(1)
	# (1) set third party lecture mode (timer off)
	lecturer_id = int(parties_list[2][0])
	StartLectureMode(connection, conf_id, parties_list[2][1],"false","15")
	sleep(1)
	CheckConfVswLayout(connection,conf_id,lecturer_id,next_speaker_id)
	
	
	# disconnect lecturer
	connection.DisconnectParty(conf_id,lecturer_id)
	
	sleep(1)
	

	connection.DelProfile(prof_id)
	connection.DeleteConf(conf_id)
	connection.WaitAllConfEnd()
	
	
def TestHD_disconnect_source_of_conf_video_sorce(connection):

	print "TestHD_disconnect_source_of_conf_video_sorce"
	num_of_dial_out_parties = 3
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
	
	# set speaker to first party
	parties_list = GetPartiesInfo(connection,conf_id)
	next_speaker_id = int(parties_list[0][0])
	print "next_speaker_id " + str(next_speaker_id)
	connection.ChangeDialOutVideoSpeaker(conf_id, next_speaker_id)
	
	# change speaker to second party
	next_speaker_id = int(parties_list[1][0])
	print "next_speaker_id " + str(next_speaker_id)
	connection.ChangeDialOutVideoSpeaker(conf_id, next_speaker_id)
		
	# disconnect first party
	connection.DisconnectParty(conf_id,int(parties_list[0][0]))
	
	sleep(1)
	

	connection.DelProfile(prof_id)
	connection.DeleteConf(conf_id)
	connection.WaitAllConfEnd()
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
TestHD_disconnect_lecturer(c)
sleep(2)
TestHD_disconnect_source_of_conf_video_sorce(c)
c.Disconnect()
#------------------------------------------------------------------------------  
