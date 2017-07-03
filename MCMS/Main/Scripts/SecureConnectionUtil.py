#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2


from McmsConnection import *

def UpdateManagementNetworkForSecure(c, dns_name="", is_secured="false", expected_status="Status OK"):
	c.Connect()
	c.LoadXmlFile("Scripts/UpdateNetworkService.xml")
	c.ModifyXml("IP_SERVICE","IS_SECURED",is_secured)
	if (dns_name!=""):
		c.ModifyXml("IP_SPAN","HOST_NAME",dns_name)
	c.Send(expected_status)
	c.Disconnect("false")

def ConnectInSecureMode():
		
	print "Try to connect in secure mode"
	os.environ["TARGET_PORT"]="443"

	print "kill start up"
		
	os.system("Scripts/Destroy.sh")
	print "start again - this time in secure mode"
	
	os.system("Scripts/Startup.sh")

def StartSecureConnection(c):
	print "===Start secure connection==="
	dns_name = os.popen( "uname -n" ).read()
	dns_name = dns_name.replace('.','-')
	dns_name = dns_name.replace('\n','')
	print "dns should be"
	print dns_name
	
	UpdateManagementNetworkForSecure(c, dns_name,"true")
	ConnectInSecureMode()

