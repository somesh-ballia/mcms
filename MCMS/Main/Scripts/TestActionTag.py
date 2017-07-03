#!/mcms/python/bin/python


# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2


from McmsConnection import *

c = McmsConnection()

# --------------- LOGIN ---------------
c.Connect()
print "Send LOGIN transaction with 'Action' tag ..."
c.LoadXmlFile('Scripts/TestActionTag/LoginWithActionTag.xml')
c.ModifyXml("LOGIN","IP","127.0.0.1")
c.ModifyXml("LOGIN","USER_NAME","SUPPORT")
c.ModifyXml("LOGIN","PASSWORD" ,"SUPPORT")
c.Send("Status OK")
c.Disconnect()

c.Connect()
print "Send LOGIN transaction without 'Action' tag ..."
c.LoadXmlFile('Scripts/TestActionTag/LoginWithoutActionTag.xml')
c.ModifyXml("LOGIN","IP","127.0.0.1")
c.ModifyXml("LOGIN","USER_NAME","SUPPORT")
c.ModifyXml("LOGIN","PASSWORD" ,"SUPPORT")
c.Send("Status OK")
c.Disconnect()


# ------------- GET_STATE -------------
c.Connect()
print "Send GET_STATE transaction with 'Action' tag ..."
c.LoadXmlFile('Scripts/TestActionTag/GetStateWithActionTag.xml')
c.Send("Status OK")
c.Disconnect()

c.Connect()
print "Send GET_STATE transaction without 'Action' tag ..."
c.LoadXmlFile('Scripts/TestActionTag/GetStateWithoutActionTag.xml')
c.Send("Status OK")
c.Disconnect()

# ----------- ADD/DELETE_USER ---------
c.Connect()
print "Send ADD_NEW_USER transaction with 'Action' tag ..."
c.LoadXmlFile('Scripts/TestActionTag/AddNewUserWithActionTag.xml')
c.ModifyXml("NEW_OPERATOR","USER_NAME","WithActionKey")
c.ModifyXml("NEW_OPERATOR","PASSWORD", "WithActionKey")
c.Send("Status OK")
c.Disconnect()

c.Connect()
print "Send DELETE_USER transaction with 'Action' tag ..."
c.LoadXmlFile('Scripts/TestActionTag/DeleteUserWithActionTag.xml')
c.ModifyXml("DELETE_OPERATOR","USER_NAME","WithActionKey")
c.Send("Status OK")
c.Disconnect()


c.Connect()
print "Send ADD_NEW_USER transaction without 'Action' tag ..."
c.LoadXmlFile('Scripts/TestActionTag/AddNewUserWithoutActionTag.xml')
c.ModifyXml("NEW_OPERATOR","USER_NAME","WithoutActionKey")
c.ModifyXml("NEW_OPERATOR","PASSWORD", "WithoutActionKey")
c.Send("Status OK")
c.Disconnect()

c.Connect()
print "Send DELETE_USER transaction without 'Action' tag ..."
c.LoadXmlFile('Scripts/TestActionTag/DeleteUserWithoutActionTag.xml')
c.ModifyXml("DELETE_OPERATOR","USER_NAME","WithoutActionKey")
c.Send("Status OK")
c.Disconnect()
