#!/mcms/python/bin/python


from McmsConnection import *


class EthernetSettingsUtils ( McmsConnection ):
    
#------------------------------------------------------------------------------
	def GetCurrentEthernetSettings(self,expected_status="Status OK"):
		print "Get current EthernetSettings."
		self.SendXmlFile("Scripts/EthernetSettings_get_req.xml")
		ethernet_settings_list = self.xmlResponse.getElementsByTagName("ETHERNET_SETTINGS")
		print
		print "Slot \tPort \tPort Type            \tSpeed \t"
		for index in range(len(ethernet_settings_list)):
			slot_number = ethernet_settings_list[index].getElementsByTagName("SLOT_NUMBER")[0].firstChild.data
			port = ethernet_settings_list[index].getElementsByTagName("PORT")[0].firstChild.data
			ethernet_port_type = ethernet_settings_list[index].getElementsByTagName("ETHERNET_PORT_TYPE")[0].firstChild.data
			speed = ethernet_settings_list[index].getElementsByTagName("SPEED")[0].firstChild.data
			print slot_number + " \t" + port + " \t" + '%(ethernet_port_type)-20s' %{"ethernet_port_type":ethernet_port_type} + " \t" + speed
		print
		
		return(0)

#------------------------------------------------------------------------------           
	def UpdateCurrentEthernetSettings(self, slot_number, port, port_type, speed, expected_status="Status OK"):
		print "Update \t" + slot_number + " \t" + port + " \t" + port_type + " \t" + "to \t" + speed
		self.LoadXmlFile('Scripts/EthernetSettings_update_req.xml')
		self.ModifyXml("ETHERNET_SETTINGS","SLOT_NUMBER",slot_number)
		self.ModifyXml("ETHERNET_SETTINGS","PORT",port)
		self.ModifyXml("ETHERNET_SETTINGS","ETHERNET_PORT_TYPE",port_type)
		self.ModifyXml("ETHERNET_SETTINGS","SPEED",speed)
		self.Send(expected_status)
		
		return(0)

