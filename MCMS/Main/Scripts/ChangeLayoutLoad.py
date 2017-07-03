#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1


from McmsConnection import *
            
#------------------------------------------------------------------------------
def ChangeConfLayoutTypeTest(connection, confid, newConfLayout):
    print "Conference ID: "+ confid + " Changing Layout Type To: " + newConfLayout        
    connection.LoadXmlFile('Scripts/ConfVideoLayout/ChangConfLayout.xml')
    connection.ModifyXml("SET_VIDEO_LAYOUT","ID",confid)
    connection.ModifyXml("FORCE","LAYOUT",newConfLayout)       
    connection.Send()
    connection.WaitAllOngoingChangedLayoutType(confid,newConfLayout)
    return
    
#------------------------------------------------------------------------------

#Create CP conf with 3 parties
c = McmsConnection()
c.Connect()
c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',
                         'Scripts/AddVideoParty1.xml',
                         180,
                         60,
                         "false")

#Get ConfId
c.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
confid = c.GetTextUnder("CONF_SUMMARY","ID")
if confid == "":
   c.Disconnect()                
   sys.exit("Can not monitor conf:" + status)

#Changing conf layout type to each layout
for x in range(2):
    print "round " + str(x)
    for layout in availableLayoutTypes:
        ChangeConfLayoutTypeTest(c, confid,layout)
#        sleep(1)

#Delete Conf
sleep(10)
c.DeleteConf(confid)
sleep(10)
c.WaitAllConfEnd()

c.Disconnect()


