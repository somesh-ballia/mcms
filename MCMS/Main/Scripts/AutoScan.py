#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#-LONG_SCRIPT_TYPE

from McmsConnection import *

#------------------------------------------------------------------------------
def ChangeConfLayoutTypeAndAutoScanCell(connection, confid, newConfLayout, autoScanCell,num_retries=30):
	interval_value = 10
	if(connection.IsProcessUnderValgrind("ConfParty")):
		interval_value = 30
	connection.LoadXmlFile('Scripts/XmlAutoScan/SetAutoScanInterval.xml')
	connection.ModifyXml("SET_AUTO_SCAN_INTERVAL","ID",confid)
	connection.ModifyXml("SET_AUTO_SCAN_INTERVAL","AUTO_SCAN_INTERVAL",interval_value)
	connection.Send()

	# start AutoScan in Cell (autoScanCell)
	print "Conference ID: "+ confid + " Changing Layout Type To: " + newConfLayout + ", with auto scan cell: " +  str(autoScanCell)
	connection.LoadXmlFile('Scripts/ConfVideoLayout/ChangConfLayout.xml')
	connection.ModifyXml("SET_VIDEO_LAYOUT","ID",confid)
	connection.ModifyXml("FORCE","LAYOUT",newConfLayout)
	force = connection.loadedXml.getElementsByTagName("CELL")
	for cell in force:
		cell_id = cell.getElementsByTagName("ID")[0].firstChild.data
		if (cell_id == str(autoScanCell)):
			cell.getElementsByTagName("FORCE_STATE")[0].firstChild.data = "auto_scan"
			break
	connection.Send()
	#
	connection.WaitCellIsMarkedAsAutoScan(confid,newConfLayout,str(autoScanCell),60)
	connection.WaitAllPartiesSeesPartyInCell(confid, 1, autoScanCell-1)
	print "  sleep 9 seconds (let the scan image to change) "
	sleep(9)
	if(connection.IsProcessUnderValgrind("ConfParty")):
		print "  ConfParty under valgrind sleep another 9 seconds "
		sleep(9)
	connection.WaitAllPartiesSeesPartyInCell(confid, 2, autoScanCell-1)
	
	return

#------------------------------------------------------------------------------
def FillAutoScanOrder(connection,conf_id, parties_order=[]):
	connection.LoadXmlFile('Scripts/XmlAutoScan/SetAutoScanOrder.xml')
	connection.ModifyXml("SET_AUTO_SCAN_ORDER","ID",confid)
	for x in range (len(parties_order)):
		connection.AddXML("AUTO_SCAN_ORDER","PARTY_ORDER")
		connection.AddXML("PARTY_ORDER","ID",str(parties_order[x]),x)
		connection.AddXML("PARTY_ORDER","ORDER",str(x),x)
	
	connection.Send()
#------------------------------------------------------------------------------
#Create CP conf with 6 parties
c = McmsConnection()
c.Connect()
num_of_parties = 6
c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',  #confFile
                         'Scripts/AddVideoParty1.xml',  #partyFile
                         num_of_parties,				#num_of_parties
                         60,							#num_retries
                         "false"						#deleteConf="TRUE"
                         )
#Get ConfId
c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
confid = c.GetTextUnder("CONF_SUMMARY","ID")
if confid == "":
	c.Disconnect()
	sys.exit("Can not monitor conf:" + status)

for x in range(num_of_parties):
	partyname = "Party"+str(num_of_parties-x)
	epsimguiname = "DIAL_OUT#1000" + str(num_of_parties-x)
	currPartyID = c.GetPartyId(confid,partyname)
	c.LoadXmlFile('Scripts/SpeakerChange/SimActiveSpeaker.xml')
	c.ModifyXml("ACTIVE_SPEAKER","PARTY_NAME",epsimguiname)
	c.Send()

	c.LoadXmlFile('Scripts/TransConf2.xml')
	c.ModifyXml('GET','ID',confid)
	print "Checking that audio active Speaker is party: "+ partyname
	for retry in range(30):
		c.Send()
		activePartyId = c.xmlResponse.getElementsByTagName('AUDIO_SOURCE_ID')[0].firstChild.data
		if (activePartyId == currPartyID):
			break

#Changing conf layout type to each layout
autoScanCell = 2
ChangeConfLayoutTypeAndAutoScanCell(c, confid,"2x1",autoScanCell)
FillAutoScanOrder(c,confid,[4,1,5])
c.WaitAllPartiesSeesPartyInCell(confid, 4, autoScanCell-1)
print "disconnecting party4 (the party that is in the auto scan cell)"
c.DisconnectParty(confid,4)
sleep(1)
c.WaitAllPartiesSeesPartyInCell(confid, 1, autoScanCell-1)
FillAutoScanOrder(c,confid)


#Delete Conf
c.DeleteConf(confid)
c.WaitAllConfEnd()

c.Disconnect()
"""
<PARTY_ORDER>
	<ID>4</ID>
	<ORDER>0</ORDER>
</PARTY_ORDER>
<PARTY_ORDER>
	<ID>1</ID>
	<ORDER>1</ORDER>
</PARTY_ORDER>
<PARTY_ORDER>
	<ID>2</ID>
	<ORDER>2</ORDER>
</PARTY_ORDER>
"""
