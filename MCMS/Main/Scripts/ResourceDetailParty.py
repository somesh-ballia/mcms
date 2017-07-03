#!/mcms/python/bin/python
#############################################################################
# Script which tests the resource detail functionality according to 
# predefined number of video parties.
# By Sergey and Michael.
#############################################################################
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"

from ResourceUtilities import *

c = McmsConnection()
c.Connect()
num_parties =4
c.SimpleXmlConfPartyTest('Scripts/ResourceDetailParty/AddCpConf.xml',
                         'Scripts/ResourceDetailParty/AddVideoParty1.xml',
                         num_parties, 60, "FALSE")
c.SendXmlFile("Scripts/ResourceDetailParty/CardDetailReq.xml")
unit_summary = c.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
unit_len =len(unit_summary)
needed_v_cap = int(num_parties) *500 
needed_a_cap = int(num_parties) *125      
print "The number of configured units is  " + str(unit_len)
print " The needed video capacity for " +str(num_parties) + " parties is : " + str(needed_v_cap)
print " The needed art   capacity for " +str(num_parties) + " parties is : " + str(needed_a_cap)
print




v_sum = 0
a_sum = 0
for i in range(unit_len):
        if unit_summary[i].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == "video":
           v_util = unit_summary[i].getElementsByTagName("UTILIZATION")[0].firstChild.data
           if (int(v_util) < 1001) :
               v_sum = int(v_sum) + int(v_util)
        if unit_summary[i].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == "smart":
           a_util = unit_summary[i].getElementsByTagName("UTILIZATION")[0].firstChild.data
           if (int(a_util) < 1001 ) :
               a_sum = int(a_sum) + int(a_util)
        
print " The sum of video capacity occupied on card 1 is = " + str(v_sum)
print " The sum of audio capacity occupied on card 1 is = " + str(a_sum)

# the total sum allocated on card 1 should be half of the needed
# because of optimisation, we are load balancing between the two cards
num_of_audio_on_board_1 = a_sum / 125
if (num_of_audio_on_board_1 != num_parties / 2 ) & (num_of_audio_on_board_1 != num_parties):
    sys.exit("error" + " - art allocation error" )
    
if (v_sum / 500 != num_of_audio_on_board_1):
    sys.exit("error" + " - video allocation error" )

print
print "  *** THE ALLOCATIONS ON MFA REFLECT THE NUMBER OF ONGOING PARTIES EXACTLY *** "
print 

c.DeleteAllConf()
c.WaitAllConfEnd()

# Checking that resource detail is empty after conference deletion

c.SendXmlFile("Scripts/ResourceDetailParty/CardDetailReq.xml")
unit_summary = c.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
v_sum = 0
a_sum = 0
for i in range(unit_len):
        if unit_summary[i].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == "video":
           v_util = unit_summary[i].getElementsByTagName("UTILIZATION")[0].firstChild.data
           if (int(v_util) < 1001) :
               v_sum = int(v_sum) + int(v_util)
        if unit_summary[i].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == "smart":
           a_util = unit_summary[i].getElementsByTagName("UTILIZATION")[0].firstChild.data
           if (int(a_util) < 1001 ) :
               a_sum = int(a_sum) + int(a_util)

if ( int(v_sum) != 0 | int(a_sum) != 0 ):
   sys.exit("error" + " - resoutce detail isn't empty after conference deletion!" )
   
c.Disconnect()


