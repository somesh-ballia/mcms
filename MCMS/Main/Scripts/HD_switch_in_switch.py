#!/mcms/python/bin/python

#############################################################################
# Test Script For HD switch in switch
# In The Test:
#
# Add profile - HD at 1920 rate
# Start conference from that profile
# Connect 10 dial out participants
# 1)
# Repeat 10 times:
#     change speaker (no wait)
# check that conf sees last speaker and new speaker sees previous speaker
# 2)
# Repeat 10 times:
#     change speaker (no wait)
# Force new party (no wait)
# check that all party sees foreced party, and forced party sees speaker
# Release force and check that all party sees speaker, and speaker sees previous speaker
# Delete conf and profile
#
# Date: 07/12/06
# By  : Ron
#############################################################################
from McmsConnection import *
from HDFunctions import *
import string 
#------------------------------------------------------------------------------

def TestHD_switch_in_switch(connection):

	num_of_dial_out_parties = 10
	num_retries = 10
	conf_name = "HD_conf_1920"
	num_of_speaker_change = 10
	
	## start conference
	prof_id = AddHdProfile(connection,"HD_profile_1920","1920", "Scripts/HD/XML/AddHdProfile.xml")
	connection.CreateConfFromProfile(conf_name, prof_id)
	conf_id = connection.WaitConfCreated(conf_name,num_retries)
	
	## connect participants
	AddDialOutHDParticipants(connection,conf_id,num_of_dial_out_parties)
	sleep(5)
	connection.WaitAllOngoingNotInIVR(conf_id)
	parties_list = GetPartiesInfo(connection,conf_id)
	
	next_speaker_id = int(parties_list[0][0])
	print "next_speaker_id " + str(next_speaker_id)
	connection.ChangeDialOutVideoSpeaker(conf_id, next_speaker_id)
	
	speaker_index = 0
	next_speaker_index = 1
	print "\n" + "start " + str(num_of_speaker_change) + " speaker changes:"
	for change_layout_index in range (num_of_speaker_change):
		speaker_index = next_speaker_index
		next_speaker_index += 1
		if(next_speaker_index >= len(parties_list)):
			next_speaker_index = 0
		current_current_speaker_id = next_speaker_id
		next_speaker_id = int(parties_list[next_speaker_index][0])
		print "\n"+"change speaker to party: " + parties_list[next_speaker_index][1] + ", monitoring id: " + str(next_speaker_id)
		connection.ChangeDialOutVideoSpeaker(conf_id, next_speaker_id)
		sleep(1)

	speaker_id = int(parties_list[speaker_index][0])	
	next_speaker_id = int(parties_list[next_speaker_index][0])
	sleep(2)
	print "\n" + "check all parties sees speaker"
	CheckConfLayout(connection,conf_id,next_speaker_id)
	print "\n" + "check speaker sees previous speaker"
	CheckConfSourceLayout(connection,conf_id,next_speaker_id,current_current_speaker_id)
	
	for change_layout_index in range (num_of_speaker_change):
		speaker_index = next_speaker_index
		next_speaker_index += 1
		if(next_speaker_index >= len(parties_list)):
			next_speaker_index = 0
		current_current_speaker_id = next_speaker_id
		next_speaker_id = int(parties_list[next_speaker_index][0])
		print "\n"+"change speaker to party: " + parties_list[next_speaker_index][1] + ", monitoring id: " + str(next_speaker_id)
		connection.ChangeDialOutVideoSpeaker(conf_id, next_speaker_id)
		
	force_party_index = 4
	force_party_id = int(parties_list[force_party_index][0])
	print "\n" + "set conf level force to party: " + parties_list[force_party_index][1] + ", monitoring id: " + str(force_party_id)
	sleep(2)
	SetConfLevelForce(connection, conf_id, "1x1", force_party_id, 0)
	speaker_id = int(parties_list[speaker_index][0])	
	next_speaker_id = int(parties_list[next_speaker_index][0])
	sleep(2)
	print "\n" + "check all parties sees conf level force"
	CheckConfLayout(connection,conf_id,force_party_id)
	print "\n" + "forced party sees last speaker"
	CheckConfSourceLayout(connection,conf_id,force_party_id,next_speaker_id)
	print "\n" + "removing conf force"
	RemoveConfLevelForce(connection, conf_id, "1x1", 0)
	sleep(2)
	print "\n" + "check all parties sees speaker"
	CheckConfLayout(connection,conf_id,next_speaker_id)
	print "\n" + "check speaker sees previous speaker"
	CheckConfSourceLayout(connection,conf_id,next_speaker_id,speaker_id)
		
	connection.DelProfile(prof_id)
	connection.DeleteConf(conf_id)
	connection.WaitAllConfEnd()
	
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestHD_switch_in_switch(c)
c.Disconnect()

#------------------------------------------------------------------------------  
    
