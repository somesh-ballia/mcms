#! /usr/bin/env python
# Send an HTML email with an embedded image and a plain text message for
# email clients that don't want to display the HTML.
#
#sys.argv[1] is the email adress to send the report to


from time import gmtime, strftime, localtime
import smtplib
import sys
import tarfile
import HTML
from datetime import date
from datetime import datetime

from email.MIMEMultipart import MIMEMultipart
from email.MIMEText import MIMEText
from email.MIMEImage import MIMEImage
from email.MIMEBase import MIMEBase
from email import Encoders

import xml.dom.minidom

from xml.dom.minidom import  parse, parseString,Document

import os
import os.path

from socket import gethostname;
###------------------------------------------------------------------------------
            
def GetDayName(day):
    if(day == 0):
        return 'Monday' 
    if(day == 1):
        return 'Tuesday' 
    if(day == 2):
        return 'Wednesday' 
    if(day == 3):
        return 'Thursday' 
    if(day == 4):
        return 'Friday' 
    if(day == 5):
        return 'Saturday' 
    if(day == 6):
        return 'Sunday' 

def GetRefTag(_line):

	if (_line.find('owners for failed scripts') != -1):
	    return 'owners' 
	
	if (_line.find('Test Results Summary:') != -1):
	    return 'Summary'  

	if (_line.find('Functionality only - no valgrind') != -1):
	    return 'FunctionalityOnly'  

	if (_line.find('Failed Scripts when running under valgrind') != -1):
	    return 'FailedValgrind'  

#	if (_line.find('Exception List') != -1):
#	    refText = refText + 'Test Results Summary<br>'
#	    return 'ExceptionList'  

	if (_line.find('Core Dumps Created') != -1):
	    return 'CoreDumps'  

	if (_line.find('Valgrind Core Dumps Created') != -1):
	    return 'CoreDumpsValgrind'  

	if (_line.find('Log Analyze Table') != -1):
	    return 'LogAnalyze'  

        if (_line.find('Scripts Run Time') != -1):
            return 'ScriptsRunningTime'

	return ""


def GetRefLink(lineToAdd, refTag):
	lineToAdd = "<a href=\"#" + refTag + "\">" + lineToAdd + "</a>" 
#	lineToAdd = lineToAdd.replace(refTag, refTag)
	return lineToAdd

def GetLastPassBaselineDateDiff(test_id,group):
    LaststrDate = "1/1/1900"
    todayDate = datetime.now()
    test_list= MainDoc.getElementsByTagName(group)[0].getElementsByTagName('Test')    
    for index in range(len(test_list)):
        test_name= test_list[index] 
        if  test_id == test_name.attributes['Id'].value:           
            LastPassedDateElem=  test_name.getElementsByTagName('LastPassedDate')
            if len(LastPassedDateElem) > 0:
                    LaststrDate= LastPassedDateElem[0].firstChild.data
            else:
                    LaststrDate= strftime("%d/%m/%Y", gmtime())
                    lastGoodDateElem= doc.createElement('LastPassedDate')
                    date_content = doc.createTextNode(LaststrDate)
                    lastGoodDateElem.appendChild(date_content)
                    test_name.appendChild(lastGoodDateElem)      
     
    if  LaststrDate != "1/1/1900":
        lastDatetime = datetime.strptime(LaststrDate,"%d/%m/%Y")
        diff = todayDate - lastDatetime
        print "difference in days " + str(diff.days +1)
        return diff.days + 1
    else:
        print "error  LaststrDate was not found : test_id  " + test_id + " group" + group
        return 0           
    

def GetLastPassBaseline(test_id,group):
    
    baseline="N/A"
    test_list= MainDoc.getElementsByTagName(group)[0].getElementsByTagName('Test')    
    for index in range(len(test_list)):
        test_name= test_list[index] 
        if  test_id == test_name.attributes['Id'].value:
            baselineElem=  test_name.getElementsByTagName('LastPassedBaseLine')
            if len(baselineElem) > 0:
                    baseline= baselineElem[0].firstChild.data
            else:
                    baseline="N/A"
    return   baseline

