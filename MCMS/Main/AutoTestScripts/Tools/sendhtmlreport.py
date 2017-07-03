#! /usr/bin/env python
# Send an HTML email with an embedded image and a plain text message for
# email clients that don't want to display the HTML.
#
#sys.argv[1] is the email adress to send the report to


from time import gmtime, strftime, localtime
import smtplib
import sys
import tarfile
from datetime import date

from email.MIMEMultipart import MIMEMultipart
from email.MIMEText import MIMEText
from email.MIMEImage import MIMEImage
from email.MIMEBase import MIMEBase
from email import Encoders

import os
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




# Define these once; use them twice!
strFrom = 'CarmelNightest@polycom.co.il'
strTo = sys.argv[1]
#now = date.today()

outputFolderPath=sys.argv[2]
daydir=sys.argv[3] #added
cc_revision_name = sys.argv[4]

version_name = sys.argv[5]


print "Output dir is: " + outputFolderPath
print "designated mail is: "+ strTo

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



try:
    fp = open(outputFolderPath+'/mail.txt', 'rt')
except IOError,e:
    (errno, strerror) = e
    print "Error in open: " +  outputFolderPath+"/mail.txt , file: Error number", errno, "(%s) !!!!!!!" % strerror


print "processing report..."
line = fp.readline()

refText="<h4>"
mailText=""
beginToInsertRef = False


while line:
	if ((line.find('#') != -1) or (line.find('*') != -1)):
		refTag = str(GetRefTag(line))
		if ((len(refTag) > 0) and beginToInsertRef == False):		
			beginToInsertRef=True
			textbody = textbody + mailText
			mailText="<br>"		
		if (line.find('#') != -1):
			lineToAdd = '<span style="color:blue">' + line + '</span>'
		else:
			lineToAdd = '<span style="color:red">' + line + '</span>'

		if (len(refTag) > 0):
			refText =   refText + GetRefLink(lineToAdd, refTag) 
			lineToAdd = "<h4>" + "<a name=\"" + refTag + "\">" + lineToAdd + "</a>"  + "</h4>"
		else:
			lineToAdd = "<h4>" + lineToAdd + "</h4>"
		mailText = mailText + lineToAdd
	elif (line.find('^') != -1):
		mailText = mailText + '<h4><span style="color:green">' + line + '</span></h4>'
	elif (line.find('+') != -1):
		mailText = mailText + '<hr color=blue />'
	else:
		mailText = mailText + line
	mailText = mailText  + '<br>'
	line = fp.readline()
fp.close()
refText=refText + "</h4>"


print "adding links.."

#For each client add link for the running ENV
#Folder is the current day
#folderPath=outputFolderPath + "/" +GetDayName( now.weekday())
folderPath=outputFolderPath + "/" + daydir

for entry in os.listdir(folderPath): 
    if ( os.path.isdir(os.path.join(folderPath,entry))):
        continue
    else:
        if entry.find('Data.log') != -1 :
            print "Find Client Info: " + entry
            #Read the data of hte client
            fileObj = open(os.path.join(folderPath,entry))
            clientData=fileObj.readline()
            print "Reading ClientDatat Line: " + clientData
            fileObj.close()
            
            if len(clientData) == 0:
                print "file: " + entry + ", has an empty content,Can not add the Link to the Mail!!"
                continue
            dataArr=clientData.split(':')
            clientName=dataArr[1]
            clientSnapshotPath=dataArr[2]
            version_link = "\\\\"+clientName + clientSnapshotPath.replace ( 'home/', 'localhome/' )
            print "Client Name=" + clientName+ " ,Path=" + version_link
            textbody = textbody + "<br> <br>"+clientName+" snapshot       :  <A HREF="+version_link+'>'+clientName +" core dumps</A><br>"
            
    
textbody = textbody + refText + mailText

textbody = textbody + "<br> <br>REPORTED ON  " + strftime("%a, %d %b %Y %H:%M:%S ", localtime()) + "<br>"

textbody = textbody + '</FONT></body></html>'
msgText = MIMEText(textbody, 'html')
msgAlternative.attach(msgText)


##
#files=[GetDayName( now.weekday())+"/test_report.txt"] 26.11 eran changed it to
files=[folderPath+"/test_report.txt"]
print folderPath
#natasha added
os.chdir(folderPath)
tar = tarfile.open("test_report.tar.gz", "w:gz")
tar.add("test_report.txt")
tar.close()
files=[folderPath+"/test_report.tar.gz", outputFolderPath+"/process_valgrind_summary.txt"]
                    
print "adding attachment.."

for file in files:
        part = MIMEBase('application', "octet-stream")
        part.set_payload( open(file,"rb").read() )
        Encoders.encode_base64(part)
        part.add_header('Content-Disposition', 'attachment; filename="%s"'
                       % os.path.basename(file))
        msgRoot.attach(part)




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
