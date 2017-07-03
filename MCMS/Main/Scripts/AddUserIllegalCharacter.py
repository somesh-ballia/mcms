#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6
import os
import sys
import shutil

from UsersUtils import *


#c = McmsConnection()
#c.Connect()
#expected_status="Status OK"
#c.LoadXmlFile('Scripts/AddUserIllegalCharacter/AddUserIllegalCharacter_login.xml') 
#c.Send(expected_status)
#c.Disconnect()



import os
import sys
import shutil

from UsersUtils import *




userName = "שגיא"
#userName = u"äöü".encode('utf-8')
#userName = u'USER_\xc3'
#userNameConv = userName.encode("utf-8")
#userName = "USER_à"
#userNameConv=unicode(userName,'utf-8')
print userName

p = UsersUtilities()
p.ConnectSpecificUser("SUPPORT","SUPPORT")
p.AddNewUser(userName, "USER_pwd","operator","Login Name contains invalid characters")

p.Disconnect()