def UpdateLastPassBaseline(test_id,group):
    test_list= MainDoc.getElementsByTagName(group)[0].getElementsByTagName('Test')    
    for index in range(len(test_list)):
        test_name= test_list[index] 
        if  test_id == test_name.attributes['Id'].value:
            baselineElem=  test_name.getElementsByTagName('LastPassedBaseLine')        
            rev_num=  doc.getElementsByTagName("Revision")[0].firstChild.data
            print "update the last good baseline"       
            if len(baselineElem) > 0:
                    baselineElem[0].firstChild.data = rev_num
            else:
                lastGoodElem= doc.createElement('LastPassedBaseLine')
                baseline_content = doc.createTextNode(rev_num)
                lastGoodElem.appendChild(baseline_content)
                test_name.appendChild(lastGoodElem)    
          #update the last good date
            print "update the last good date"
            LastPassedDateElem=  test_name.getElementsByTagName('LastPassedDate')
            if len(LastPassedDateElem) > 0:
                    LastPassedDateElem[0].firstChild.data = strftime("%d/%m/%Y", gmtime())
            else:
                lastGoodDateElem= doc.createElement('LastPassedDate')
                date_content = doc.createTextNode(strftime("%d/%m/%Y", gmtime()))
                lastGoodDateElem.appendChild(date_content)
                test_name.appendChild(lastGoodDateElem)      

def GenerateHTMLForFailedScripts(group):
    countFailedScripts=0
    table = HTML.Table(header_row=['Test Name ', 'Failure Description ','Last passed baseline','Days open'])
    #table = HTML.Table(header_row=['Test Name ', 'Failure Description '])
    table.width=700
    test_list= doc.getElementsByTagName(group)[0].getElementsByTagName('Test')    
    for index in range(len(test_list)):
        test= test_list[index]
        if  test.childNodes:                 
            status= test.getElementsByTagName("Result")[0].firstChild.data
            if status != "PASSED":
                countFailedScripts = countFailedScripts +1
                testName = test.attributes['Id'].value
                days = GetLastPassBaselineDateDiff(testName,group)
                lastBase = GetLastPassBaseline(testName,group)
                table.rows.append([testName,status,lastBase,days])
            else:
                UpdateLastPassBaseline(test.attributes['Id'].value,group)
                     
    htmlcode =""   
    if countFailedScripts > 0:
        htmlcode=  str(table)
    else:
        htmlcode=" <br><span style=color:green>^ SUCCESS! clean run, no failed tests!</span><br>"
       
    return  htmlcode

def GenerateHTMLForValgrindFailedScripts(group):
    countFailedScripts=0
    table = HTML.Table(header_row=['Test Name under Valgrind ', 'Failure Description ', 'Process under Valgrind'])
    table.width=700
    test_list= doc.getElementsByTagName(group)[0].getElementsByTagName('Test')    
    for index in range(len(test_list)):
        test= test_list[index]
        if  test.childNodes:
            valgrind_list = test.getElementsByTagName("ValgrindResult")
            for i in range(len(valgrind_list)):
                test_val= valgrind_list[i]
                status= test_val.getElementsByTagName("Result")[0].firstChild.data
                if status != "PASSED":
                    countFailedScripts = countFailedScripts +1
                    testName = test.attributes['Id'].value
                    processName= test_val.getElementsByTagName("ValgrindProcess")[0].firstChild.data
                    table.rows.append([testName,status,processName])
                             
    htmlcode =""
    if countFailedScripts > 0:
        htmlcode=  str(table)
    else:
        htmlcode=" <br><span style=color:green>^  no failed tests under valgrind !</span><br>"
    return  htmlcode

def GenerateHTMLForValgrindErrors(group):
    countFailedScripts=0
    table = HTML.Table(header_row=['Test Name ', 'Error Description '])
    table.width=700
    
    test_list= doc.getElementsByTagName(group)[0].getElementsByTagName('Test')    
    for index in range(len(test_list)):
        try:
            test= test_list[index]
            if  test.childNodes:                                 
                valgrindElem= test.getElementsByTagName("ValgrindError")                
                if len(valgrindElem) > 0:
                    status= valgrindElem[0].firstChild.data
                    if status != "NO ERRORS":
                        countFailedScripts = countFailedScripts +1
                        testName = test.attributes['Id'].value
                        table.rows.append([testName,status])
        except IndexError:     
            print "GenerateHTMLForValgrindErrors : index error in test " + test.attributes['Id'].value
    htmlcode =""
    if countFailedScripts > 0:
        htmlcode=  str(table)
    else:
        htmlcode=" <br><span style=color:green>^ SUCCESS! no errors were found!</span><br>"
       
    return  htmlcode

