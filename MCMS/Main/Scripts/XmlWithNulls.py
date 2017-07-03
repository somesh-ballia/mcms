#!/mcms/python/bin/python

import os
import sys
from McmsConnection import *

#-- SKIP_ASSERTS

if __name__ == '__main__':

	connection = McmsConnection()
	connection.Connect()
	f = open("Scripts/NullsInTheMiddle.xml",'r')
	connection.PostString(f.read(),"Field action is missing or invalid")
	f = open("Scripts/NullsInTheEnd.xml",'r')
	connection.PostString(f.read(),"Transaction is not well formed")

	connection.Disconnect()
