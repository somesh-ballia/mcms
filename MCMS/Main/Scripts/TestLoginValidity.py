#!/mcms/python/bin/python

from McmsConnection import *
import os
import sys

expected_status="Invalid Login"

c = McmsConnection()
c.Connect()
c.LoadXmlFile("Scripts/login.xml")
c.ModifyXml("LOGIN", "USER_NAME", "AUDIT")
c.ModifyXml("LOGIN", "PASSWORD", "AUDIT111111111111111111111111111111111111111")
c.Send(expected_status)
c.Disconnect()

c1 = McmsConnection()
c1.Connect()
c1.LoadXmlFile("Scripts/login.xml")
c1.ModifyXml("LOGIN", "USER_NAME", "AUDIT111111111111111111111111111111111111111")
c1.ModifyXml("LOGIN", "PASSWORD", "AUDIT")
c1.Send(expected_status)
c1.Disconnect()