#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_5
 
from McmsConnection import *
c = McmsConnection()
c.Connect()
status = c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
conf_list = c.xmlResponse.getElementsByTagName("CONF_SUMMARY")
num_of_conf=len(conf_list)
num_ongoing_parties=0

for conf in conf_list:
	conf_id = conf.getElementsByTagName("ID")[0].firstChild.data
	
	c.LoadXmlFile('Scripts/TransConf2.xml')
        c.ModifyXml("GET","ID",conf_id)
        c.Send()
        ongoing_parties = c.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        num_ongoing_parties = len(ongoing_parties)

c.Disconnect()

print num_of_conf, num_ongoing_parties
sys.exit(num_ongoing_parties)

