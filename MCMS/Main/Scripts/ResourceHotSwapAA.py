#!/mcms/python/bin/python

#############################################################################
# Script which tests the Active Alarm in HotSwap Flow. 
# By Ohad.
#############################################################################
import os

#-- SKIP_ASSERTS

from ResourceUtilities import *
from AlertUtils import *

def CheckHotswapAA():
	connection = ResourceUtilities()
	connection.Connect()
	
	isMPmMode = connection.IsMPMMode()
	
	alertsUtils = AlertUtils()
	connection.WaitUntilStartupEnd()
	sleep(10)
	
	## Check that INSUFFICIENT_RESOURCES active alarm appears after startup
	bAlertCheck = alertsUtils.IsAlertExist("Resource", "Manager",  "INSUFFICIENT_RESOURCES", 1)
	if bAlertCheck == False:
	   sys.exit("error : INSUFFICIENT_RESOURCES active alarm didn't appear after startup")
	   
	## Check that the INSUFFICIENT_RESOURCES active alarm disappears after inserting another card
	
	boardId = 2
	## Insert card (depending on card mode)
	if(isMPmMode == 1) :
		connection.SimInsertCard(3,boardId)
	else:	
		connection.SimInsertCard(9,boardId)
	sleep(10)
	
	bAlertCheck = alertsUtils.IsAlertExist("Resource", "Manager",  "INSUFFICIENT_RESOURCES", 0)
	if bAlertCheck == False:
	   sys.exit("error : INSUFFICIENT_RESOURCES active alarm didn't disappear after inserting a card")
	
	## Check that NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER active alarm appears after removing all cards
	
	print "Removing Card " + str(boardId)
	connection.LoadXmlFile('Scripts/SimRemoveCardEvent.xml')
	connection.ModifyXml("REMOVE_CARD_EVENT","BOARD_ID",boardId)
	connection.Send()
	
	boardId = 1   
	
	print "Removing Card " + str(boardId)
	connection.LoadXmlFile('Scripts/SimRemoveCardEvent.xml')
	connection.ModifyXml("REMOVE_CARD_EVENT","BOARD_ID",boardId)
	connection.Send()   
	
	sleep(10)
	
	bAlertCheck = alertsUtils.IsAlertExist("Resource", "Manager",  "NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER", 1)
	if bAlertCheck == False:
	   sys.exit("error : NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER active alarm didn't appear after removing all cards")
	   
	bAlertCheck = alertsUtils.IsAlertExist("Resource", "Manager",  "INSUFFICIENT_RESOURCES", 1)
	if bAlertCheck == False:
	   sys.exit("error : INSUFFICIENT_RESOURCES active alarm didn't appear after removing all cards")   