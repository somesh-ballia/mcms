#!/mcms/python/bin/python

#############################################################################
# A Script which Create a Conf with 3 launched participants
#
# Date: 17/01/05
# By  : Ron S.

#############################################################################

from McmsConnection import *

###------------------------------------------------------------------------------

def SimpleXmlConfWith3PartyTest(connection,confFile,numRetries):

    confname = "Conf1"
    connection.CreateConf(confname, confFile)
    
    confid = connection.WaitConfCreated(confname,numRetries)
    print "Create conf with id " + str(confid)
    sleep(1)
    
    connection.WaitAllPartiesWereAdded(confid,3,numRetries)
    connection.WaitAllOngoingConnected(confid,numRetries)
    
    #print "Delete Conference..."
    connection.DeleteConf(confid)
    #print "Wait until no conferences"
    connection.WaitAllConfEnd(50)
    
    return


##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()
print "Starting test: creating conf with 3 Blast participants ..."
SimpleXmlConfWith3PartyTest(c,
                            'Scripts/BlastDialOut3Parties/AddConfBlast3Parties.xml',                            
                             20)
c.Disconnect()
