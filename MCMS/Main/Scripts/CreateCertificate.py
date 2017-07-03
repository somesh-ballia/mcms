#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2


from McmsConnection import *
from CreateCertificateUtil import *

#Create certificate to the MCU 

c = McmsConnection()
c.Connect()

sleep (2)

StartSecuredConnectionWithoutCertificate(c)
CreateCertificateRequestWithEmptyFields(c)
CreateCertificateWithMisMatchKeys(c)
CreateCertificateWithWrongDNS(c)
CreateGoodCertificate(c)
SendCertificateAgain(c)

c.Disconnect()


