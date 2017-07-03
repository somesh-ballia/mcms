#!/mcms/python/bin/python

import os, string

#*PRERUN_SCRIPTS=TestBigCDRFilePrepare.py
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_12
#*export CLEAN_CDR=NO

from McmsConnection import *

def PrintWithoutNewLine(str):
    sys.stdout.write(str)
    sys.stdout.flush()



def GetFullCdr(cdrSummaryList):

    i = 1
    for summary in cdrSummaryList:
    
        id = summary.getElementsByTagName("ID")[0].firstChild.data
    
        message = "Perform Get Full : iteration = " + str(i) + " ; ID = " + id + "..."
        i = i + 1
        PrintWithoutNewLine(message)
        
        cGetCdrList.LoadXmlFile("Scripts/GetCDRFull.xml")
        cGetCdrList.ModifyXml("GET","ID", id)
        cGetCdrList.Send("Status OK")
	#if we want to test the response----------
        #print cGetCdrList.xmlResponse.toprettyxml()
	#-----------------------------------------
        print "OK"


    
##--------------------------------------- TEST ---------------------------------

cGetCdrList = McmsConnection()
cGetCdrList.Connect()


print "Perform Get CDR list ..."
cGetCdrList.SendXmlFile('Scripts/GetCDRList.xml')
print "OK"


cdrSummaryList = cGetCdrList.xmlResponse.getElementsByTagName("CDR_SUMMARY")
GetFullCdr(cdrSummaryList)


cGetCdrList.Disconnect()

##------------------------------------------------------------------------------