def GenerateHTMLForCoreDumps(group):
     countCores=0
     html_list = HTML.List()
     test_list= doc.getElementsByTagName(group)[0].getElementsByTagName('Test')    
     for index in range(len(test_list)):
        test= test_list[index]
        if  test.childNodes:                 
            coredumpElem= test.getElementsByTagName("CoreDump")
            if len(coredumpElem) > 0:
                corename = coredumpElem[0].firstChild.data
                html_list.lines.append(corename)
                countCores = countCores +1
                
     if countCores > 0:
        htmlcode=  str(html_list)
     else:
        htmlcode=" <br><span style=color:green> no core files were found!</span><br>"
     return htmlcode
 
def GenerateHTMLForStatistics(group,isvalgrind):
    countFailed=0
    countPassed=0
    numberTests=0
    test_list= doc.getElementsByTagName(group)[0].getElementsByTagName('Test')
    numberTests =   len(test_list)  
    for index in range(len(test_list)):
        test= test_list[index]
        if  test.childNodes:
            if isvalgrind == 1:            
                valgrind_list = test.getElementsByTagName("ValgrindResult")
                for i in range(len(valgrind_list)):
                    test_val= valgrind_list[i]
                    status= test_val.getElementsByTagName("Result")[0].firstChild.data
                    if status != "PASSED":
                        countFailed = countFailed +1 
                    else:
                        countPassed = countPassed +1
            else:
                status= test.getElementsByTagName("Result")[0].firstChild.data
                if status != "PASSED":
                     countFailed = countFailed +1 
                else:
                      countPassed = countPassed +1
    
    htmlcode ="<br> Number of Scripts " + str(numberTests) +" Number of pass tests " + str(countPassed) + " Number of failed tests "+ str(countFailed)
    #table = HTML.Table(header_row=['Tests Number ', 'Passed Tests','Failed Tests'])
    #table.width=700
    #table.rows.append([numberTests,countPassed,countFailed])
    #htmlcode=  str(table)        
    return htmlcode
             
def GenerateReport(GroupName,textbody):
    textbody = textbody + "<br><hr color=blue /><b>Tests for Group " + GroupName + "</b><br><span style=color:blue># Night Test Failed Scripts </span><br>"
    
    
    textbody = textbody + GenerateHTMLForFailedScripts(GroupName) + "<br> "+ GenerateHTMLForStatistics(GroupName,0)
    
    textbody = textbody + "<br><hr color=blue /><br><span style=color:blue># Valgrind Errors </span><br>"
    
    textbody = textbody + GenerateHTMLForValgrindErrors(GroupName)
    
    textbody = textbody + "<br><hr color=blue /><br><span style=color:blue># Failed Scripts run under Valgrind </span><br>"
    
    textbody = textbody + GenerateHTMLForValgrindFailedScripts(GroupName) + "<br> "+ GenerateHTMLForStatistics(GroupName,1)
    
    
    textbody = textbody + "<br><hr color=blue /><br><span style=color:blue># Check for Core files  </span><br>"
    
    textbody = textbody + GenerateHTMLForCoreDumps(GroupName)
    return textbody
       
# Define these once; use them twice!
strFrom = 'CarmelNightest@polycom.co.il'
strTo = sys.argv[1]
#now = date.today()

outputFolderPath=sys.argv[2]
daydir=sys.argv[3] #added
cc_revision_name = sys.argv[4]

version_name = sys.argv[5]

GroupName = sys.argv[6]

print "Output dir is: " + outputFolderPath
print "designated mail is: "+ strTo
print "Test of Group :"  + GroupName;
print "Version Name: " + version_name
#logs_link = mail_outputdir+"/"+GetDayName( now.weekday())
logs_link = outputFolderPath + "/"+ daydir

# convert path to windows format
hostname = gethostname()
#hostname = hostname[:9] #lnx-vm-XX
if (logs_link.find('misc/nethome') != -1):
    logs_link = logs_link.replace('misc/nethome', hostname)
else:
    logs_link = logs_link.replace('nethome', hostname)
logs_link = logs_link.replace('/', '\\')
logs_link = "\\" + logs_link

