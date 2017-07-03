#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2


from McmsConnection import *

def UpdateManagementNetwork(c, dns_name=""):
	c.LoadXmlFile("Scripts/UpdateNetworkService.xml")
	if (dns_name!=""):
		c.ModifyXml("IP_SPAN","HOST_NAME",dns_name)
	c.Send()

def CreateCertificateRequest(c, update_dns=1, empty_field="none", expected_status="Status OK"):
	c.LoadXmlFile("Scripts/TLS/CreateCertificateRequest.xml")
	
	if empty_field == "country_name":
		c.ModifyXml("CERTIFICATE_DATA","COUNTRY_NAME","")
		
	if empty_field == "state_province":
		c.ModifyXml("CERTIFICATE_DATA","STATE_OR_PROVINCE_NAME","")
		
	if empty_field == "locality":
		c.ModifyXml("CERTIFICATE_DATA","LOCALITY_NAME","")
	
	if empty_field == "organization_name":
		c.ModifyXml("CERTIFICATE_DATA","ORGANIZATION_NAME","")
		
	if empty_field == "organization_unit_name":
		c.ModifyXml("CERTIFICATE_DATA","ORGANIZATION_UNIT_NAME","")
	
	if update_dns == 1:
		dns_name = os.popen( "uname -n" ).read()
		dns_name = dns_name.replace('\n','')
		dns_name = dns_name.replace('.','-')
		c.ModifyXml("CERTIFICATE_DATA","COMMON_NAME",dns_name)
	
	c.Send(expected_status)
	
def CreateCertificate(c, update_certificate="update", expected_status="Status OK", create_self_sign_cert="true"):
	
	if create_self_sign_cert == "true":
		os.popen("openssl x509 -req -days 60 -in Keys/temp_cert.csr -signkey Keys/temp_private.pem -out Keys/temp_cert_off.pem -passin pass:polycom")
	
	if os.path.isfile("Keys/temp_cert_off.pem"):	
		infile = open("Keys/temp_cert_off.pem", "r")
		line = infile.read()
		infile.close()

	dns_name = os.popen( "uname -n" ).read()
	dns_name = dns_name.replace('\n','')
	dns_name = dns_name.replace('.','-')
	UpdateManagementNetwork(c, dns_name)
	
	c.LoadXmlFile("Scripts/TLS/SendCertificate.xml")
	
	if update_certificate == "gibrish":
		c.ModifyXml("SEND","CERTIFICATE","Judith Shuva's Gibrish Certificate")
	if update_certificate == "update":
		c.ModifyXml("SEND","CERTIFICATE",line)
	
	c.Send(expected_status)

def StartSecuredConnectionWithoutCertificate(c):
	print "===Start secured connection without certificate==="
	if os.path.isfile("tmp/privateKey.pem"):
		os.remove("tmp/privateKey.pem")
	if os.path.isfile("Keys/cert_off.pem"):
		os.remove("Keys/cert_off.pem")
	c.LoadXmlFile("Scripts/UpdateNetworkService.xml")
	c.ModifyXml("IP_SERVICE","IS_SECURED","true")
	c.Send("The connection cannot be secured because an SSL/TLS certificate cannot be found")
	
def CreateCertificateRequestWithEmptyFields(c):
	print "===Create certificate request with empty fields==="
	print "Empty country name"
	CreateCertificateRequest(c, 1,"country_name","Invalid country name")
	print "Empty state or province"
	CreateCertificateRequest(c, 1,"state_province","Invalid state or province name")
	print "Empty locality"
	CreateCertificateRequest(c, 1,"locality","Invalid locality name")
	print "Empty organization name"
	CreateCertificateRequest(c, 1,"organization_name","Invalid organization name")
	print "Empty state or province"
	CreateCertificateRequest(c, 1,"organization_unit_name","Invalid organizational unit name")
	
def CreateCertificateWithMisMatchKeys(c):
	print "===Send certificate that doesn't match the private key==="
	
	CreateCertificateRequest(c)
	CreateCertificate(c, "gibrish", "Cannot install the SSL/TLS certificate")
	CreateCertificate(c, "no_update", "Cannot install the SSL/TLS certificate")
	
def CreateCertificateWithWrongDNS(c):
	print "===Create certificate with wrong DNS==="

	CreateCertificateRequest(c, 0)	
	CreateCertificate(c, "update", "Certificate host name does not match the RMX host name")
		
def CreateGoodCertificate(c):
	print "===Create certificate==="
	
	CreateCertificateRequest(c)
	CreateCertificate(c)
	
def SendCertificateAgain(c):
	print "===Send same certificate again==="
	CreateCertificate(c, "", "Cannot install the SSL/TLS certificate", "false")


