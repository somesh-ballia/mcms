#!/mcms/python/bin/python


from McmsConnection import *



class AuditUtils (McmsConnection):

    def GetAuditBufferFileName(self):
        tmpFileName = "tmpOutputFile.yura"
        command = "ls -a Audit/ | grep .buffer > " + tmpFileName
        os.system(command)
        
        fileObject = open(tmpFileName)
        fileLine = fileObject.readline()
        fileObject.close()
        
        os.system("rm " + tmpFileName)

        lineSplit = fileLine.split("\n")
        result = "Audit/" + lineSplit[0]
        
        return result

        
    def GetAuditFileList(self, expectedStatus):
        self.SendXmlFile('Scripts/GetAuditFileList.xml', expectedStatus)
        return self.xmlResponse

    def GetAuditFile(self, fileName, destinationPath):
        return True

    def GetAuditEvents(self, expectedStatus):
        self.SendXmlFile('Scripts/GetAuditEventList.xml', expectedStatus)
        return self.xmlResponse

    def DoesEventExist(self, action, status):
        res = DoesEventExistInMemory(action, status)
        if(True == res):
            res = DoesEventExistInFile(action, status)
        return res
        
    def DoesEventExistInMemory(self, action, status, description = ""):
        xmlResponse = self.GetAuditEvents("Status OK")
#        print xmlResponse.toprettyxml(encoding="utf-8") 
        xmlNode = self.FindEventInXmlEventList(xmlResponse, action, status, description)
        if(None != xmlNode):
            return True
        return False

    def DoesEventExistInFile(self, xmlFileName, action, status, description = ""):
        xmlTree = parse(xmlFileName)
        xmlNode = self.FindEventInXmlEventList(xmlTree, action, status, description)
        if(None != xmlNode):
            return True
        return False
    
    def ValidateAuditFile(self):
        return True

    def GetNumAuditFiles(self):
        return 0

    
    def FindEventInXmlEventList(self, xmlTree, action, status, description):
        auditListSummary = xmlTree.getElementsByTagName("AUDIT_EVENT_LIST")[0]
        auditList = auditListSummary.getElementsByTagName("AUDIT_EVENT")
        for auditElem in auditList:
            actionElem = auditElem.getElementsByTagName("ACTION")[0]
            if(action == actionElem.firstChild.data):
                return actionElem
            
        return None
    
