#!/mcms/python/bin/python
#-LONG_SCRIPT_TYPE
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

#*export SYSTEM_CFG_USER_FILE=Scripts/ExternalDbOperations/system_cfg.xml
import os
import sys
import shutil
import glob

from SysCfgUtils import *
from UsersUtils import *
from McmsConnection import *


def SetSystemParams(connection):
    print " -> Set SYSTEM.CFG parameters to External DB support "
    #    EXTERNAL_DB_IP=10.227.1.9      #this is isrwebcomm2003 Server
    #    EXTERNAL_DB_PORT=5005
    #    EXTERNAL_DB_LOGIN=POLYCOM
    #    EXTERNAL_DB_PASSWORD=POLYCOM
    #    EXTERNAL_DB_DIRECTORY=A
    #sleep (1)
    connection.LoadXmlFile('Scripts/ExternalDbOperations/system_cfg.xml')
    connection.Send()
    return


def GetNumOfIvrServices(connection):
    #print "Monitoring IVR service list..."
    connection.SendXmlFile('Scripts/GetIvrList.xml')
    ivr_services = connection.xmlResponse.getElementsByTagName("IVR_SERVICE")
    num_ivr_services = len(ivr_services)
    #print "num_ivr_services = ", num_ivr_services
    return num_ivr_services


def CreateServices(connection,eq_service_name,ivr_service_name):
    num_serv_before = GetNumOfIvrServices(connection)
    print " -> Adding new EQ service (", eq_service_name, ") ..."
    connection.LoadXmlFile('Scripts/ExternalDbOperations/AddEqService.xml')
    connection.ModifyXml("AV_COMMON","SERVICE_NAME",eq_service_name)
    connection.Send()
    print " -> Adding new IVR service (", ivr_service_name, ") ..."
    connection.LoadXmlFile('Scripts/ExternalDbOperations/AddIvrService.xml')
    connection.ModifyXml("AV_COMMON","SERVICE_NAME",ivr_service_name)
    connection.Send()
    # wait up to 10 seconds till services will appear
    retries = 10
    for retry in range(retries+1):
        sleep(1)
        connection.SendXmlFile('Scripts/GetIvrList.xml')
        num_serv_now = GetNumOfIvrServices(connection)
        if num_serv_now == num_serv_before+2 :
            break
        if retry == retries:
            return False
    return True


def CreateEntryQ(connection,eq_name,eq_service_name,NumericID):
    print " -> Adding new EQ (", eq_name, ") ..."
    connection.LoadXmlFile('Scripts/ExternalDbOperations/AddEntryQ.xml')
    connection.ModifyXml("RESERVATION","NAME",eq_name)
    connection.ModifyXml("RESERVATION","NUMERIC_ID",NumericID)
    connection.ModifyXml("RESERVATION","AV_MSG",eq_service_name)
    connection.Send()
    return True


def TestConfDetails(connection,eq_name,eq_numeric_id):
    print " -> External DB CONF_DETAILS Test"
    #expected_conf_name = "Amir"
    expected_conf_name = "91002"
    partyName = "my_party_1"
    partyId_In_Db = "91002"

    print " -> -> Dial In from endpoint..."
    connection.SimulationAddH323Party(partyName,eq_numeric_id)
    connection.SimulationConnectH323Party(partyName)

    print " -> -> Wait EQ awakes..."
    eq_conf_id = connection.WaitUntillEQorMRAwakes(eq_name,1,10,True)

    print " -> -> Send DTMF from endpoint..."
    connection.SimulationH323PartyDTMF(partyName,partyId_In_Db)

    print " -> -> Wait conference (" + expected_conf_name + ") creates..."
    conf_id = connection.WaitConfCreated(expected_conf_name,30)

    print " -> -> Sleep 10 sec..."
    sleep(10)

    print " -> -> Cleanup..."
    connection.DeleteConf(conf_id)
    connection.DeleteConf(eq_conf_id)
    connection.WaitAllConfEnd(30)
    connection.SimulationDeleteH323Party(partyName)
    print " -> End External DB CONF_DETAILS test"
    return True


