#!/mcms/python/bin/python

import sys

from McmsConnection import *
from TimeSettingUtils import *


timeSettingUtils = TimeSettingUtils()
timeSettingUtils.Connect()

if len(sys.argv) == 11:
	timeSettingUtils.SetCurrentTimeSetting(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4],sys.argv[5], sys.argv[6], sys.argv[7], sys.argv[8], sys.argv[9], sys.argv[10])
elif len(sys.argv) == 1:
	timeSettingUtils.GetCurrentTimeSetting()
else:
	print "Invalid number of arguments"
	print "Usage: " + sys.argv[0] + "    MCU_BASE_TIME GMT_OFFSET_SIGN GMT_OFFSET IS_NTP {NTP_IP_ADDRESS NTP_SERVER_STATUS}(3)"
	print "    or " + sys.argv[0]

timeSettingUtils.Disconnect()
