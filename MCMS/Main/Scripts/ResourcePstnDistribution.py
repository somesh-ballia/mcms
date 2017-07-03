#!/mcms/python/bin/python
#############################################################################

#############################################################################
# Script which tests the distribution of PSTN parties over the cards. 
# By Ohad.
#############################################################################

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_80_YES_PstnYES_v100.0.cfs"

import os
from ResourceUtilities import *

numRetries = 20

connection = ResourceUtilities()
connection.Connect()

# Verify that there are 2 MFA + 2 RTM (+ Switch)
connection.SendXmlFile('Scripts/CardListMonitor.xml')
cards_list = connection.xmlResponse.getElementsByTagName("CARD_SUMMARY_DESCRIPTOR")
num_cards_list = len(cards_list)

if ( num_cards_list < 5):
	sys.exit("Not enough cards for this test")

num_of_parties = 12
confName = 'PSTN_Conf'
connection.CreateConf(confName)
confid = connection.WaitConfCreated(confName) 

connection.LoadXmlFile("Scripts/PSTN_Party.xml")
for x in range(num_of_parties):
    partyname = "Pstn"+str(x+1)
    phone="3333"+str(x+1)
    connection.AddPSTN_DialoutParty(confid,partyname,phone)

connection.WaitAllPartiesWereAdded(confid, num_of_parties, numRetries*num_of_parties)
connection.WaitAllOngoingConnected(confid)
    
## Check capacity
capacityMFA1 = GetCapacity( 1, 1, 0xFFFF)
capacityRTM1 = GetCapacity( 1, 2, 0xFFFF)
capacityMFA2 = GetCapacity( 2, 1, 0xFFFF)
capacityRTM2 = GetCapacity( 2, 2, 0xFFFF)

print "MFA capacity on Board 1 is : " + str(capacityMFA1)
print "RTM capacity on Board 1 is : " + str(capacityRTM1)
print "MFA capacity on Board 2 is : " + str(capacityMFA2)
print "RTM capacity on Board 2 is : " + str(capacityRTM2)

if ( capacityMFA1 == 0 ) or ( capacityRTM1 == 0) or ( capacityMFA2 == 0 ) or ( capacityRTM2 == 0 ):
	connection.DeleteConf(confid)   
	connection.WaitAllConfEnd()    
	sys.exit("Wrong distribution of PSTN resources on cards")

print "Successful distribution of PSTN resources on cards"

connection.DeleteConf(confid)   
connection.WaitAllConfEnd()

connection.Disconnect()