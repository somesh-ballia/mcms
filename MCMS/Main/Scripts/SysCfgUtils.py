#!/mcms/python/bin/python
import xml.dom.minidom
import os

from xml.dom import Node 
from McmsConnection import *



#------------------------------------------------------------------------------
def AddChildNode(doc, currentElement, childTagName, ChildText = ""):
    childElement = doc.createElement(childTagName)
    if ChildText <> "":
        childTextElement = doc.createTextNode(ChildText)
        childElement.appendChild(childTextElement)
    currentElement.appendChild(childElement)
    return childElement



#------------------------------------------------------------------------------
class SysCfgParam:
    m_Key = ""
    m_Data = ""
    m_Section = ""
    m_Type = ""

    def IsEqual(self, other):
        if self.m_Key <> other.m_Key:
            return False
        if self.m_Data <> other.m_Data:
            return False
        if self.m_Section <> other.m_Section:
             return False
        if self.m_Type <> other.m_Type:
            return False
        return True

    def Print(self, title = ""):
        print title + ": " + self.m_Section + " " + self.m_Key + " " + self.m_Data + " " + self.m_Type


#------------------------------------------------------------------------------
class SysCfgParamList:
    m_Name = ""
    m_SysCfgParamList = [] # list

    def __init__(self):
        self.m_SysCfgParamList = []
    
    def Append(self, newNode):
        self.m_SysCfgParamList.append(newNode)
    
    def AppendRange(self, newRange):
        for param in newRange.m_SysCfgParamList:
            self.Append(param)
        
    def Clean(self):
        while self.Length() <> 0:
            self.m_SysCfgParamList.pop()
    
    def IsContain(self, node):
        isFound = False
        for param in self.m_SysCfgParamList:
            isFound = param.IsEqual(node)
            if isFound == True:
                break
        return isFound

    def Print(self):
        i = 0
        for cfgParam in self.m_SysCfgParamList:
            i = i + 1
            cfgParam.Print(str(i))
    
    def Length(self):
        i = 0
        for cfgParam in self.m_SysCfgParamList:
            i = i + 1
        return i

    def GetCfgType(self):
        return self.m_SysCfgParamList[0].m_Type

    def GetSection(self):
        return self.m_SysCfgParamList[0].m_Section

    def SerializeXml(self, doc, rootElement):
        for cfgParam in self.m_SysCfgParamList:
            cfgPairElement = AddChildNode(doc, rootElement, "CFG_PAIR")
            AddChildNode(doc, cfgPairElement, "KEY", cfgParam.m_Key) 
            AddChildNode(doc, cfgPairElement, "DATA", cfgParam.m_Data)


    


#------------------------------------------------------------------------------
class SyscfgUtilities ( McmsConnection ):
    m_string = ""

    m_SysCfgParamList = SysCfgParamList()

#------------------------------------------------------------------------------    
    def __init__(self):
        self.m_SysCfgParamList = SysCfgParamList()

#------------------------------------------------------------------------------
    def SetParamList(self, otherSysCfgParamList):
        self.m_SysCfgParamList.Clean()
        self.m_SysCfgParamList.AppendRange(otherSysCfgParamList)

#------------------------------------------------------------------------------      
    def CreateCfgParam(self, section, key, data, type):
        cfgParam = SysCfgParam()
        cfgParam.m_Key = key
        cfgParam.m_Data = data
        cfgParam.m_Section = section
        cfgParam.m_Type = type
        return cfgParam

#------------------------------------------------------------------------------
    def AddCfgParam(self, section, key, data, type):
        cfgParam = self.CreateCfgParam(section, key, data, type)
        self.m_SysCfgParamList.Append(cfgParam)

#------------------------------------------------------------------------------
    def PrintCfgTable(self):
        self.m_SysCfgParamList.Print()
 
#------------------------------------------------------------------------------
    def Length(self):
        return self.m_SysCfgParamList.Length()
    
#------------------------------------------------------------------------------
    def Clean(self):
        self.m_SysCfgParamList.Clean()
        
#------------------------------------------------------------------------------
## SerializeXml should create tree like this; SET_CFG is an input root element
## 
##         <SET_CFG>
## 			<CFG_TYPE>user</CFG_TYPE>
## 			<SYSTEM_CFG>
## 				<CFG_SECTION>
## 					<NAME>MCMS_PARAMETERS_USER</NAME>
## 					<CFG_PAIR>
## 						<KEY>ALLOW_NON_ENCRYPT_PARTY_IN_ENCRYPT_CONF</KEY>
## 						<DATA>NO</DATA>
## 					</CFG_PAIR>
## 				</CFG_SECTION>	
## 			</SYSTEM_CFG>

## 	   </SET_CFG>

    
#------------------------------------------------------------------------------        
    def SerializeXml(self, doc, rootElement):
        AddChildNode(doc, rootElement, "CFG_TYPE", self.m_SysCfgParamList.GetCfgType())
        cfgElement = AddChildNode(doc, rootElement, "SYSTEM_CFG")        
        sectionElement = AddChildNode(doc, cfgElement, "CFG_SECTION")
        AddChildNode(doc, sectionElement, "NAME", self.m_SysCfgParamList.GetSection())

        self.m_SysCfgParamList.SerializeXml(doc, sectionElement)
        
        
          
        
