#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2


from McmsConnection import *
from CreateCertificateUtil import *

#Create certificate to the MCU 

c = McmsConnection()
c.Connect()

sleep (2)

if os.path.isfile("CACert/temp_ca_cert.cer"):	
	infile = open("CACert/temp_ca_cert.cer", "r")
	line = infile.read()
	infile.close()

c.LoadXmlFile("Scripts/TLS/SendCACertificate.xml")
c.ModifyXml("SEND_CA","CERTIFICATE",line)

c.Send()

c.Disconnect()


