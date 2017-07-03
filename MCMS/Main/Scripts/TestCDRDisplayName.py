#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2


from McmsConnection import *



#os.system("Bin/McuCmd simulateleak ApacheModule 1024 1")

#expectedDisplayName = u"жопа"
expectedDisplayName = u"到处群魔乱舞"
param = expectedDisplayName.encode("utf-8")
cmdLine="Scripts/AutoRealVoip.py DISPLAY_NAME=" + param
os.system(cmdLine)


cGetCdrList = McmsConnection()
cGetCdrList.Connect()


print "Perform Get CDR list ..."
cGetCdrList.SendXmlFile('Scripts/GetCDRList.xml')
print "OK"


cdrSummaryList = cGetCdrList.xmlResponse.getElementsByTagName("CDR_SUMMARY")
cdrSummary = cdrSummaryList[0]
displayName = cdrSummary.getElementsByTagName("DISPLAY_NAME")[0].firstChild.data
id = cdrSummary.getElementsByTagName("ID")[0].firstChild.data

if(displayName <> expectedDisplayName):
    print str(displayName) + " <> " + expectedDisplayName.encode("utf-8")
    sys.exit(1)

print "Test Get CDR list successed. " + displayName.encode("utf-8") + " was received as expected ( " + expectedDisplayName.encode("utf-8") + " )"


cGetCdrFull = McmsConnection()
cGetCdrFull.Connect()

cGetCdrFull.LoadXmlFile("Scripts/GetCDRFull.xml")
cGetCdrFull.ModifyXml("GET","ID", id)
cGetCdrFull.Send("Status OK")


confStart10List = cGetCdrFull.xmlResponse.getElementsByTagName("CONF_START_10")    
confStart5Elem = confStart10List[0]

displayName = confStart5Elem.getElementsByTagName("DISPLAY_NAME")[0].firstChild.data
if(displayName <> expectedDisplayName):
    print str(displayName) + " <> " + expectedDisplayName.encode("utf-8")
    sys.exit(1)

print "Test Get Ful CDR successed. " + displayName.encode("utf-8") + " was received as expected( " + expectedDisplayName.encode("utf-8") + " )"