#------------------------------------------------------------------------------
    def SendSetCfg(self, expectedStatus = "Status OK"):
        self.LoadXmlFile("Scripts/SetSysCfgPattern.xml")
        doc = self.loadedXml
        setCfgElementList = doc.getElementsByTagName("SET_CFG")
        if setCfgElementList.length <> 1:
            print "setCfgElementList.length <> 1"
            sys.exit(1)
            
        setCfgElement = setCfgElementList[0]
        self.SerializeXml(doc, setCfgElement)
        self.Send(expectedStatus)
        
#------------------------------------------------------------------------------
    def SendSetCfgParam(self,flag_key,data_val, expectedStatus = "Status OK"):
        preprint =  str(data_val)
        print "Send Setcfgparam flag "  + flag_key + "=" + preprint + " in system.cfg"
        self.LoadXmlFile('Scripts/SetSystemCfgParam.xml') 
        self.ModifyXml("SET_CFG_PARAM","NAME","MCMS_PARAMETERS_USER")
        self.ModifyXml("SET_CFG_PARAM","KEY",flag_key)
        self.ModifyXml("SET_CFG_PARAM","DATA",data_val)
        self.Send(expectedStatus)
    
#------------------------------------------------------------------------------
    def UpdateSyscfgFlag(self,section_name,flag_key,data_val,cfg_type ="debug"):
        preprint =  str(data_val)
        print "Send Set in section "+ section_name + ": "  + flag_key + "=" + preprint + " in system.cfg"
        self.LoadXmlFile('Scripts/SetSystemCfg.xml') 
        self.ModifyXml("SET_CFG","NAME",section_name)
        self.ModifyXml("SET_CFG","KEY",flag_key)
        self.ModifyXml("SET_CFG","DATA",data_val)
        self.ModifyXml("SET_CFG","CFG_TYPE",cfg_type)
        self.Send()
       
        #sys.exit()
        #return(0)
#------------------------------------------------------------------------------ 

#------------------------------------------------------------------------------
   # def UpdateSyscfgFlag(self,section_name,flag_key,data_val,cfg_type ="debug"):
   #     preprint =  str(data_val)
   #     print "Send Set in section "+ section_name + ": "  + flag_key + "=" + preprint + " in system.cfg"
   #     self.LoadXmlFile('Scripts/SetSystemCfg.xml') 
   #     self.ModifyXml("SET_CFG","NAME",section_name)
   #     self.ModifyXml("SET_CFG","KEY",flag_key)
   #     self.ModifyXml("SET_CFG","DATA",data_val)
   #     self.ModifyXml("SET_CFG","CFG_TYPE",cfg_type)
        #self.Send()
   #     open('thefile.xml','a').write(self.loadedXml.toprettyxml())
        #sys.exit()
        #return(0)
#------------------------------------------------------------------------------
        
   # def SendSyscfgFlags(self,DebugUser = "User"): 
   # 	#self.Upload(open('thefile.xml','r').read(),"/Cfg/SystemCfg"+DebugUser+".xml")
   #     self.Upload(m_string,"/Cfg/SystemCfg"+DebugUser+".xml")
        #self.send('thefile.xml')
   #     sys.exit()
   #     return(0)
    
          
#--------------------------------------------------------------------------------          
    def CheckSyscfgFlag(self,section_name,flag_key,data_val):
        print "get the flag from system.cfg ..."
        self.SendXmlFile("Scripts/GetSystemCfg.xml")
        cfg_elm_list1 = self.xmlResponse.getElementsByTagName("RESPONSE_TRANS_CFG")
        x =len(cfg_elm_list1)
        for index in x:
		cfg_desc1 =self.xmlResponse.getElementsByTagName("CFG_PAIR")[0].getElementsByTagName("KEY")[0].firstChild.data
		if cfg_desc1 == flag_key : 
		   cfg_data = c.xmlResponse.getElementsByTagName("CFG_PAIR")[0].getElementsByTagName("DATA")[0].firstChild.data 
		   if cfg_data == data_val:
		      print cfg_desc+"= " + cfg_data + " found system.cfg" 
#              return (1) 
        #sys.exit()
        return(0) 
    
#------------------------------------------------------------------------------           
    def SetFederalMode(self, jitc_mode, force_pwd_policy, clearCfg=True):
        print "Setting System.cfg Federal Mode..."
        if(clearCfg):
            self.Clean()
        self.AddCfgParam("MCMS_PARAMETERS_USER", "JITC_MODE", jitc_mode, "user")
        self.AddCfgParam("MCMS_PARAMETERS_USER", "FORCE_STRONG_PASSWORD_POLICY", force_pwd_policy, "user")
        self.AddCfgParam("MCMS_PARAMETERS_USER", "USER_LOCKOUT_WINDOW_IN_MINUTES", "60", "user")
        self.AddCfgParam("MCMS_PARAMETERS_USER", "USER_LOCKOUT_DURATION_IN_MINUTES", "60", "user")
        self.AddCfgParam("MCMS_PARAMETERS_USER", "PASSWORD_HISTORY_SIZE", "16", "user")
    
        self.SendSetCfg()
    
        self.Sleep(2)
    
        os.environ["CLEAN_CFG"]="NO"

        os.system("Scripts/Destroy.sh")
        
        if("YES" == jitc_mode):
            os.system("Scripts/StartupWithFederal.sh")
        else:
            os.system("echo NO > /mcms/JITC_MODE.txt")
            os.system("rm /mcms/Cfg/EncOperatorDB.xml")
            os.system("Scripts/Startup.sh")
    
        self.Sleep(5)
        
        
        
        
