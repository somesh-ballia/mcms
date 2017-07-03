#!/mcms/python/bin/python

#############################################################################
# Test Script For HD foreces
# In The Test:
#
# Add profile - HD at 1920 rate
# Start conference from that profile
# Connect 10 dial out participants
# Repeat 10 times:
#     set party force
#	  check that all party sees foreced party, and forced party sees speaker
#     change speaker 3 times and check that forced party sees new speaker
# Release force and check that all party sees speaker, and speaker sees previous speaker
# Delete conf and profile
#
# Date: 09/12/07
# By  : Romem
#############################################################################
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

from McmsConnection import *
from HDFunctions import *
import string  
#------------------------------------------------------------------------------

def TestHD_Force_PartyLevel(connection):

	num_of_dial_out_parties = 10
	num_retries = 10
	conf_name = "HD_conf_1920"
	num_of_force_change = 10
	num_of_speaker_change = 3
	
	## start conference
	prof_id = AddHdProfile(connection,"HD_profile_1920","1920", "Scripts/HD/XML/AddHdProfile.xml")
	connection.CreateConfFromProfile(conf_name, prof_id)
	conf_id = connection.WaitConfCreated(conf_name,num_retries)
	
	## connect participants
	AddDialOutHDParticipants(connection,conf_id,num_of_dial_out_parties)
	sleep(5)
	connection.WaitAllOngoingNotInIVR(conf_id)
	
	parties_list = GetPartiesInfo(connection,conf_id)
	
	current_speaker_id = int(parties_list[3][0])
	connection.ChangeDialOutVideoSpeaker(conf_id, current_speaker_id)
        sleep(1)
	next_speaker_id = int(parties_list[0][0])
	print "next_speaker_id " + str(next_speaker_id)
	connection.ChangeDialOutVideoSpeaker(conf_id, next_speaker_id)
	sleep(1)
	CheckConfLayout(connection,conf_id,next_speaker_id)
        speaker_index = 0
	next_speaker_index = 1
        confLayoutType = "1x1"
	force_party_index = 4
        forced_party_index = 1
	force_party_id = int(parties_list[force_party_index][0])
        forced_party_id = int(parties_list[force_party_index - 1][0])
        print "\n Step 1: Conference Level Force" +  "\n"
        print "\n"+"Forcing in Conference level to party: " + parties_list[force_party_index][1] + ", monitoring id: " 
        print "\n"+str(force_party_id)
	SetConfLevelForce(connection, conf_id, "1x1", force_party_id, 0)
	sleep(1)
	CheckConfLayout(connection,conf_id,force_party_id)
	CheckConfSourceLayout(connection,conf_id,force_party_id,next_speaker_id)
        sleep(1)
        print "\n"+"Step 2:Party level force in case there is conference level force"+"\n"
        force_party_index_2 = 3
        forced_party_index_2 =7
	force_party_id_2 = int(parties_list[force_party_index_2][0])
        forced_party_id_2 = int(parties_list[force_party_index_2 - 1][0])
        print "\n"+"Forcing in party level to party: " + parties_list[forced_party_index_2][1] + ", monitoring id: " 
        print "\n"+str(forced_party_id_2)
        print "\n"+"change force to party: " + parties_list[force_party_index_2][1] + ", monitoring id: " + str(force_party_id_2)
        SetPartyLevelForce(connection, conf_id, forced_party_id_2, confLayoutType, force_party_id_2, 0)        
        sleep(1)
        CheckPartyLayout(connection,conf_id,forced_party_id_2,force_party_id_2)
        RemovePartyLevelForce(connection, conf_id, forced_party_id_2, confLayoutType, 0)
	sleep(1)
        RemoveConfLevelForce(connection, conf_id, "1x1", 0)
        sleep(1)
	print "\n"+"Step 3:Party level force in case no force in conference level"+"\n"
        SetPartyLevelForce(connection, conf_id, forced_party_id, confLayoutType, force_party_id, 0)
        print "\n"+"Forcing in party level to party: " + parties_list[forced_party_index][1] + ", monitoring id: " + str(forced_party_id)
        print "\n"+"change force to party: " + parties_list[force_party_index][1] + ", monitoring id: " + str(force_party_id)
        sleep(1)
        CheckPartyLayout(connection,conf_id,forced_party_id,force_party_id)
        RemovePartyLevelForce(connection, conf_id, forced_party_id, confLayoutType, 0)
	sleep(1)
        print "\n"+"Step 4:Back to Audio Activation"+"\n"
	speaker_id = int(parties_list[speaker_index][0])	
	next_speaker_id = int(parties_list[next_speaker_index][0])
        connection.ChangeDialOutVideoSpeaker(conf_id, next_speaker_id)
        sleep(1)
	CheckConfVswLayout(connection,conf_id,next_speaker_id,speaker_id)	
	
        connection.DeleteConf(conf_id)
        connection.WaitAllConfEnd()
	connection.DelProfile(prof_id)
	
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestHD_Force_PartyLevel(c)
c.Disconnect()

#------------------------------------------------------------------------------  
    
