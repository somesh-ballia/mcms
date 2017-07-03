#!/mcms/python/bin/python

#############################################################################
#Script which tests allocation priorities in Amos, especially putting  participants of same conf together
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-LONG_SCRIPT_TYPE
#-- SKIP_ASSERTS

import os
from ResourceUtilities import *
from ISDNFunctions import *

c = ResourceUtilities()
c.Connect()

AC_UNIT_ID = 14
FFFF = 65535

print "============================================================"
print "Checking startup and the expected configuration - it should be auto mode with 288-video 160-audio" 
print "-----------------------------------------------------------"
c.CheckModes("auto","auto")
c.CheckAudioVideoAutoConfiguration(160, 288)
c.TestResourcesReportPortsType("video",288,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",288,"FREE")
c.TestResourcesReportPortsType("audio",160,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",160,"FREE")
print "==========================================================="
print "ALLOCATION PRIORITIES FOR VOIP"
print "-----------------------------------------------------------"
print "For voip, the allocation procedure is simple: only use load-balancing"
print "We will add one conference with 80 audio only participants, and see that they are equally set on all 4 cards"
print "And since it's auto mode, they will be grouped in a few units, starting with the AC unit" 
audio_only_confId = c.CreateConfWithAudioOnlyParties("Audio only", 80)
c.TestResourcesReportPortsType("audio",80,"OCCUPIED")
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,1000)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,1000)
c.CheckUtilisationOfUnit(3,AC_UNIT_ID,1000)
c.CheckUtilisationOfUnit(4,AC_UNIT_ID,1000)
smart_utilization_board1 = c.CountTotaloccupied(1,"smart")
smart_utilization_board2 = c.CountTotaloccupied(2,"smart")	
smart_utilization_board3 = c.CountTotaloccupied(3,"smart")
smart_utilization_board4 = c.CountTotaloccupied(4,"smart")	
if(smart_utilization_board1 != 1313):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 1: " + str(smart_utilization_board1))
if(smart_utilization_board2 != 1313):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 2: " + str(smart_utilization_board2))
if(smart_utilization_board3 != 1313):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 3: " + str(smart_utilization_board3))
if(smart_utilization_board4 != 1313):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 4: " + str(smart_utilization_board4))   
print "Indeed the VOIPs are equally put on four cards"   
c.DeleteConf(audio_only_confId)
c.WaitConfEnd(audio_only_confId)
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
print "==========================================================="
print "ALLOCATION PRIORITIES FOR IP VIDEO"
print "-----------------------------------------------------------"
print "Initiating different kinds of profiles, that we will use later"
print "Two profiles that should be CIF"
profId128_Motion = c.AddProfileWithRate("Profile128_motion",128,"motion")
profId128_Sharpness = c.AddProfileWithRate("Profile128_sharpness",128,"sharpness")
print "Two profiles that should be SD"
profId512_Motion = c.AddProfileWithRate("Profile512_motion",512,"motion")
profId768_Sharpness = c.AddProfileWithRate("Profile768_sharpness",768,"sharpness")
print "Two profiles that should be HD720"
profId1024_Motion = c.AddProfileWithRate("Profile1024_motion",1024,"motion")
profId1536_Sharpness = c.AddProfileWithRate("Profile1536_sharpness",1536,"sharpness")
print "Two profiles that should be HD1080"
profId1920_Motion = c.AddProfileWithRate("Profile1920_motion",1920,"motion")
profId4M_Sharpness = c.AddProfileWithRate("Profile4M_sharpness",4096,"sharpness")
print "-----------------------------------------------------------"
print "Check that video confs are put together on cards"
print "First we will create 4 conferences with 40, 60, 20, 80 participants each. They should each one be encapsulated in one card" 
confid1 = c.CreateConfFromProfileWithVideoParties("test1",profId128_Motion,40)
c.WaitAllOngoingConnected(confid1)
confid2 = c.CreateConfFromProfileWithVideoParties("test2",profId128_Motion,60)
c.WaitAllOngoingConnected(confid2)
confid3 = c.CreateConfFromProfileWithVideoParties("test3",profId128_Motion,20)
c.WaitAllOngoingConnected(confid3)
confid4 = c.CreateConfFromProfileWithVideoParties("test4",profId128_Motion,80)
c.WaitAllOngoingConnected(confid4)
c.TestResourcesReportPortsType("video",200,"OCCUPIED")
smart_utilization_board1 = c.CountTotaloccupied(1,"smart")
smart_utilization_board2 = c.CountTotaloccupied(2,"smart")	
smart_utilization_board3 = c.CountTotaloccupied(3,"smart")
smart_utilization_board4 = c.CountTotaloccupied(4,"smart")	
if(smart_utilization_board1 != 2563):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 1: " + str(smart_utilization_board1))
if(smart_utilization_board2 != 3813):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 2: " + str(smart_utilization_board2))
if(smart_utilization_board3 != 1313):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 3: " + str(smart_utilization_board3))
if(smart_utilization_board4 != 5063):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 4: " + str(smart_utilization_board4)) 
video_utilization_board1 = c.CountTotaloccupied(1,"video")
video_utilization_board2 = c.CountTotaloccupied(2,"video")	
video_utilization_board3 = c.CountTotaloccupied(3,"video")
video_utilization_board4 = c.CountTotaloccupied(4,"video")	
if(video_utilization_board1 != 10000):
   c.Disconnect()
   sys.exit("Unexpected video utilization on board 1: " + str(video_utilization_board1))