def TestPartyDetails(connection,ivr_service_name,conf_numeric_id):
    print " -> External DB PARTY_DETAILS Test"
    conf_name = "TEST_CONFERENCE_PARTY_DETAILS"
    partyName = "my_party_2"
    partyId_In_Db = "91002"

    print " -> -> Change default IVR service ..."
    connection.LoadXmlFile('Scripts/ExternalDbOperations/SetDefaultIvrService.xml')
    connection.ModifyXml("SET_DEFAULT","SERVICE_NAME",ivr_service_name)
    connection.Send()

    sleep(10)
    
    print " -> -> Create conference ..."
    connection.LoadXmlFile('Scripts/ExternalDbOperations/AddCpConf.xml')
    connection.ModifyXml("RESERVATION","NAME",conf_name)
    connection.ModifyXml("RESERVATION","NUMERIC_ID",conf_numeric_id)
    #connection.ModifyXml("RESERVATION","PASSWORD","2222")
    #connection.ModifyXml("RESERVATION","ENTRY_PASSWORD","1111")
    connection.Send()

    print " -> -> Wait conference (" + conf_name + ") creates..."
    conf_id = connection.WaitConfCreated(conf_name,10)

    print " -> -> Dial In from endpoint..."
    connection.SimulationAddH323Party(partyName,conf_numeric_id)
    connection.SimulationConnectH323Party(partyName)

    print " -> -> Sleep 2 sec..."
    sleep(2)

    print " -> -> Wait all parties..."
    connection.WaitAllPartiesWereAdded(conf_id,1,10)# num_parties, retries
    connection.WaitAllOngoingConnected(conf_id)
    print " -> -> Send DTMF from endpoint..."
    connection.SimulationH323PartyDTMF(partyName,"1111")

    print " -> -> Sleep 10 sec..."
    sleep(10)

    print " -> -> Cleanup..."
    connection.DeleteConf(conf_id)
    connection.WaitAllConfEnd(30)
    connection.SimulationDeleteH323Party(partyName)
    print " -> End External DB PARTY_DETAILS test"


def TestPartyDetailsUTF8(connection,ivr_service_name,conf_numeric_id):
    print " -> External DB PARTY_DETAILS Test"
    conf_name = "TEST_CONFERENCE_PARTY_DETAILS_UTF8"
    partyName = "my_party_2"
    partyId_In_Db = "91006"

    print " -> -> Change default IVR service ..."
    connection.LoadXmlFile('Scripts/ExternalDbOperations/SetDefaultIvrService.xml')
    connection.ModifyXml("SET_DEFAULT","SERVICE_NAME",ivr_service_name)
    connection.Send()

    sleep(10)
    
    print " -> -> Create conference ..."
    connection.LoadXmlFile('Scripts/ExternalDbOperations/AddCpConf.xml')
    connection.ModifyXml("RESERVATION","NAME",conf_name)
    connection.ModifyXml("RESERVATION","NUMERIC_ID",conf_numeric_id)
    #connection.ModifyXml("RESERVATION","PASSWORD","2222")
    #connection.ModifyXml("RESERVATION","ENTRY_PASSWORD","1111")
    connection.Send()

    print " -> -> Wait conference (" + conf_name + ") creates..."
    conf_id = connection.WaitConfCreated(conf_name,10)

    print " -> -> Dial In from endpoint..."
    connection.SimulationAddH323Party(partyName, conf_numeric_id)
    connection.SimulationConnectH323Party(partyName)

    print " -> -> Sleep 2 sec..."
    sleep(2)

    print " -> -> Wait all parties..."
    connection.WaitAllPartiesWereAdded(conf_id,1,10)# num_parties, retries
    connection.WaitAllOngoingConnected(conf_id)
    print " -> -> Send DTMF from endpoint..."
    connection.SimulationH323PartyDTMF(partyName,"1111")

    print " -> -> Sleep 10 sec..."
    sleep(10)
    
    print " -> -> Cleanup..."
    connection.DeleteConf(conf_id)
    connection.WaitAllConfEnd(30)
    connection.SimulationDeleteH323Party(partyName)
    print " -> End External DB PARTY_DETAILS test"


