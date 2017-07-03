#!/mcms/python/bin/python

#############################################################################
# Eror handling Script which Try to Add Conf with Min Particepent defined to various values
# 
# 1.Zero
# 3.In Limit - 20
# 2.Over Limit - 21
# 3.Parameter Limit 30 
#
# Date: 31/01/05
# By  : Yoella.
# Re-Write date = 1/8/13
# Re-Write name = Uri A.
#
#############################################################################

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8

from McmsConnection import *

#------------------------------------------------------------------------------
def AddConfMinParticepentsDefined(connection, confFile, ConfName, MinParties, status="Status OK"):
    
    print "Try to Add Conf " + ConfName + " Min Partiecepents set to " + str(MinParties) 
    SetAndSendFileMinParties(connection, confFile, ConfName, MinParties, status)
    confFound,confid = WaitUntillConfCreated(connection, ConfName)
    if confFound:
        print "Conf " + ConfName + " Min Partiecepent set to " + str(MinParties) + " created"
        connection.DeleteConf(confid)
    else:
       print "Error!! Conf " + ConfName + " Min Partiecepent set to " + str(MinParties) + " NOT created" 
    connection.WaitAllConfEnd(50)
    return

#------------------------------------------------------------------------------
def AddConfMinParticepentsTest(connection,confFile,numRetries):
    
    ConfName = "CpConf"

    # try to add conf with min 0 parties
    MinParties = 0
    AddConfMinParticepentsDefined(connection, confFile, ConfName+"1", MinParties)
     
    # try to add conf with min 20 parties
    MinParties = 20
    AddConfMinParticepentsDefined(connection, confFile, ConfName+"2", MinParties)
     
    # try to add conf with min 3000 parties
    MinParties = 3000
    sendstatus = "Reserved resources for video participants exceeds the maximum of video participants per conference. Additional connections of video participants are denied"
    AddConfMinParticepentsDefined(connection, confFile, ConfName+"3", MinParties, sendstatus)
     
    
#    print "Wait to see if conf createtd...",
#   confFound,confid = WaitUntillConfCreated(connection,confname)
#    print
#    if confFound:
#        print "Error!! Conf " + confname + " Min Partiecepent set to 21(OverLimit) created !!" 
#        connection.DeleteConf(confid)
#    else:
#       print "Limit Over!! Conf " + confname + " Min Partiecepent set to 21 NOT created (its OK)" 
           
#    MinParties = 30
#    sendstatus = "Insufficient resources"
#    AddConfMinParticepentsDefined(connection, confFile, ConfName, MinParties, sendstatus)

    
#    print "Wait to see if conf createtd...",
#    confFound,confid = WaitUntillConfCreated(connection,confname)
#    print
#    if confFound:
#       print "Error!! Conf " + confname + " Min Partiecepent set to 30(OverLimit) created !!" 
#       connection.DeleteConf(confid)
#    else:
#      print "Limit Over!! Conf " + confname + " Min Partiecepent set to 30 NOT created (its OK)" 

    connection.WaitAllConfEnd(50)

    return

###------------------------------------------------------------------------------
def WaitUntillConfCreated(connection,FindName):
    print "Wait untill Conf: "+ FindName +" create...",
    confId=""
    confName=""
    findConf=0
    for retry in range(3):
        connection.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        confList = connection.xmlResponse.getElementsByTagName("CONF_SUMMARY")
        listSize = len(confList)
        for i in range( listSize ):
            confName=confList[i].getElementsByTagName("NAME")[0].firstChild.data
            if (confName == FindName):
                confId=confList[i].getElementsByTagName("ID")[0].firstChild.data
                findConf=1
                break
                print
        if findConf:
            print
            break
        if (retry == 3):
            break
        sys.stdout.write(".")
        sys.stdout.flush()
    
    return findConf,confId


###------------------------------------------------------------------------------ 
def SetAndSendFileMinParties(connection,confFile,ConfName,min_parties,status="Status OK"):   
    connection.LoadXmlFile(confFile)
    connection.ModifyXml("RESERVATION","NAME",ConfName)
    connection.ModifyXml("MEET_ME_PER_CONF","ON","true")
    connection.ModifyXml("MEET_ME_PER_CONF","AUTO_ADD","true")
    connection.ModifyXml("MEET_ME_PER_CONF","MIN_NUM_OF_PARTIES",min_parties)
    connection.Send(status)
    return
##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

AddConfMinParticepentsTest(c,
                  'Scripts/ConfTamplates/AddConfTemplate.xml',
                  20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------
