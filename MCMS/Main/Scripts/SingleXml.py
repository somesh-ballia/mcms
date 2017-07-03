#!/usr/bin/python

from McmsConnection import *

c = McmsConnection()
c.Connect()

c.LoadXmlFile(sys.argv[1])
c.Send()
c.PrintLastResponse()


c.Disconnect()
