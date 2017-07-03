#!/mcms/python/bin/python


from McmsConnection import *


class TimeSettingUtils ( McmsConnection ):
    
#------------------------------------------------------------------------------
	def GetCurrentTimeSetting(self,expected_status="Status OK"):
		print "Get current TimeSetting."
		self.SendXmlFile("Scripts/TimeSetting_get_req.xml")
		mcu_base_time = self.xmlResponse.getElementsByTagName("MCU_BASE_TIME")[0].firstChild.data
		gmt_offset_sign = self.xmlResponse.getElementsByTagName("GMT_OFFSET_SIGN")[0].firstChild.data
		gmt_offset = self.xmlResponse.getElementsByTagName("GMT_OFFSET")[0].firstChild.data
		is_ntp = self.xmlResponse.getElementsByTagName("IS_NTP")[0].firstChild.data
		ntp_servers_parameters = self.xmlResponse.getElementsByTagName("NTP_SERVERS_PARAMETERS")
		print
		print "GMT Time \t\tGMT Offset Sign \tGMT Offset \tUse NTP Server"
		print mcu_base_time + " \t" + gmt_offset_sign + " \t\t\t" + gmt_offset + " \t\t" + is_ntp
		print
		print "NTP_SERVERS_PARAMETERS"
		for index in range(len(ntp_servers_parameters)):
			ntp_ip_address = ntp_servers_parameters[index].getElementsByTagName("NTP_IP_ADDRESS")[0].firstChild.data
			ntp_server_status = ntp_servers_parameters[index].getElementsByTagName("NTP_SERVER_STATUS")[0].firstChild.data
			print '%(ntp_ip_address)-20s' %{"ntp_ip_address":ntp_ip_address} + " \t" + ntp_server_status
		print
		
		return(0)

#------------------------------------------------------------------------------           
	def SetCurrentTimeSetting(self, mcu_base_time, gmt_offset_sign, gmt_offset, is_ntp, 
			ntp_ip_address_1, ntp_server_status_1,
			ntp_ip_address_2, ntp_server_status_2,
			ntp_ip_address_3, ntp_server_status_3,
			expected_status="Status OK"):
		print "Set TimeSetting to"
		print
		print "GMT Time \t\tGMT Offset Sign \tGMT Offset \tUse NTP Server"
		print mcu_base_time + " \t" + gmt_offset_sign + " \t\t\t" + gmt_offset + " \t\t" + is_ntp
		print
		print "NTP_SERVERS_PARAMETERS"
		print '%(ntp_ip_address)-20s' %{"ntp_ip_address":ntp_ip_address_1} + " \t" + ntp_server_status_1
		print '%(ntp_ip_address)-20s' %{"ntp_ip_address":ntp_ip_address_2} + " \t" + ntp_server_status_2
		print '%(ntp_ip_address)-20s' %{"ntp_ip_address":ntp_ip_address_3} + " \t" + ntp_server_status_3
		
		self.LoadXmlFile('Scripts/TimeSetting_set_req.xml')
		self.loadedXml.getElementsByTagName("MCU_BASE_TIME")[0].firstChild.data = mcu_base_time
		self.loadedXml.getElementsByTagName("GMT_OFFSET_SIGN")[0].firstChild.data = gmt_offset_sign
		self.loadedXml.getElementsByTagName("GMT_OFFSET")[0].firstChild.data = gmt_offset
		self.loadedXml.getElementsByTagName("IS_NTP")[0].firstChild.data = is_ntp
		self.loadedXml.getElementsByTagName("NTP_SERVERS_PARAMETERS")[0].getElementsByTagName("NTP_IP_ADDRESS")[0].firstChild.data = ntp_ip_address_1
		self.loadedXml.getElementsByTagName("NTP_SERVERS_PARAMETERS")[0].getElementsByTagName("NTP_SERVER_STATUS")[0].firstChild.data = ntp_server_status_1
		self.loadedXml.getElementsByTagName("NTP_SERVERS_PARAMETERS")[1].getElementsByTagName("NTP_IP_ADDRESS")[0].firstChild.data = ntp_ip_address_2
		self.loadedXml.getElementsByTagName("NTP_SERVERS_PARAMETERS")[1].getElementsByTagName("NTP_SERVER_STATUS")[0].firstChild.data = ntp_server_status_2
		self.loadedXml.getElementsByTagName("NTP_SERVERS_PARAMETERS")[2].getElementsByTagName("NTP_IP_ADDRESS")[0].firstChild.data = ntp_ip_address_3
		self.loadedXml.getElementsByTagName("NTP_SERVERS_PARAMETERS")[2].getElementsByTagName("NTP_SERVER_STATUS")[0].firstChild.data = ntp_server_status_3
		self.Send(expected_status)
		
		return(0)