if(video_utilization_board2 != 15000):
   c.Disconnect()
   sys.exit("Unexpected video utilization on board 2: " + str(video_utilization_board2))
if(video_utilization_board3 != 5000):
   c.Disconnect()
   sys.exit("Unexpected video utilization on board 3: " + str(video_utilization_board3))
if(video_utilization_board4 != 20000):
   c.Disconnect()
   sys.exit("Unexpected video utilization on board 4: " + str(video_utilization_board4))
print "Indeed each of the cards has the expected utulisation"       
print "Now creating another conf, it will be on the less occupied board (3)" 
confid5 = c.CreateConfFromProfileWithVideoParties("test5",profId128_Motion,40)
c.WaitAllOngoingConnected(confid5)
c.TestResourcesReportPortsType("video",240,"OCCUPIED")
smart_utilization_board1 = c.CountTotaloccupied(1,"smart")
smart_utilization_board2 = c.CountTotaloccupied(2,"smart")	
smart_utilization_board3 = c.CountTotaloccupied(3,"smart")
smart_utilization_board4 = c.CountTotaloccupied(4,"smart")	
if(smart_utilization_board1 != 2563):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 1: " + str(smart_utilization_board1))
if(smart_utilization_board2 != 3813):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 2: " + str(smart_utilization_board2))
if(smart_utilization_board3 != 3813):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 3: " + str(smart_utilization_board3))
if(smart_utilization_board4 != 5063):
   c.Disconnect()
   sys.exit("Unexpected smart utilization on board 4: " + str(smart_utilization_board4)) 
video_utilization_board1 = c.CountTotaloccupied(1,"video")
video_utilization_board2 = c.CountTotaloccupied(2,"video")	
video_utilization_board3 = c.CountTotaloccupied(3,"video")
video_utilization_board4 = c.CountTotaloccupied(4,"video")	
if(video_utilization_board1 != 10000):
   c.Disconnect()
   sys.exit("Unexpected video utilization on board 1: " + str(video_utilization_board1))
if(video_utilization_board2 != 15000):
   c.Disconnect()
   sys.exit("Unexpected video utilization on board 2: " + str(video_utilization_board2))
if(video_utilization_board3 != 15000):
   c.Disconnect()
   sys.exit("Unexpected video utilization on board 3: " + str(video_utilization_board3))
if(video_utilization_board4 != 20000):
   c.Disconnect()
   sys.exit("Unexpected video utilization on board 4: " + str(video_utilization_board4))
