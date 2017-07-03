#!/mcms/python/bin/python

#*export CLEAN_CDR=NO

#*PRERUN_SCRIPTS=TestCDRPrepare.py
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_12

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
        
        print "OK"


    
##--------------------------------------- TEST ---------------------------------

cGetCdrList = McmsConnection()
cGetCdrList.Connect()


#cGetCdrFull = McmsConnection()
#cGetCdrFull.Connect()


print "Perform Get CDR list ..."
cGetCdrList.SendXmlFile('Scripts/GetCDRList.xml')
print "OK"


cdrSummaryList = cGetCdrList.xmlResponse.getElementsByTagName("CDR_SUMMARY")
GetFullCdr(cdrSummaryList)


cGetCdrList.Disconnect()
#cGetCdrFull.Disconnect()

##------------------------------------------------------------------------------