mailSubject = "Night Test Results " + strftime("%d/%m/%Y", gmtime()) + " " +  version_name + " Revision: " + cc_revision_name 

print "subject:" + mailSubject

msgRoot = MIMEMultipart('related')
msgRoot['Subject'] = mailSubject
msgRoot['From'] = strFrom
msgRoot['To'] = strTo
msgRoot.preamble = 'This is a multi-part message in MIME format.'

# Encapsulate the plain and HTML versions of the message body in an
# 'alternative' part, so message agents can decide which they want to display.
msgAlternative = MIMEMultipart('alternative')
msgRoot.attach(msgAlternative)

msgText = MIMEText('This is the alternative plain text message.')
msgAlternative.attach(msgText)


# We reference the image in the IMG SRC attribute by the ID we give it below
textbody = '<html><head><title>1234</title></head><body><pre><b>'
textbody = textbody + "Reporting machine : "+gethostname()+' <br>'
textbody = textbody + "Date : "+strftime("%a, %d %b %Y %H:%M:%S ", localtime()) + "<br>"

# Add the version links here

textbody = textbody + "<br> <br>Version Log files          :  <A HREF="+logs_link+'>Log files created from running the night test</A><br>'

doc = Document()
MainDoc = Document()
isMainRev=0
 
doc= parse('/mcms/NightTest/night_test_results.xml')
if version_name == "Main" :
    isMainRev=1
    if os.path.isfile('/mcms/NightTest/main_night_test_results.xml'):    
        MainDoc= parse('/mcms/NightTest/main_night_test_results.xml')
        
    else:
        MainDoc= parse('/mcms/NightTest/night_test_results.xml')
        os.system("touch ~/nighttest/Main/DB/night_test_results.xml")
        
    os.system("ln -sf ~/nighttest/Main/DB/night_test_results.xml /mcms/NightTest/main_night_test_results.xml")
else:
    if os.path.isfile('/mcms/NightTest/fsn_500_night_test_results.xml'):    
        MainDoc= parse('/mcms/NightTest/fsn_500_night_test_results.xml')
    else:
        MainDoc= parse('/mcms/NightTest/night_test_results.xml')
        os.system("touch ~/nighttest/FSN-500/DB/night_test_results.xml")
    os.system("ln -sf ~/nighttest/FSN-500/DB/night_test_results.xml /mcms/NightTest/fsn_500_night_test_results.xml")
    
#os.path.isfile('')     
if GroupName !="All":
    textbody = GenerateReport(GroupName,textbody)
else:
    textbody = GenerateReport("Conference",textbody)
    textbody = GenerateReport("CallControl",textbody)
    textbody = GenerateReport("Management",textbody)
   # textbody = GenerateReport("Framework",textbody)

msgText = MIMEText(textbody, 'html')
msgAlternative.attach(msgText)

print "Update Night test DB to file "
filename=""
if isMainRev == 1:    
   filename="/mcms/NightTest/main_night_test_results.xml"
else:
   filename="/mcms/NightTest/fsn_500_night_test_results.xml"
     
f =  open(filename, "w+")    
MainDoc.writexml(f)
f.close()


#-----------------------------------------------------
#Mail sending


strToArray=strTo.split(',')

#userAccount = sys.argv[1]

#server = smtplib.SMTP('mail.polycom.co.il')
#server = smtplib.SMTP('isrexch01.israel.polycom.com')
server = smtplib.SMTP('ISRMailprd01.polycom.com')


server.verify(strTo)


server.sendmail(strTo, strToArray, msgRoot.as_string())

server.quit()




####
#msgText = MIMEText('<b>Some <i>HTML</i> text</b> and an image.<br><img src="cid:image1"><br>Nifty!', 'html')
####

# This example assumes the image is in the current directory
#fp = open('test.jpg', 'rb')
#msgImage = MIMEImage(fp.read())
#fp.close()

# Define the image's ID as referenced above
#msgImage.add_header('Content-ID', '<image1>')
#msgRoot.attach(msgImage)

# Send the email (this example assumes SMTP authentication is required)
#import smtplib
#smtp = smtplib.SMTP()
#smtp.connect('mail.polycom.co.il')
#smtp.login('exampleuser', 'examplepass')
#smtp.sendmail(strFrom, strTo, msgRoot.as_string())
#smtp.quit()
