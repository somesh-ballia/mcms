#!/mcms/python/bin/python
#############################################################################
# Script which tests the unit resource detail functionality according to 
# predefined video party.
# By Sergey.
# Note (Zoe): the party usually will be on board number 2 (this happens when board 1 sends startup complete indication before board 2)
# In some cases (usually under valgrind) board 2 will send startup complete indication before board 1
# In this case, the party will be put on board 1
# Therefor the test checks if the party is on board 2, and if it's not it searches for it on board 1
#############################################################################
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"

from ResourceUtilities import *

#------------------------------------------------------------------------------
def CountResources(connection, board_id,conf_id,party_id,needed_v_cap,needed_a_cap):
	v_sum = 0
	a_sum = 0
	conf_id_unit = -1
	party_id_unit = -1

	connection.LoadXmlFile('Scripts/ResourceDetailParty/CardDetailReq.xml')
	connection.ModifyXml("GET","BOARD_NUMBER",board_id)
	connection.Send()
	
	unit_summary = connection.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
	unit_len =len(unit_summary)

	print "The number of configured units is  " + str(unit_len)	+ " on board " + str(board_id)
	
	for i in range(unit_len):
	    if unit_summary[i].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == "video":
	       v_util = unit_summary[i].getElementsByTagName("UTILIZATION")[0].firstChild.data
	       if (int(v_util) < 1001) :
	           c.LoadXmlFile('Scripts/ResourceDetailParty/UnitDetReq.xml')
	           c.ModifyXml("GET_UNIT","UNIT_NUMBER",int(i) +1 )
	           c.ModifyXml("GET_UNIT","BOARD_NUMBER",board_id )
	           c.Send()
	           active_ports = c.xmlResponse.getElementsByTagName("ACTIVE_PORT_DETAILS")
	           arr_len = len(active_ports)
	           if( arr_len != 2 ):
	              print "arr_len_vid = " + str(arr_len)
	              sys.exit("error" + " - video ports number doesn't equal to 2" )
	           v_sum = int(v_sum) + int(v_util)
	    if unit_summary[i].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == "smart":
	       a_util = unit_summary[i].getElementsByTagName("UTILIZATION")[0].firstChild.data
	       if (int(a_util) < 1001 ) :
	           c.LoadXmlFile('Scripts/ResourceDetailParty/UnitDetReq.xml')
	           c.ModifyXml("GET_UNIT","UNIT_NUMBER",int(i) +1 )
	           c.ModifyXml("GET_UNIT","BOARD_NUMBER",board_id )
	           c.Send()
	           active_ports = c.xmlResponse.getElementsByTagName("ACTIVE_PORT_DETAILS")
	           arr_len = len(active_ports)
	           if( arr_len != 1 ):
	               print "arr_len = " + str(arr_len)
	               sys.exit("error" + " - more then one art port" )
	           conf_id_unit  = active_ports[0].getElementsByTagName("CONF_ID")[0].firstChild.data
	           party_id_unit = active_ports[0].getElementsByTagName("PARTY_ID")[0].firstChild.data             
	           a_sum = int(a_sum) + int(a_util)
           
	print " The sum of video capacity occupied is = " + str(v_sum)
	print " The sum of audio capacity occupied is = " + str(a_sum)
  
	print " conf Id from unit details = " + str(conf_id_unit)
	print " party Id from unit details = " + str(party_id_unit)
	
	if ( conf_id_unit != conf_id  ) | ( party_id_unit != party_id ):
		return 0
	
	if (a_sum / 125 != num_parties) | (a_sum % num_parties != 0):
		sys.exit("error" + " - art allocation error" )
    
	if (v_sum / 500 != num_parties) | (v_sum % num_parties != 0):
		sys.exit("error" + " - video allocation error" )
	
	return 1	
 #------------------------------------------------------------------------------           




c = McmsConnection()
c.Connect()

print "Adding Conf..."
num_of_undef_parties = 3
c.LoadXmlFile('Scripts/ResourceDetailParty/AddCpConf.xml')
c.ModifyXml("MEET_ME_PER_CONF","ON","true")
c.ModifyXml("MEET_ME_PER_CONF","MIN_NUM_OF_PARTIES",int(num_of_undef_parties))
c.Send()
conf_id = c.WaitConfCreated("VideoConf")
#conf_id = c.GetTextUnder("RESERVATION","ID")

print "conf_id = " + str(conf_id)

print "Adding First Party..."

c.LoadXmlFile('Scripts/ResourceDetailParty/AddVideoParty1.xml')
c.ModifyXml("ADD_PARTY","ID",conf_id)
c.Send()
sleep(1)
c.LoadXmlFile('Scripts/ResourceDetailParty/TransConf2.xml')
c.ModifyXml("GET","ID",conf_id)
c.Send()
c.WaitAllPartiesWereAdded(conf_id,1,20)
c.WaitAllOngoingConnected(conf_id)
party_id = c.GetTextUnder("PARTY","ID")

print "Party Id = " + party_id

num_parties =1


needed_v_cap = int(num_parties) *500 
needed_a_cap = int(num_parties) *125      

print " The needed video capacity for " +str(num_parties) + " parties is : " + str(needed_v_cap)
print " The needed art   capacity for " +str(num_parties) + " parties is : " + str(needed_a_cap)
print



if(CountResources(c, 2, conf_id, party_id, needed_v_cap, needed_a_cap) == 0):
	if(CountResources(c, 1, conf_id, party_id, needed_v_cap, needed_a_cap) == 0):
		sys.exit("error" + " - card unit detail error" )



print
print "  *** THE MFA UNIT DETAIL REFLECTS THE ONGOING PARTIES EXACTLY *** "
print 

c.DisconnectParty(conf_id,party_id)
c.WaitPartyDisConnected(conf_id,party_id,80)
c.DeleteConf(conf_id)
c.WaitAllConfEnd(30)

sleep (1)
# Checking that resource detail is empty after conference deletion

c.SendXmlFile("Scripts/ResourceDetailParty/CardDetailReq.xml")
unit_summary = c.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
unit_len =len(unit_summary)

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
   sys.exit("error" + " - resource detail isn't empty after conference deletion!" )
   
c.Disconnect()

