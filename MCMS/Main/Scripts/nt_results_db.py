#!/mcms/python/bin/python

import os, string
import sys
import xml.dom.minidom
import subprocess
from sets import Set
from xml.dom.minidom import  parse, parseString,Document

#helper functions
def AddTestsToElement(AllSet,domElem, filename):
    fp = open(filename)
    for line in fp:
            test  = doc.createElement('Test')
            test.setAttribute("Id"  , line.strip())
            domElem.appendChild(test) 
            AllSet.add(line.strip())   
    fp.close()

def AddTestsToAll(AllSet,allElem):
    for test_id in AllSet:
        test  = doc.createElement('Test')
        test.setAttribute("Id"  , test_id)
        allElem.appendChild(test)
         
def CreateXmlFile():
    
    # expecting first argument to be revision number
    revision_number = sys.argv[2]
    print "revision number : " + revision_number
    
    print "-----------------prepare  xml file ----------------------------------------------------------------------"
    
    
    # create base element
    base = doc.createElement('NightTestResults')
    doc.appendChild(base)
    # create an entry element
    rev = doc.createElement('Revision')
    rev_content = doc.createTextNode(revision_number)
    rev.appendChild(rev_content)
    base.appendChild(rev)
    
    mngmt     = doc.createElement('Management')
    #framework = doc.createElement('Framework')
    CallControl =  doc.createElement('CallControl')
    Conference  = doc.createElement('Conference')
    system      = doc.createElement('System')
    allElem         = doc.createElement('All')
    all      = Set([])
    print "-----------------prepare  build db file ----------------------------------------------------------------------"
    AddTestsToElement(all, mngmt ,'/mcms/TestResults/mngnt_scripts.txt'  )
    AddTestsToElement(all, Conference ,'/mcms/TestResults/conf_scripts.txt' )
    AddTestsToElement(all, CallControl ,'/mcms/TestResults/call_control_scripts.txt'  )
    #AddTestsToElement(all, framework ,'/mcms/TestResults/framework_scripts.txt'  )
    AddTestsToElement(all, system ,'/mcms/TestResults/system_scripts.txt'  )
    AddTestsToAll(all,allElem)
    
    base.appendChild(mngmt)
    base.appendChild(Conference)
    base.appendChild(CallControl)
    #base.appendChild(framework)
    base.appendChild(system)
    base.appendChild(allElem)
    print "prepare folders and write to file "
    os.system("mkdir -p ~/nighttest/Main/DB/")
    os.system("mkdir -p ~/nighttest/FSN-500/DB/")
    os.system("mkdir -p /mcms/NightTest/")
    f =  open("/mcms/NightTest/night_test_results.xml", "w+")
    doc.writexml(f)
    f.close()
    
def IsTestInGroup(test_id , group):
    test_list= doc.getElementsByTagName(group)[0].getElementsByTagName('Test')
    print "test list range :" + str(len(test_list))
    for index in range(len(test_list)):
        test_name= test_list[index] 
        if  test_id == test_name.attributes['Id'].value:
            print "found test " + test_id
            return 1
    return 0

def GetTestId():
    test_id = sys.argv[2]
    startIndex = test_id.find('/')
    startIndex = startIndex + 1 
    endIndex   = test_id.find('.')
    test_id    = test_id[startIndex:endIndex]
    return test_id
  

def AddResultStatusToTest(elemResult,data,group,valgrind_process):
       
    test_id = GetTestId()
    
    print "test_id is " + test_id        
    test_list= doc.getElementsByTagName(group)[0].getElementsByTagName('Test')
    print "test list range :" + str(len(test_list))
    for index in range(len(test_list)):
        test_name= test_list[index] 
        if  test_id == test_name.attributes['Id'].value:
            print "found test " + test_id
            if valgrind_process =="":
                status_content = doc.createTextNode(data)
                elemResult.appendChild(status_content)
                test_name.appendChild(elemResult)
            else:
                print "add valgrind result"
                elemValResult = doc.createElement('Result')
                elemValProcess = doc.createElement('ValgrindProcess')
                status_content = doc.createTextNode(data)
                process_content = doc.createTextNode(valgrind_process)
                elemValResult.appendChild(status_content)
                elemValProcess.appendChild(process_content)
                elemResult.appendChild(elemValResult)
                elemResult.appendChild(elemValProcess)
                test_name.appendChild(elemResult)        
  


def ConvertStatusCodeToString(statusCode , statusStr):
    print "ConvertStatusCodeToString - code " + statusCode
    if statusCode == '0':
         statusStr="PASSED"
    elif statusCode == '1233':
         statusStr="FAILED (process kill failed)"
    elif statusCode == '1234':
        statusStr="FAILED (exception traces found)"
    elif statusCode == '1235':
        statusStr="FAILED (EXPECTED_ASSERTS Could not be found)"
    elif statusCode == '10':
        statusStr="NO ERRORS"
    elif statusCode == '13':
        statusStr="MEMORY ERRORS and also LEAKs were found"
    elif statusCode == '12':
        statusStr="MEMORY ERRORS"
    elif statusCode == '11':
        statusStr="MEMORY LEAKs"
    else:
        statusStr="FAILED (script found an error)"
        
    print "ConvertStatusCodeToString - statusStr " + statusStr 
    return     statusStr


def updateDB(action,group):
    
    data=sys.argv[3]
    statusStr= "InvalidCode"    
    if action =="AddTestStatus":
        elemResult = doc.createElement('Result')
        print "check if need to add add last good revision " +data
        if data == '0':       
            rev_num=  doc.getElementsByTagName("Revision")[0].firstChild.data
            lastGoodElem= doc.createElement('LastPassedBaseLine')
            AddResultStatusToTest(lastGoodElem,rev_num,group,"")
        statusStr = ConvertStatusCodeToString(data,statusStr)
        AddResultStatusToTest(elemResult,statusStr,group,"")
        
        
    if action=="AddValgrindStatus":
        valgrind_process = sys.argv[5]
        print "valgrind process " + valgrind_process
        elemResult = doc.createElement('ValgrindResult')
        statusStr =ConvertStatusCodeToString(data,statusStr)
        AddResultStatusToTest(elemResult,statusStr,group,valgrind_process)
        
    if action=="AddValgrindError":            
        elemResult = doc.createElement('ValgrindError')
        statusStr =ConvertStatusCodeToString(data,statusStr)
        AddResultStatusToTest(elemResult,statusStr,group,"")
    
    if action=="AddCoreDump":
        elemResult = doc.createElement('CoreDump')
        AddResultStatusToTest(elemResult,data,group,"")    



action = sys.argv[1]
doc = Document()

if action =="init":
    CreateXmlFile()
    sys.exit(0)
    
valgrind_process =""    
doc= parse('/mcms/NightTest/night_test_results.xml')
updateDB(action,'All')

test_Id =GetTestId()
    
if IsTestInGroup(test_Id ,'Management'):
    updateDB(action,'Management')

#if IsTestInGroup(test_Id ,'Framework'):
#    updateDB(action,'Framework')

if IsTestInGroup(test_Id ,'System'):
    updateDB(action,'System')

if IsTestInGroup(test_Id ,'CallControl'):
    updateDB(action,'CallControl')

if IsTestInGroup(test_Id ,'Conference'):
    updateDB(action,'Conference')

print "write to file "
f =  open("/mcms/NightTest/night_test_results.xml", "w+")
doc.writexml(f)
f.close()