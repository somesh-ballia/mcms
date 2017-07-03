#!/mcms/python/bin/python


# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2


from McmsConnection import *

c = McmsConnection()

# --------------- IPV4 ---------------
c.Connect()
print "Send LOGIN transaction with IpV4 adderss ..."
c.LoadXmlFile('Scripts/TestActionTag/LoginWithActionTag.xml')
c.ModifyXml("LOGIN","IP","127.0.0.1")
c.ModifyXml("LOGIN","USER_NAME","SUPPORT")
c.ModifyXml("LOGIN","PASSWORD" ,"SUPPORT")
c.Send("Status OK")
c.Disconnect()


# --------------- IPV6 ---------------
c.Connect()
print "Send LOGIN transaction with IpV6 adderss ..."
c.LoadXmlFile('Scripts/TestActionTag/LoginWithActionTag.xml')
c.ModifyXml("LOGIN","IP","2001:db8:0:1:213:21ff:feb5:f266")
c.ModifyXml("LOGIN","USER_NAME","SUPPORT")
c.ModifyXml("LOGIN","PASSWORD" ,"SUPPORT")
c.Send("Status OK")
sleep(120)
c.Disconnect()
