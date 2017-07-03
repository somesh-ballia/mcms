#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2

from McmsConnection import *
from CreateCertificateUtil import *
from SecureConnectionUtil import *

def UpdateUnexistDnsAndStartSecureConnection():
	print "===Config the system in secure with wrong DNS name==="
	print "update dns name = Judith and secure flag = true"
	UpdateManagementNetworkForSecure(c, "Judith", "true", "Certificate host name does not match the RMX host name")

def UpdateDNSWhenTheSystemInSecureMode():
	print "===Update DNS name when the system is in secure mode==="
	UpdateManagementNetworkForSecure(c, "Judith", "true", "DNS name cannot be changed while in Secure Mode")
	
def ConferenceBlockTest():
	print "===Test conference block, when there is no valid private key==="
	#os.environ["TARGET_PORT"]="8080"
	os.environ["TARGET_PORT"]="80"
	
	print "Remove the private key"
	
	if os.path.isfile("Keys/private3.pem"):
		os.remove("Keys/private3.pem")
	
	print "kill start up"
	
#	os.environ["CLEAN_CFG"]="YES"
	
	os.system("Scripts/Destroy.sh")
	print "start again in secure mode with no private key"
	os.system("Scripts/Startup.sh")

	print "Adding Conf..."
	c.Connect()
	c.SendXmlFile("Scripts/AddVideoCpConf.xml", "Conference Creation is blocked")

c = McmsConnection()

os.environ["CLEAN_CFG"]="NO"
os.environ["MPL_SIM_FILE"]="VersionCfg/MPL_SIM_SECURED_MPMX.XML"

#c.Connect()

#CreateGoodCertificate(c)

#c.Disconnect()

os.environ["RMX_IN_SECURE"]="YES"
#UpdateUnexistDnsAndStartSecureConnection()
StartSecureConnection(c)

#UpdateDNSWhenTheSystemInSecureMode()

#start secure connection without certificate
#this test still need to be tested
#ConferenceBlockTest()

