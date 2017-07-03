#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2

from McmsConnection import *
from CreateCertificateUtil import *

def UpdateManagementNetwork(dns_name="", is_secured="false", expected_status="Status OK"):
	c.Connect()
	c.LoadXmlFile("Scripts/UpdateNetworkService.xml")
	c.ModifyXml("IP_SERVICE","IS_SECURED",is_secured)
	if (dns_name!=""):
		c.ModifyXml("IP_SPAN","HOST_NAME",dns_name)
	c.Send(expected_status)
	c.Disconnect("false")

def ConnectInSecureMode():
	print "===Start secured connection==="
		
	print "Try to connect in secure mode"
	os.environ["TARGET_PORT"]="443"
	c.Connect()
	c.Disconnect()

def StartSecureConnection():
	print "===Start secure connection==="
	dns_name = os.popen( "uname -n" ).read()
	dns_name = dns_name.replace('\n','')
	UpdateManagementNetwork(dns_name,"true")
	print "Wait 5 second until apache start"
	sleep(5)
	ConnectInSecureMode()
	
def UpdateUnexistDnsAndStartSecureConnection():
	print "===Config the system in secure with wrong DNS name==="
	print "update dns name = Judith and secure flag = true"
	UpdateManagementNetwork("Judith", "true", "Certificate host name does not match the RMX host name")

def UpdateDNSWhenTheSystemInSecureMode():
	print "===Update DNS name when the system is in secure mode==="
	UpdateManagementNetwork("Judith", "true", "DNS name cannot be changed while in Secure Mode")
	
def ConferenceBlockTest():
	print "===Test conference block, when there is no valid private key==="
	#os.environ["TARGET_PORT"]="8080"
	os.environ["TARGET_PORT"]="80"
	os.environ["MPL_SIM_FILE"]="VersionCfg/MPL_SIM_SECURED.XML"
	
	print "Remove the private key"
	
	if os.path.isfile("Keys/private.pem"):
		os.remove("Keys/private.pem")
	
	print "kill start up"
	
	os.system("Scripts/Destroy.sh")
	print "start again - this time in secure mode"
	os.system("Scripts/Startup.sh")

	print "Adding Conf..."
	c.Connect()
	c.SendXmlFile("Scripts/AddVideoCpConf.xml", "Conference Creation is blocked")

c = McmsConnection()
c.Connect()

CreateGoodCertificate(c)

c.Disconnect()

UpdateUnexistDnsAndStartSecureConnection()
StartSecureConnection()
UpdateDNSWhenTheSystemInSecureMode()

# start secure connection without certificate
ConferenceBlockTest()


