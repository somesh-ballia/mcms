#!/mcms/python/bin/python

#############################################################################
#Script which tests reset port configuration feature
#it will also run under valgrind
#############################################################################

import os
from ResourceUtilities import *

def TestResetPort(video_one_card, video_two_cards, one_card_file_name, two_card_file_name, card_type_for_insert_card, ):
	
	c = ResourceUtilities()
	c.Connect()
	print ""
	print "#################################################"
	print "Check CARDS == SLIDER == LICENSE, in startup"
	print "#################################################"
	print "Resetting port configuration and checking result"
	c.ResetPortConfiguration(0,video_two_cards)
	print ""
	print "#################################################"
	print "Check CARD < SLIDER, while running"
	print "#################################################"
	print "Removing card, and checking that there are insufficient resources"
	c.SimRemoveCard(1)
	sleep(10)
	if c.IsThereAnActiveAlarmOfInsufficientResources() == 0 :
	   	c.Disconnect()                
	   	sys.exit("There's NO active alarm of insufficient resources!!!")  
	c.CheckAudioVideoAutoConfiguration(0,video_two_cards,"The current slider settings require more system resources than are currently available")
	print "-----------------------------------------------------------"
	print "Resetting port configuration and checking result"
	c.ResetPortConfiguration(0,video_one_card)
	if c.IsThereAnActiveAlarmOfInsufficientResources() == 1 :
	   	c.Disconnect()                
	   	sys.exit("There's still an active alarm of insufficient resources!!!")  
	print ""
	print "#################################################"
	print "Check CARDS == SLIDER < LICENSE, in startup"
	print "#################################################"
	print "Resetting MCU, load it with one card and see that it loaded as expected"
	c.Disconnect()
	os.environ["CLEAN_CFG"]="NO"
	os.environ["RESOURCE_SETTING_FILE"]=""
	os.environ["MPL_SIM_FILE"]=one_card_file_name
	os.system("Scripts/Destroy.sh")
	os.system("Scripts/Startup.sh")
	c.Connect()
	print "-----------------------------------------------------------"
	print "Check configuration as expected"
	c.CheckAudioVideoAutoConfiguration(0,video_one_card)
	if c.IsThereAnActiveAlarmOfInsufficientResources() == 1 :
	   	c.Disconnect()                
	   	sys.exit("There's still an active alarm of insufficient resources!!!")  
	print ""
	print "#################################################"
	print "Check CARDS > SLIDER, while running"
	print "#################################################"
	print "Insert a card, and see that now the slider says that there are more resources available"
	c.SimInsertCard(card_type_for_insert_card,2)
	sleep(10)
	if c.IsThereAnActiveAlarmOfInsufficientResources() == 1 :
	   	c.Disconnect()                
	   	sys.exit("There IS an active alarm of insufficient resources!!!")  
	c.CheckAudioVideoAutoConfiguration(0,video_one_card,"There are unallocated resources in the system. Please adjust the following sliders to optimize resource capacity")
	print "-----------------------------------------------------------"
	print "Resetting port configuration and checking result"
	c.ResetPortConfiguration(0,video_two_cards)
	if c.IsThereAnActiveAlarmOfInsufficientResources() == 1 :
	   	c.Disconnect()                
	   	sys.exit("There IS an active alarm of insufficient resources!!!")  
	print "#################################################"
	print "Check CARD < SLIDER, in startup"
	print "#################################################"
	print "Resetting MCU, load it with one card and see that it loaded as expected"
	c.Disconnect()
	os.environ["CLEAN_CFG"]="NO"
	os.environ["RESOURCE_SETTING_FILE"]=""
	os.environ["MPL_SIM_FILE"]=one_card_file_name
	os.system("Scripts/Destroy.sh")
	os.system("Scripts/Startup.sh")
	c.Connect()
	c.WaitUntilStartupEnd()
	print "-----------------------------------------------------------"
	print "Check configuration as expected"
	c.CheckAudioVideoAutoConfiguration(0,video_two_cards,"The current slider settings require more system resources than are currently available")
	if c.IsThereAnActiveAlarmOfInsufficientResources() == 0 :
	   	c.Disconnect()                
	   	sys.exit("There's NO active alarm of insufficient resources!!!")  
	print "-----------------------------------------------------------"
	print "Resetting port configuration and checking result"
	c.ResetPortConfiguration(0,video_one_card)
	if c.IsThereAnActiveAlarmOfInsufficientResources() == 1 :
	   	c.Disconnect()                
	   	sys.exit("There's still an active alarm of insufficient resources!!!")  
	print "#################################################"
	print "Check CARD > SLIDER, in startup"
	print "#################################################"
	print "Resetting MCU, load it with TWO cards and see that it loaded as expected"
	c.Disconnect()
	os.environ["CLEAN_CFG"]="NO"
	os.environ["RESOURCE_SETTING_FILE"]=""
	os.environ["MPL_SIM_FILE"]=two_card_file_name
	os.system("Scripts/Destroy.sh")
	os.system("Scripts/Startup.sh")
	c.Connect()
	print "-----------------------------------------------------------"
	print "Check configuration as expected"
	c.CheckAudioVideoAutoConfiguration(0,video_one_card,"There are unallocated resources in the system. Please adjust the following sliders to optimize resource capacity")
	if c.IsThereAnActiveAlarmOfInsufficientResources() == 1 :
	   	c.Disconnect()                
	   	sys.exit("There IS an active alarm of insufficient resources!!!")  
	print "-----------------------------------------------------------"
	print "Resetting port configuration and checking result"
	c.ResetPortConfiguration(0,video_two_cards)
	if c.IsThereAnActiveAlarmOfInsufficientResources() == 1 :
	   	c.Disconnect()                
	   	sys.exit("There IS an active alarm of insufficient resources!!!")  
	print "----------------END TEST-------------------------------------"
	
	c.Disconnect()