def TestExAsciiBlocking(connection,ivr_service_name,conf_numeric_id):
    print " -> External DB responds in ExAscii encoding"
    conf_name = "TEST_CONFERENCE_EX_ASCII"
    partyName = "my_party_2"
#    partyId_In_Db = "91007"

    print " -> -> Change default IVR service ..."
    connection.LoadXmlFile('Scripts/ExternalDbOperations/SetDefaultIvrService.xml')
    connection.ModifyXml("SET_DEFAULT","SERVICE_NAME",ivr_service_name)
    connection.Send()

    sleep(10)
    
    print " -> -> Create conference ..."
    connection.LoadXmlFile('Scripts/ExternalDbOperations/AddCpConf.xml')
    connection.ModifyXml("RESERVATION","NAME",conf_name)
    connection.ModifyXml("RESERVATION","NUMERIC_ID",conf_numeric_id)
    #connection.ModifyXml("RESERVATION","PASSWORD","2222")
    #connection.ModifyXml("RESERVATION","ENTRY_PASSWORD","1111")
    connection.Send()

    print " -> -> Wait conference (" + conf_name + ") creates..."
    conf_id = connection.WaitConfCreated(conf_name,10)

    print " -> -> Dial In from endpoint..."
    connection.SimulationAddH323Party(partyName, conf_numeric_id)
    connection.SimulationConnectH323Party(partyName)

    print " -> -> Sleep 2"
    sleep(2)

    print " -> -> Wait all parties..."
    connection.WaitAllPartiesWereAdded(conf_id,1,10)# num_parties, retries
    connection.WaitAllOngoingConnected(conf_id)
    print " -> -> Send DTMF from endpoint..."
    connection.SimulationH323PartyDTMF(partyName,"1111")

    print " -> -> Sleep 2"
    sleep(2)
    
    os.system("Scripts/Flush.sh")

    outputFileName = "tmpTerminalCommandOutput.yura"
    
    grepCommand = "cat LogFiles/Log_SN00000000*.txt"
    grepCommand = grepCommand + " | grep -A5 \"CQAAPIRxSocket::ReceiveFromSocket\""
    grepCommand = grepCommand + " | grep -A4 \"Drop the packet\""
#    grepCommand = grepCommand + " | grep -A3 \"the encoding is unknown: UTF-8\""
    grepCommand = grepCommand + " > " + outputFileName
    print "Grep Command=" + grepCommand
    os.system(grepCommand)

    outputFile = open(outputFileName)
    outputLine = outputFile.readline()
    outputFile.close()

    command = "rm " + outputFileName
    os.system(command)
    
    if(outputLine == ""):
        print "ERROR: the drop packet was not found in the log files"
        sys.exit(1)

    print "Extended ASCII was blocked at the QAAPI process, Very Good"


def TestConfDetailsDisplayName(connection,eq_name,eq_numeric_id):
    print " -> External DB CONF_DETAILS DISPLAY_NAME Test"
    #expected_conf_name = "Judith"
    expected_conf_name = "91008"
    partyName = "my_party_1"
    partyId_In_Db = "91008"

    print " -> -> Dial In from endpoint..."
    connection.SimulationAddH323Party(partyName,eq_numeric_id)
    connection.SimulationConnectH323Party(partyName)

    print " -> -> Wait EQ awakes..."
    eq_conf_id = connection.WaitUntillEQorMRAwakes(eq_name,1,10,True)

    print " -> -> Send DTMF from endpoint..."
    connection.SimulationH323PartyDTMF(partyName,partyId_In_Db)

    print " -> -> Wait conference (" + expected_conf_name + ") creates..."
    conf_id = connection.WaitConfCreated(expected_conf_name,20)

    print " -> -> Sleep 10 sec..."
    sleep(10)

    print " -> -> Cleanup..."
    connection.DeleteConf(conf_id)
    connection.DeleteConf(eq_conf_id)
    connection.WaitAllConfEnd(30)
    connection.SimulationDeleteH323Party(partyName)
    print " -> End External DB CONF_DETAILS DISPLAY_NAME test"
    return True


