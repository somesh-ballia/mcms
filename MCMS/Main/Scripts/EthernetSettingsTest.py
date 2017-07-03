#!/mcms/python/bin/python

import sys

from McmsConnection import *
from EthernetSettingsUtils import *


ethernetSettingsUtils = EthernetSettingsUtils()
ethernetSettingsUtils.Connect()

if len(sys.argv) == 5:
	ethernetSettingsUtils.UpdateCurrentEthernetSettings(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
elif len(sys.argv) == 1:
	ethernetSettingsUtils.GetCurrentEthernetSettings()
else:
	print "Invalid number of arguments"
	print "Usage: " + sys.argv[0] + "    Slot Port Speed"
	print "    or " + sys.argv[0]
	print
	print "    speed could be: speed_auto, speed_10, speed_10_duplex_full, speed_100, speed_100_duplex_full, speed_1000, speed_1000_duplex_full"

ethernetSettingsUtils.Disconnect()
