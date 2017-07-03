#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE

#############################################################################
# Test Script For HD speaker change
# In The Test:
#
# Add profile - HD at 1920 rate
# Start conference from that profile
# Connect 20 dial out participants
# Repeat 60 times:
#     change speaker, wait 1 second, check that conf sees new speaker and new speaker sees previous speaker
# Delete conf and profile
#
# Date: 07/12/06
# By  : Ron
#############################################################################
from McmsConnection import *
from HDFunctions import *
import string 
#------------------------------------------------------------------------------

def TestHD_Speaker_change(connection):

	num_of_dial_out_parties = 20
	num_retries = 10
	conf_name = "HD_conf_1920"
	num_of_speaker_change = 60
	
	## start conference
	prof_id = AddHdProfile(connection,"HD_profile_1920","1920", "Scripts/HD/XML/AddHdProfile.xml")
	connection.CreateConfFromProfile(conf_name, prof_id)
	conf_id = connection.WaitConfCreated(conf_name,num_retries)
	
	## connect participants
	AddDialOutHDParticipants(connection,conf_id,num_of_dial_out_parties)
	sleep(5)
	connection.WaitAllOngoingNotInIVR(conf_id)
	sleep(2)
	
	parties_list = GetPartiesInfo(connection,conf_id)
	
	next_speaker_id = int(parties_list[0][0])
	print "next_speaker_id " + str(next_speaker_id)
	connection.ChangeDialOutVideoSpeaker(conf_id, next_speaker_id)
	
	speaker_index = 0
	next_speaker_index = 1
	
	for change_layout_index in range (num_of_speaker_change):
		speaker_index = next_speaker_index
		next_speaker_index += 1
		if(next_speaker_index >= len(parties_list)):
			next_speaker_index = 0
		current_current_speaker_id = next_speaker_id
		next_speaker_id = int(parties_list[next_speaker_index][0])
		print "\n"+"change speaker to party: " + parties_list[next_speaker_index][1] + ", monitoring id: " + str(next_speaker_id)
		connection.ChangeDialOutVideoSpeaker(conf_id, next_speaker_id)
		sleep(2)
		CheckConfLayout(connection,conf_id,next_speaker_id)
		CheckConfSourceLayout(connection,conf_id,next_speaker_id,current_current_speaker_id)
		
	connection.DeleteConf(conf_id)
	connection.WaitAllConfEnd()
	connection.DelProfile(prof_id)
	
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestHD_Speaker_change(c)
c.Disconnect()

#------------------------------------------------------------------------------  
    