def TestExternalDbOperations():
    print
    print "Start External DB operations test"
    
    # ----- Define parameters
    eq_service_name = "TEST_EQ_SERVICE"
    ivr_service_name = "TEST_IVR_SERVICE"
    eq_numeric_id = 1501
    eq_name = "TEST_ENTRY_Q"
    conf_numeric_id = 91001

    # u need this code only if u running this script directly it is recommended to run this script using the run_night_test.sh or RunTest.sh
    # ----- Set SYSTEM.CFG parameters
    #conn = McmsConnection()
    #conn.Connect()
    #sleep(1)
    #SetSystemParams(conn)

    #os.environ["CLEAN_CFG"]="NO"
    #print "removing profiles data so confparty won't raise assert"
    #currentPath = os.getcwd()
    #os.chdir('Cfg/Profiles/')
    #filelist = glob.glob('*.xml')
    #for f in filelist:
    #	os.remove(f)
    #os.chdir(currentPath)
    #os.chdir('Cfg/MeetingRooms/')
    #filelist = glob.glob('*.xml')
    #for f in filelist:
    #	os.remove(f)
    #os.chdir(currentPath)
    #os.system("Scripts/Startup.sh")

    #    print "Wait 30s after reset - Apache should set up...."
    #print " -> Wait 15s for external DB socket to connect..."
    #sleep(15)

    conn = McmsConnection()
    conn.Connect()

    sleep(1)

    # ----- Create EQ and IVR services for test
    CreateServices(conn,eq_service_name,ivr_service_name)

    sleep(1)

    # ----- Create EQ
    CreateEntryQ(conn,eq_name,eq_service_name,eq_numeric_id)

    sleep(5)

    # ----- External DB CONF_DETAILS test
    TestConfDetails(conn,eq_name,eq_numeric_id)
    
    sleep(5)
   
    # ----- External DB CONF_DETAILS DISPLAY_NAME test
    TestConfDetailsDisplayName(conn,eq_name,eq_numeric_id)
    
    sleep(5)

    # ----- External DB PARTY_DETAILS test
    print "Test Party Details 91001"
    TestPartyDetails(conn,ivr_service_name,conf_numeric_id)

    sleep(5)

    print "Test Party Details 91006"
    TestPartyDetailsUTF8(conn,ivr_service_name, "91006")

    sleep(5)
    
    print "Test ExAscii blocking 91007"
    TestExAsciiBlocking(conn,ivr_service_name, "91007")

    sleep(5)
	
    # ----- End
    print "End of External DB operations test"

    conn.Disconnect()
    os.environ["CLEAN_CFG"]="YES"




## ---------------------- Test --------------------------

## outputFileName = "tmpTerminalCommandOutput.yura"
    
## grepCommand = "cat LogFiles/Log00001.txt"
## grepCommand = grepCommand + " | grep -A5 \"CQAAPIRxSocket::ReceiveFromSocket Drop the packet\""
## grepCommand = grepCommand + " | grep -A4 \"EILSEQ : Input conversion stopped due to an input byte that does not belong to the input codeset.\""
## grepCommand = grepCommand + " | grep -A3 \"Encoding validation failed. UTF-8 -> UTF-8\""
## grepCommand = grepCommand + " | grep \"\u0414\u0410\""
## grepCommand = grepCommand + " > " + outputFileName

## print
## print grepCommand
## print

    
TestExternalDbOperations()





























