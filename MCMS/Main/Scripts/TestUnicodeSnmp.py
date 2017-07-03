#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_16


from McmsConnection import *



def SendPrint(conn, title, expectedStatus):
    sys.stdout.write("Sends " + title + " in not ASCII")
    sys.stdout.flush()

    conn.Send(expectedStatus)

    sys.stdout.write(" ---- OK")
    sys.stdout.flush()
    
    print ""


conn = McmsConnection()
conn.Connect()

conn.LoadXmlFile('Scripts/TransSnmpSet.xml')


unicodeLocation = u"куку-лулу"
unicodeContactName = u"куку-лулу"
unicodeSysName = u"куку-лулу"
unicodeCommunityName =  u"куку-лулу"

expectedRejectStatus = "Field Value must be ASCII"

conn.ModifyXml("SNMP_DATA","LOCATION", unicodeLocation)
SendPrint(conn, "Location", expectedRejectStatus)
conn.ModifyXml("SNMP_DATA","LOCATION", "cucu lulu land")


conn.ModifyXml("SNMP_DATA","CONTACT_NAME", unicodeContactName)
SendPrint(conn, "Contact Name", expectedRejectStatus)
conn.ModifyXml("SNMP_DATA","CONTACT_NAME", "cucu_lulu name")


conn.ModifyXml("SNMP_DATA","SYSTEM_NAME", unicodeContactName)
SendPrint(conn, "System Name", expectedRejectStatus)
conn.ModifyXml("SNMP_DATA","SYSTEM_NAME", "cucu lulu system")

conn.ModifyXml("COMMUNITY_PERMISSION","COMMUNITY_NAME", unicodeCommunityName)
SendPrint(conn, "Community Name", expectedRejectStatus)
conn.ModifyXml("SNMP_DATA","SYSTEM_NAME", "cucu lulu community")




