#!/mcms/python/bin/python

from ResourceUtilities import *

#########################################################################
def SimpleNonTerminateXConfWithYParticipaints(connection,numOfConfs,numOfParties,
                                  confFile,
                                  partyFile,
                                  numRetries):
    confIdArray = [0]*numOfConfs
    for confNum in range(numOfConfs):
        confname = "Conf"+str(confNum+1)
        connection.LoadXmlFile(confFile)
        connection.ModifyXml("RESERVATION","NAME",confname)
        print "Adding Conf: " + confname + "  ..."
        connection.Send()

        print "Wait untill Conf number " + str(confNum+1) + " will be createtd...",
        for retry in range(numRetries+1):
            connection.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
            confid = connection.GetTextUnder("CONF_SUMMARY","ID")
            conf_list = connection.xmlResponse.getElementsByTagName("CONF_SUMMARY")
            if len(conf_list) == (confNum +1) :
                confIdArray[confNum] = conf_list[confNum].getElementsByTagName("ID")[0].firstChild.data
                print
                break
            if (retry == numRetries):
                print connection.xmlResponse.toprettyxml()
                connection.Disconnect()                
                sys.exit("Can not monitor conf:" + status)
            sys.stdout.write(".")
            sys.stdout.flush()

        print "Create conf with id " + str(confIdArray[confNum]) 
        
        connection.LoadXmlFile(partyFile)        
        for x in range(numOfParties):
            partyname = "Party" + str(x+1) ##+ str(confIdArray[confNum])
            partyip =  "1.2.3." + str(x+1)+str(confIdArray[confNum])
            print "Adding Party " + partyname + ", with ip= " + partyip
            connection.ModifyXml("PARTY","NAME",partyname)
            connection.ModifyXml("PARTY","IP",partyip)
            connection.ModifyXml("ADD_PARTY","ID",confIdArray[confNum])
            connection.Send()
            
        connection.WaitAllPartiesWereAdded(confIdArray[confNum],numOfParties,numRetries)
        connection.WaitAllOngoingConnected(confIdArray[confNum],numRetries)

    ############# for our use here no need to delete !!! ###########
    #for deletedConfNum in range(numOfConfs):
        #print "Delete Conference..."
     #   connection.DeleteConf(confIdArray[deletedConfNum])
         
    #print "Wait until no conferences"
    #connection.WaitAllConfEnd(50)
    
    return

#############################################################################################

r = ResourceUtilities()
r.Connect()

r.SendXmlFile("Scripts/ResourceFullCapacity/ResourceMonitorCarmel.xml")
num_of_resource = r.GetTotalCarmelParties()


print "RSRC_REPORT_RMX[Total] = " + str(num_of_resource)

r.TestFreeCarmelParties(num_of_resource)

##### split to several conferences with parties

num_conf = 1
num_parties = num_of_resource

for factor in range(int(num_of_resource)):
    
   num = int ( int(num_of_resource) % int(factor+2) )
   
   if num == 0:  
         num_conf = factor+2
         num_parties = ( int(num_of_resource) / int(factor+2) )
         break


SimpleNonTerminateXConfWithYParticipaints(r,int(num_conf),int(num_parties),
                               'Scripts/ResourceFullCapacity/AddVideoCpConf.xml',
                               'Scripts/ResourceFullCapacity/AddVideoParty1.xml',
                               20)

r.TestFreeCarmelParties( 0 )

r.DeleteAllConf()
r.WaitAllConfEnd(100)


r.TestFreeCarmelParties(int(num_of_resource))

r.Disconnect()

########################################################################################