print "Indeed each of the cards has the expected utulisation"  
c.DeleteConf(confid1)
c.DeleteConf(confid2)
c.DeleteConf(confid3)
c.DeleteConf(confid4)
c.DeleteConf(confid5)
c.WaitConfEnd(confid1)
c.WaitConfEnd(confid2)
c.WaitConfEnd(confid3)
c.WaitConfEnd(confid4)
c.WaitConfEnd(confid5)
c.TestResourcesReportPortsType("video",0,"OCCUPIED")  
print "-----------------------------------------------------------"
print "Checking the profiles and the resources that they take"
print "Initiating two conferences with the CIF profiles, each one with 1 participant, they will each take 1 CIF-resources"
confid128_motion = c.CreateConfFromProfileWithVideoParties("128_motion",profId128_Motion,1)
c.WaitAllOngoingConnected(confid128_motion)
c.TestResourcesReportPortsType("video",1,"OCCUPIED")
confid128_sharpness = c.CreateConfFromProfileWithVideoParties("128_sharpness",profId128_Sharpness,1)
c.WaitAllOngoingConnected(confid128_sharpness)
c.TestResourcesReportPortsType("video",2,"OCCUPIED")
print "Initiating two conferences with the SD profiles, each one with 1 participant, they will each take 2.666 CIF-resources (so in total and rounded 6 CIF resources)"
confid512_motion = c.CreateConfFromProfileWithVideoParties("512_motion",profId512_Motion,1)
c.WaitAllOngoingConnected(confid512_motion)
c.TestResourcesReportPortsType("video",5,"OCCUPIED")
confid768_sharpness = c.CreateConfFromProfileWithVideoParties("768_sharpness",profId768_Sharpness,1)
c.WaitAllOngoingConnected(confid768_sharpness)
c.TestResourcesReportPortsType("video",8,"OCCUPIED")
print "Initiating two conferences with the HD720 profiles, each one with 1 participant, they will each take 4 CIF-resources"
confid1024_motion = c.CreateConfFromProfileWithVideoParties("1024_motion",profId1024_Motion,1)
c.WaitAllOngoingConnected(confid1024_motion)
c.TestResourcesReportPortsType("video",12,"OCCUPIED")
confid1536_sharpness = c.CreateConfFromProfileWithVideoParties("1536_sharpness",profId1536_Sharpness,1)
c.WaitAllOngoingConnected(confid1536_sharpness)
c.TestResourcesReportPortsType("video",16,"OCCUPIED")
print "Initiating two conferences with the HD1080 profiles, each one with 1 participant, they will each take 8 CIF-resources"
confid1920_motion = c.CreateConfFromProfileWithVideoParties("1920_motion",profId1920_Motion,1)
c.WaitAllOngoingConnected(confid1920_motion)
c.TestResourcesReportPortsType("video",24,"OCCUPIED")
confid4M_sharpness = c.CreateConfFromProfileWithVideoParties("4M_sharpness",profId4M_Sharpness,1)
c.WaitAllOngoingConnected(confid4M_sharpness)
c.TestResourcesReportPortsType("video",32,"OCCUPIED")
print "-----------------------------------------------------------"
print "Disconnecting all participants in all conferences"
c.DisconnectParty(confid128_motion,0)
c.DisconnectParty(confid128_sharpness,0)
c.DisconnectParty(confid512_motion,0)
c.DisconnectParty(confid768_sharpness,0)
c.DisconnectParty(confid1024_motion,0)
c.DisconnectParty(confid1536_sharpness,0)
c.DisconnectParty(confid1920_motion,0)
c.DisconnectParty(confid4M_sharpness,0)
c.WaitAllOngoingDisConnected(confid128_motion)
c.WaitAllOngoingDisConnected(confid128_sharpness)
c.WaitAllOngoingDisConnected(confid512_motion)
c.WaitAllOngoingDisConnected(confid768_sharpness)
c.WaitAllOngoingDisConnected(confid1024_motion)
c.WaitAllOngoingDisConnected(confid1536_sharpness)
c.WaitAllOngoingDisConnected(confid1920_motion)
c.WaitAllOngoingDisConnected(confid4M_sharpness)
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
print "-----------------------------------------------------------"
video_unit_on_board1 = c.GetIdOfAVideoUnit(1)
video_unit_on_board2 = c.GetIdOfAVideoUnit(2)
print "On board 3 & 4, disable all "
c.SimDisableCard(3)
c.SimDisableCard(4)
print "On board 1, disable all except for 1 video unit"
c.SimDisableCard(1)
c.SimEnableUnit(1,video_unit_on_board1)
print "On board 2, disable all except for 1 audio unit (the audio controller unit)"
c.SimDisableCard(2)
c.SimEnableUnit(2,AC_UNIT_ID)
#wait for all keep-alives to come back... 
sleep(10)
c.WaitUnitEnabled(1,video_unit_on_board1)
c.WaitUnitEnabled(2,AC_UNIT_ID)
print "-----------------------------------------------------------"
print "Connecting one CIF party - it will be with audio on one card and video on the other"
print "##this checks step 4 in DecideAboutBestBoardsForIPVideoParty: video and ART separate, not optimal video (for CIF: not closing of fragmentation)"
c.ReconnectParty(confid128_motion,"true",0)
c.WaitAllOngoingConnected(confid128_motion)
c.TestResourcesReportPortsType("video",1,"OCCUPIED")
#On video unit the utilization should be 250, and on unit 14 (AC) of board 2, it should be 130 (62.5 for audio and 62.5 for AC)
c.CheckUtilisationOfUnit(1,video_unit_on_board1,250)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,125)
print "-----------------------------------------------------------"
print "Connecting two HD720 party. They will be downgraded to CIF. It will be with audio on one card and video on the other"
print "##this checks step 4 in DecideAboutBestBoardsForIPVideoParty: video and ART separate, not optimal video (for non CIF participants: downgrade)"
c.ReconnectParty(confid1024_motion,"true",0)
c.WaitAllOngoingConnected(confid1024_motion)
c.ReconnectParty(confid1536_sharpness,"true",0)
c.WaitAllOngoingConnected(confid1536_sharpness)
c.TestResourcesReportPortsType("video",3,"OCCUPIED")
c.CheckUtilisationOfUnit(1,video_unit_on_board1,750)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,250)
print "-----------------------------------------------------------"
print "Enabeling a video unit on board 2 too. Reconnect the other CIF participant."
print "It should put the video on the first card (and ART on second) in order to close the fragmentation"
print "##this checks step 2 in DecideAboutBestBoardsForIPVideoParty: video and ART separate, optimal video (for CIF: closing of fragmentation)" 
c.SimEnableUnit(2,video_unit_on_board2)
c.WaitUnitEnabled(2,video_unit_on_board2)
c.ReconnectParty(confid128_sharpness,"true",0)
c.WaitAllOngoingConnected(confid128_sharpness)
c.TestResourcesReportPortsType("video",4,"OCCUPIED")
c.CheckUtilisationOfUnit(1,video_unit_on_board1,1000)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,313)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,FFFF)
print "-----------------------------------------------------------"
print "Disconnecting all participants, and reconnecting 1 CIF participant. This time it should be on board 2"
print "##this checks step 3 in DecideAboutBestBoardsForIPVideoParty: video and ART together, not optimal video (for CIF: not closing of fragmentation)"
c.DisconnectParty(confid128_motion,0)
c.DisconnectParty(confid128_sharpness,0)
c.DisconnectParty(confid1024_motion,0)
c.DisconnectParty(confid1536_sharpness,0)
c.WaitAllOngoingDisConnected(confid128_motion)
c.WaitAllOngoingDisConnected(confid128_sharpness)
c.WaitAllOngoingDisConnected(confid1024_motion)
c.WaitAllOngoingDisConnected(confid1536_sharpness)
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.ReconnectParty(confid128_motion,"true",0)
c.WaitAllOngoingConnected(confid128_motion)
c.TestResourcesReportPortsType("video",1,"OCCUPIED")
c.CheckUtilisationOfUnit(1,video_unit_on_board1,FFFF)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,250)
print "-----------------------------------------------------------"
print "Connecting one SD party. It will be with audio on one card and video on the other"
print "##this checks step 2 in DecideAboutBestBoardsForIPVideoParty: video and ART separate, optimal video (for non CIF participants: no downgrade)" 
c.ReconnectParty(confid512_motion,"true",0)
c.WaitAllOngoingConnected(confid512_motion)
c.TestResourcesReportPortsType("video",4,"OCCUPIED")
c.CheckUtilisationOfUnit(1,video_unit_on_board1,1000)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,188)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,250)
print "-----------------------------------------------------------"
print "Disconnecting the SD party. And connecting one HD720 party (it will be downgraded to CIF). It will be with audio and video on same other"
print "##this checks step 3 in DecideAboutBestBoardsForIPVideoParty: video and ART together, not optimal video (for non CIF participants: downgrade)"
c.DisconnectParty(confid512_motion,0)
c.WaitAllOngoingDisConnected(confid512_motion) 
c.ReconnectParty(confid1024_motion,"true",0)
c.WaitAllOngoingConnected(confid1024_motion)
c.TestResourcesReportPortsType("video",2,"OCCUPIED")
c.CheckUtilisationOfUnit(1,video_unit_on_board1,FFFF)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,188)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,500)
print "-----------------------------------------------------------"
print "Cleanup and preparations for ISDN and PSTN tests"
print "Delete all conferences (part of them might have been auto-terminated)"
c.DeleteAllConf()
print "Remove RTM card of board 2"
print "So now we have on board 1: rtm + 1 video unit, and on board 2: 1 audio unit + 1 video unit"
c.SimRemoveCard(2,2)
c.SimRemoveCard(3,2)
c.SimRemoveCard(4,2)
confname = "Conf"
c.CreateConfFromProfile(confname,profId128_Motion)
confid = c.WaitConfCreated(confname)  
print "==========================================================="
print "ALLOCATION PRIORITIES FOR PSTN - DIAL-OUT"
print "-----------------------------------------------------------"
print "##this checks step 2 in DecideAboutBestBoardsForPSTNPartyDialOut: RTM and ART separate + load balancing between RTMs"
partyName = "PSTN_Dialout1"
c.AddPSTN_DialoutParty(confid,partyName,"1111")
c.WaitAllOngoingConnected(confid)
c.TestResourcesReportPortsType("audio",1,"OCCUPIED")
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,125)
c.CheckUnitActive(1,1,2)
c.CheckUnitAvailable(1,2,2)
c.CheckUnitAvailable(1,3,2)
c.CheckUnitAvailable(1,4,2)
partyName = "PSTN_Dialout2"
c.AddPSTN_DialoutParty(confid,partyName,"2222")
c.WaitAllOngoingConnected(confid)
c.TestResourcesReportPortsType("audio",2,"OCCUPIED")
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,188)
c.CheckUnitActive(1,1,2)
c.CheckUnitActive(1,2,2)
c.CheckUnitAvailable(1,3,2)
c.CheckUnitAvailable(1,4,2)
print "-----------------------------------------------------------"
print "Enable the AC unit on board 1 too. Now both ART + RTM should be on board 1"
print "##this checks step 1 in DecideAboutBestBoardsForPSTNPartyDialOut: RTM and ART together"
c.SimEnableUnit(1,AC_UNIT_ID)
c.WaitUnitEnabled(1,AC_UNIT_ID)
partyName = "PSTN_Dialout3"
c.AddPSTN_DialoutParty(confid,partyName,"3333")
c.WaitAllOngoingConnected(confid)
c.TestResourcesReportPortsType("audio",3,"OCCUPIED")
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,188)
c.CheckUnitActive(1,1,2)
c.CheckUnitActive(1,2,2)
c.CheckUnitActive(1,3,2)
c.CheckUnitAvailable(1,4,2)
partyName = "PSTN_Dialout4"
c.AddPSTN_DialoutParty(confid,partyName,"4444")
c.WaitAllOngoingConnected(confid)
c.TestResourcesReportPortsType("audio",4,"OCCUPIED")
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,188)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,188)
c.CheckUnitActive(1,1,2)
c.CheckUnitActive(1,2,2)
c.CheckUnitActive(1,3,2)
c.CheckUnitActive(1,4,2)
c.DeleteConf(confid)
c.WaitConfEnd(confid)
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
print "==========================================================="
print "ALLOCATION PRIORITIES FOR ISDN - DIAL-OUT"
print "-----------------------------------------------------------"
confname = "Conf"
c.CreateConfFromProfile(confname,profId128_Motion)
confid = c.WaitConfCreated(confname)  
partyName = "ISDN_DialOut1"
AddIsdnDialoutParty(c,confid,partyName,"1111")
print "Current situation is: on board 1: RTM, ART, Video. on board 2: ART, Video"
print "##this checks step 1 in DecideAboutBestBoardsForISDNVideoPartyDialOut: RTM, Video and ART together, optimal video"
c.WaitAllOngoingConnected(confid)
c.TestResourcesReportPortsType("video",1,"OCCUPIED")
c.CheckUnitActive(1,1,2)
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,FFFF)
c.CheckUtilisationOfUnit(1,video_unit_on_board1,250)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,FFFF)
c.DisconnectParty(confid,0)
c.WaitAllOngoingDisConnected(confid)
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
print "-----------------------------------------------------------"
print "Disable video unit on board 1 (where RTM and ART are)"
print "##this checks step 2 in DecideAboutBestBoardsForISDNVideoPartyDialOut: RTM and ART together, Video on other card"
c.SimDisableUnit(1,video_unit_on_board1)
c.WaitUnitDisabled(1,video_unit_on_board1)
c.ReconnectParty(confid,"true",0)
c.WaitAllOngoingConnected(confid)
c.TestResourcesReportPortsType("video",1,"OCCUPIED")
c.CheckUnitActive(1,1,2)
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,FFFF)
c.CheckUtilisationOfUnit(1,video_unit_on_board1,FFFF)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,250)
c.DisconnectParty(confid,0)
c.WaitAllOngoingDisConnected(confid)
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
print "-----------------------------------------------------------"
print "Re-enable video unit on board 1 (where RTM and ART are), but disable ART there"
print "##this checks step 3 in DecideAboutBestBoardsForISDNVideoPartyDialOut: RTM and Video together, ART on other card"
c.SimEnableUnit(1,video_unit_on_board1)
c.SimDisableUnit(1,AC_UNIT_ID)
c.WaitUnitEnabled(1,video_unit_on_board1)
c.WaitUnitDisabled(1,AC_UNIT_ID)
c.ReconnectParty(confid,"true",0)
c.WaitAllOngoingConnected(confid)
c.TestResourcesReportPortsType("video",1,"OCCUPIED")
c.CheckUnitActive(1,1,2)
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,FFFF)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(1,video_unit_on_board1,250)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,FFFF)
c.DisconnectParty(confid,0)
c.WaitAllOngoingDisConnected(confid)
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
print "-----------------------------------------------------------"
print "Re-enable video on board 2, but disable it on board 1"
print "##this checks step 3 in DecideAboutBestBoardsForISDNVideoPartyDialOut: ART and Video together, RTM on other card"
c.SimEnableUnit(2,video_unit_on_board1)
c.SimDisableUnit(1,video_unit_on_board1)
c.WaitUnitEnabled(2,video_unit_on_board1)
c.WaitUnitDisabled(1,video_unit_on_board1)
c.ReconnectParty(confid,"true",0)
c.WaitAllOngoingConnected(confid)
c.TestResourcesReportPortsType("video",1,"OCCUPIED")
c.CheckUnitActive(1,1,2)
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,FFFF)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(1,video_unit_on_board1,FFFF)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,250)
print "-----------------------------------------------------------"
print "Cleanup and preparations for ISDN and PSTN dial-in tests"
print "Delete the current conference"
c.DeleteConf(confid)
c.WaitConfEnd(confid)
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
print "create EQ with ISDN access"
targetEqName = "IsdnEQ"
eqPhone="3344"
c.CreatePSTN_EQ(targetEqName, eqPhone,profId128_Motion)
eqId, eqNID = c.WaitMRCreated(targetEqName)
MRNID = "6789"
MRName = "CIF Meeting room"
c.CreateMRWithNumericId(MRName, profId128_Motion, MRNID)
print "Current situation is: on board 1: RTM on board 2: ART, Video"
print "==========================================================="
print "ALLOCATION PRIORITIES FOR PSTN - DIAL-IN"
print "-----------------------------------------------------------"
print "##This will check the option for dial-in of RTM and ART separate" 
partyname = "PSTN_In"
c.SimulationAddPSTNParty(partyname,eqPhone)
sleep(3)
c.SimulationConnectPSTNParty(partyname)
eqConfId = c.WaitUntillEQorMRAwakes(targetEqName,1,10,True)
c.SimulationH323PartyDTMF(partyname, MRNID)
sleep(1)
MRConfId = c.WaitConfCreated(MRName,10)
c.WaitAllOngoingConnected(MRConfId)
print "Party indeed moved to MR"
c.TestResourcesReportPortsType("audio",1,"OCCUPIED")
c.CheckUnitActive(1,1,2)
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,FFFF)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,125)
c.DeletePSTNPartyFromSimulation(partyname)
c.WaitAllOngoingDisConnected(MRConfId)
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
print "Current situation is: on board 1: RTM on board 2: ART, Video"
print "-----------------------------------------------------------"
print "Enable the AC unit and the video back on board 1"
print "##This will check the option for dial-in of RTM and ART together" 
c.SimEnableUnit(1,AC_UNIT_ID)
c.WaitUnitEnabled(1,AC_UNIT_ID)
partyname = "PSTN_In2"
c.SimulationAddPSTNParty(partyname,eqPhone)
sleep(3)
c.SimulationConnectPSTNParty(partyname)
eqConfId = c.WaitUntillEQorMRAwakes(targetEqName,1,10,True)
c.SimulationH323PartyDTMF(partyname, MRNID)
sleep(1)
MRConfId = c.WaitConfCreated(MRName,10)
c.WaitAllOngoingConnected(MRConfId)
print "Party indeed moved to MR"
c.TestResourcesReportPortsType("audio",1,"OCCUPIED")
c.CheckUnitActive(1,1,2)
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,FFFF)
c.DeletePSTNPartyFromSimulation(partyname)
c.WaitAllOngoingDisConnected(MRConfId)
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
print "==========================================================="
print "ALLOCATION PRIORITIES FOR ISDN - DIAL-IN"
print "-----------------------------------------------------------"
print "The current situation is: on board 1: RTM, ART  on board 2: ART, Video"
print "##This will check the option for dial-in of RTM, ART together and Video on other card" 
partyname = "ISDN_In1"
SimulationAddIsdnParty(c,partyname,eqPhone)
sleep(3)
SimulationConnectIsdnParty(c,partyname)
eqConfId = c.WaitUntillEQorMRAwakes(targetEqName,1,10,True)
c.SimulationH323PartyDTMF(partyname, MRNID)
sleep(1)
MRConfId = c.WaitConfCreated(MRName,10)
c.WaitAllOngoingConnected(MRConfId)
print "Party indeed moved to MR"
c.TestResourcesReportPortsType("video",1,"OCCUPIED")
c.CheckUnitActive(1,1,2)
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,FFFF)
c.CheckUtilisationOfUnit(1,video_unit_on_board1,FFFF)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,250)
c.DeletePSTNPartyFromSimulation(partyname)
c.WaitAllOngoingDisConnected(MRConfId)
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
print "-----------------------------------------------------------"
print "Disable the ART on board 1. So that the current situation is: on board 1: RTM on board 2: ART, Video"
print "##This will check the option for dial-in of RTM and video together, ART separate" 
c.SimDisableUnit(1,AC_UNIT_ID)
c.WaitUnitDisabled(1,AC_UNIT_ID)
partyname = "ISDN_In2"
SimulationAddIsdnParty(c,partyname,eqPhone)
sleep(3)
SimulationConnectIsdnParty(c,partyname)
eqConfId = c.WaitUntillEQorMRAwakes(targetEqName,1,10,True)
c.SimulationH323PartyDTMF(partyname, MRNID)
sleep(1)
MRConfId = c.WaitConfCreated(MRName,10)
c.WaitAllOngoingConnected(MRConfId)
print "Party indeed moved to MR"
c.TestResourcesReportPortsType("video",1,"OCCUPIED")
c.CheckUnitActive(1,1,2)
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,FFFF)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(1,video_unit_on_board1,FFFF)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,250)
print "-----------------------------------------------------------"
print "Enable the video and the AC unit back on board 1. So that the current situation is: on board 1: RTM, ART, Video on board 2: ART, Video (that is partially used)"
print "Since the video on board 2 is not empty, the allocation will prefer to put the video there" 
c.SimEnableUnit(1,video_unit_on_board1)
c.SimEnableUnit(1,AC_UNIT_ID)
c.WaitUnitEnabled(1,video_unit_on_board1)
c.WaitUnitEnabled(1,AC_UNIT_ID)
partyname = "ISDN_In3"
SimulationAddIsdnParty(c,partyname,eqPhone)
sleep(3)
SimulationConnectIsdnParty(c,partyname)
eqConfId = c.WaitUntillEQorMRAwakes(targetEqName,1,10,True)
c.SimulationH323PartyDTMF(partyname, MRNID)
sleep(1)
MRConfId = c.WaitConfCreated(MRName,10)
c.WaitAllOngoingConnected(MRConfId)
print "Party indeed moved to MR"
c.TestResourcesReportPortsType("video",2,"OCCUPIED")
c.CheckUnitActive(1,1,2)
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(1,video_unit_on_board1,FFFF)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,500)
print "-----------------------------------------------------------"
print "Disable the AC unit on board 1. So that the current situation is: on board 1: RTM, Video on board 2: ART, Video (that is partially used)"
print "Since the video on board 2 is not empty, the allocation will prefer to put the video there" 
print "##This will check the option for dial-in of ART and video together, RTM separate" 
c.SimDisableUnit(1,AC_UNIT_ID)
c.WaitUnitDisabled(1,AC_UNIT_ID)
partyname = "ISDN_In4"
SimulationAddIsdnParty(c,partyname,eqPhone)
sleep(3)
SimulationConnectIsdnParty(c,partyname)
eqConfId = c.WaitUntillEQorMRAwakes(targetEqName,1,10,True)
c.SimulationH323PartyDTMF(partyname, MRNID)
sleep(1)
MRConfId = c.WaitConfCreated(MRName,10)
c.WaitAllOngoingConnected(MRConfId)
print "Party indeed moved to MR"
c.TestResourcesReportPortsType("video",3,"OCCUPIED")
c.CheckUnitActive(1,1,2)
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,188)
c.CheckUtilisationOfUnit(1,video_unit_on_board1,FFFF)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,750)
c.DeletePSTNPartyFromSimulation(partyname)
partyname = "ISDN_In2"
c.DeletePSTNPartyFromSimulation(partyname)
partyname = "ISDN_In3"
c.DeletePSTNPartyFromSimulation(partyname)
c.WaitAllOngoingDisConnected(MRConfId)
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
print "-----------------------------------------------------------"
print "Re-enable the AC unit on board 1. Delete all the participants, and reconnect one"
print "##This will check the option for ISDN dial-in of RTM, ART and video all together" 
c.SimEnableUnit(1,AC_UNIT_ID)
c.WaitUnitEnabled(1,AC_UNIT_ID)
partyname = "ISDN_In5"
SimulationAddIsdnParty(c,partyname,eqPhone)
sleep(3)
SimulationConnectIsdnParty(c,partyname)
eqConfId = c.WaitUntillEQorMRAwakes(targetEqName,1,10,True)
c.SimulationH323PartyDTMF(partyname, MRNID)
sleep(1)
MRConfId = c.WaitConfCreated(MRName,10)
c.WaitAllOngoingConnected(MRConfId)
print "Party indeed moved to MR"
c.TestResourcesReportPortsType("video",1,"OCCUPIED")
c.CheckUnitActive(1,1,2)
c.CheckUtilisationOfUnit(1,AC_UNIT_ID,125)
c.CheckUtilisationOfUnit(2,AC_UNIT_ID,FFFF)
c.CheckUtilisationOfUnit(1,video_unit_on_board1,250)
c.CheckUtilisationOfUnit(2,video_unit_on_board2,FFFF)
print "-----------------------------------------------------------"

c.Disconnect()

