#!/mcms/python/bin/python

# #############################################################################
# Creating Simulation capset with H264 aspect ratio different from default
#
# Date: 29/4/07
# By  : Vasily 

#
############################################################################
#
#

#from CapabilitiesSetsDefinitions import *
from McmsConnection import *


def TestCreateSimCapsWithDifferentH264():
    print
    print "Start TestCreateSimCapsWithDifferentH264 test"
    print

    c = McmsConnection()
    c.Connect()

    c.LoadXmlFile('Scripts/SimAddCapSet.xml')
    c.ModifyXml("ADD_CAP_SET","NAME","MyCapSetWithAspectRatio150")
    c.ModifyXml("VIDEO_H264_DETAILS","H264_ASPECT_RATIO",150)
    c.Send()

    c.Disconnect()



TestCreateSimCapsWithDifferentH264()