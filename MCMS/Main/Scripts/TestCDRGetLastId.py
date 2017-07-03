#!/mcms/python/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_17

from McmsConnection import *



def CreateCDRs():
    os.system("Scripts/Add20ConferenceNew.py")

def PerformResetWithoutCleaningCDRs():
    os.system("Scripts/Destroy.sh")
    os.environ["CLEAN_CDR"]="NO"
    os.system("Scripts/Startup.sh")

def GetMaxIdViaXML():
    cGetCdrList = McmsConnection()
    cGetCdrList.Connect()
    
    print "Perform Get CDR list ..."
    cGetCdrList.SendXmlFile('Scripts/GetCDRList.xml')
    print "OK"
    
    maxIdViaXML = -1
    cdrSummaryList = cGetCdrList.xmlResponse.getElementsByTagName("CDR_SUMMARY")
    for summary in cdrSummaryList:
        strCurrentID = summary.getElementsByTagName("ID")[0].firstChild.data
        intCurrentID = int(strCurrentID)
        if(intCurrentID > maxIdViaXML):
            maxIdViaXML = intCurrentID

    cGetCdrList.Disconnect()
       
    return maxIdViaXML

def GetMaxIdViaTerminal():
    outputFileName = "tmpTerminalCommandOutput.yura"
    commandLine = "Bin/McuCmd @get_last_conf_id CDR > " + outputFileName
    os.system(commandLine)
    
    header = "Last Conf Id : "
    headerLen = len(header)
    commandOutput = open(outputFileName);

    found = "NO"
    line = "Cucu_lulu"
    while (line != ""):
        line = commandOutput.readline()
        if(line[0:headerLen] == header):
            found = "YES"
            break
        
    commandOutput.close()   
    os.system("rm " + outputFileName)
    
    if(found == "NO"):
        print "FAILED to get last conf id via terminal command"
        sys.exit(1)
            
    listCommand = line.split(" ") #line = Last Conf Id : 10
    strIdViaTerminal = listCommand[4]
    maxIdViaTerminal = int(strIdViaTerminal)

    return maxIdViaTerminal




##--------------------------------------- TEST ---------------------------------


CreateCDRs()
PerformResetWithoutCleaningCDRs()
maxIdViaXML = GetMaxIdViaXML()
maxIdViaTerminal = GetMaxIdViaTerminal()

strIdCmp = "via terminal : " + str(maxIdViaTerminal) + " ; via XML : " + str(maxIdViaXML)

if(maxIdViaTerminal != maxIdViaXML):
    print "the IDs are not the same; " + strIdCmp
    sys.exit(2)

print "the IDs OK ; " +  strIdCmp

sys.exit(0)


##--------------------------------------- TEST ---------------------------------
