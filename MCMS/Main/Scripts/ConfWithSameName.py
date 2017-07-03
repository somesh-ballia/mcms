#!/mcms/python/bin/python

#############################################################################
# Eror handling Script which Creating Conf with a name of existing conf 
#
# Date: 17/01/05
# By  : Ron S.

#############################################################################

from McmsConnection import *

###------------------------------------------------------------------------------
def SimpleConfWithExistConfName(connection,confFile,numRetries):
    
    confname = "ConfWithTheSameName"
    connection.CreateConf(confname, confFile)
    confid = connection.WaitConfCreated(confname, numRetries)

# trying to add second conf with the same name as the first conf
    print "Adding Conf With existing name: " + confname + "  ..."
    connection.CreateConf(confname, confFile)

    print "Wait untill Conf:" + confname + " will be createtd...",
    for retry in range(numRetries+1):
        connection.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        confid2 = connection.GetTextUnder("CONF_SUMMARY","ID")
#        if confid2 != "":
        conf_list = connection.xmlResponse.getElementsByTagName("CONF_SUMMARY")
        if len(conf_list) == 2 :
           print 
           print "Create conf with id " + str(confid2)
           print "A Conf with already existing name was created: " + confname
           connection.Disconnect()
           sys.exit("exit due to: Conf with already existing name was created")
           break

        if (retry == numRetries):
            print
            print connection.xmlResponse.toprettyxml()
            print "The Conf:" + confname + " was not created because that a Conf with the same name is already exist" 
#           connection.Disconnect()                
#           sys.exit("Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)
             
        

    #print "Delete Conference..."
    connection.DeleteConf(confid)
         
    #print "Wait until no conferences"
    connection.WaitAllConfEnd(50)
    
    return

##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

SimpleConfWithExistConfName(c,
                            'Scripts/ConfWithSameName/AddVoipConf.xml',
                            20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------
