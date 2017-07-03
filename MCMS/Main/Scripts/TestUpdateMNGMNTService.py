#!/mcms/python/bin/python


# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_11

import os

from UsersUtils import *



#------------------------------------------------------------------------------       
#    local function, it compares XML trees.
#    if there is at least one node that is not equals to its twin node at the second tree
#    the function returns false. Otherwise it returns true. the order of nodes is important.
#    the function is recursive.
#------------------------------------------------------------------------------
    
def TestXmlTreeEqualityRecursive(leftNode, rightNode):
    
    # ------ Check current node
    if(leftNode.hasChildNodes() <> rightNode.hasChildNodes()):
        print "The number of childs is not the same" 
        return False
    elif(True == leftNode.hasChildNodes()):      
        if(leftNode.tagName <> rightNode.tagName):
            print "Tags are not the same : " + str(leftNode.tagName) + " - " + str(rightNode.tagName) 
            return False
        
    if(leftNode.nodeType <> rightNode.nodeType):
        print "Types are not the same : " + str(leftNode.nodeType) + " - " + str(rightNode.nodeType)
        return False
    
    # every node is followed be a strange node that has '\n' and '\t' in data. 
    firstChar = str(leftNode.nodeValue)[0]
    if(firstChar <> '\n'):
        if(leftNode.nodeValue <> rightNode.nodeValue):
            print "Values are not the same : " + str(leftNode.nodeValue) + " - " + str(rightNode.nodeValue)
            print "left node : " + str(leftNode)
            print "right node : " + str(rightNode)
            return False
        
        
    # ------ Check childs
    lelfChildNodes = leftNode.childNodes 
    rightChildNodes = rightNode.childNodes
    
    if(lelfChildNodes.length <> rightChildNodes.length):
        print "Number of childs is not the same : " + str(lelfChildNodes.length) + " - " + str(rightChildNodes.length)
        return False
    
    for lelfChild, rightChild in zip(lelfChildNodes, rightChildNodes):
        if(False == TestXmlTreeEqualityRecursive(lelfChild, rightChild)):
            print "One of childs is not equal"
            return False
        
    # Current Node(and all his childs of cource) is Equal
    return True






#------------------------------------------------------------------------------       
#    local function, it compares parts of XML documents.
#    it compares the first element which is found
#------------------------------------------------------------------------------
    
def CompareXMLDocuments(leftDocument, rightDocument, tagName):
    
    leftXmlTree = leftDocument.getElementsByTagName(tagName)
    rightXmlTree = rightDocument.getElementsByTagName(tagName)
    
    if(leftXmlTree.length <> rightXmlTree.length):
        print "Number of appearances of the element is not the same  : " + tagName
        return False

    isEqual = True
    if(0 < leftXmlTree.length):
        isEqual = TestXmlTreeEqualityRecursive(leftXmlTree[0], rightXmlTree[0])

    return isEqual






     
#------------------------------------------------------------------------------       
#    Test
#------------------------------------------------------------------------------


    
#------ 1. Update MNGMNT service

userUtilsUpdate = UsersUtilities()
userUtilsCfg = UsersUtilities()

userUtilsUpdate.ConnectSpecificUser("SUPPORT","SUPPORT")
userUtilsUpdate.SendXmlFile("Scripts/UpdateNetworkService.xml", "Status OK")





#------ 2. Load the updated MNGMNT service and the transaction

userUtilsCfg.LoadXmlFile("Cfg/NetworkCfg_Management.xml")
userUtilsUpdate.LoadXmlFile("Scripts/UpdateNetworkService.xml")





#------ 3. Check the Equality of the update and transaction services

cfgRootElem = userUtilsCfg.loadedXml.documentElement
updateRootElem = userUtilsUpdate.loadedXml.documentElement

isEqual = CompareXMLDocuments(cfgRootElem, updateRootElem, "IP_SERVICE")
if(False == isEqual):
    sys.exit("Services are not equal, should be equal")

print "Services are equal, it was expected"



#------ 4. Check the TestXmlTreeEqualityRecursive, this time it should return false.

ipServiceListUpdate = updateRootElem.getElementsByTagName("NAME")
ipServiceListUpdate[0].firstChild.data = "cucu_lulu"

isEqual = CompareXMLDocuments(cfgRootElem, updateRootElem, "IP_SERVICE")
if(True == isEqual):
    sys.exit("Services are equal, should be different")

print "Services are not equal, it was expected"

print "Equality successed"

userUtilsUpdate.Disconnect()
