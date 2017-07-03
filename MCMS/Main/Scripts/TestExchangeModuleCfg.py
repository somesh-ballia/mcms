#!/mcms/python/bin/python

# #############################################################################
# Change configuration of Exchange Module
#
# Date: 23/08/09
# By  : Vasily  
#
############################################################################
#
#

from McmsConnection import *

expected_status="In progress;Status OK"

def SetCfg(isEnabled,url,user,password,domain,isDelegate,mailbox):
    c = McmsConnection()
    c.Connect()

    c.LoadXmlFile('Scripts/ExchangeModuleScripts/SetMcuExchangeConfigParams.xml')

    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","SERVICE_ENABLED",isEnabled)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","WEB_SERVICES_URL",url)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","USER_NAME",user)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","PASSWORD",password)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","DOMAIN_NAME",domain)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","WEB_SERVICES_DELEGATE",isDelegate)
    c.ModifyXml("MCU_EXCHANGE_CONFIG_PARAMS","PRIMARY_SMTP_MAILBOX",mailbox)
    c.Send(expected_status)
    print c.xmlResponse.toprettyxml()
    
    c.LoadXmlFile('Scripts/ExchangeModuleScripts/GetMcuExchangeConfigLastInidcation.xml')
    c.Send(expected_status)
    print "Test Finished successfull , Exchanage Configured!!!"  
    c.Disconnect()


def TestSetCfg():
    print
    print "Start ChangeExchangeModuleConfiguration test"
    print

    print "set enabled = true "
    SetCfg("true","https://10.226.249.11/EWS/Exchange.asmx","nighttest","Polycom123","miglab","true","")
    print "End of ChangeExchangeModuleConfiguration test"
    print

TestSetCfg()

