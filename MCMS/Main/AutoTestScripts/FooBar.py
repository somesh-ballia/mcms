#!/mcms/python/bin/python

import sys
import os 
sys.path.append(os.path.dirname(__file__) + "/../Scripts")

print os.path.dirname(__file__) + "/../Scripts" 

from McmsConnection import *

c = McmsConnection()

print "connect..."
c.Connect()

print "disconnect..."
c.Disconnect()

print "the end"
