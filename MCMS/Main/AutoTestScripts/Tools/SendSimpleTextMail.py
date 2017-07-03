#! /usr/bin/env python

# Import smtplib for the actual sending function
from time import gmtime, strftime, localtime
import smtplib
import sys

from datetime import date

# Import the email modules we'll need
from email.mime.text import MIMEText
from socket import gethostname;

strTo = sys.argv[1]
mailSubject = sys.argv[2]
mailContent = sys.argv[3]

strFrom = 'CarmelNightest@polycom.co.il'


textbody = '<html><head><title></title></head><body>'
textbody = textbody + "Reporting machine: "+gethostname()+' <br>'
textbody = textbody + "Date : "+strftime("%a, %d %b %Y %H:%M:%S ", localtime()) + "<br>" + "<br>"+ "<br>"

textbody = textbody + "<b><pre>" + mailContent + "</pre></b>" + "<br>"
textbody = textbody + '</body></html>'

msg = MIMEText(textbody, 'html')

msg['Subject'] = mailSubject
msg['From'] = strFrom
msg['To'] = strTo

strToArray=strTo.split(',')

# Send the message via our own SMTP server, but don't include the
# envelope header.
server = smtplib.SMTP('ISRMailprd01.polycom.com')

server.verify(strTo)

server.sendmail(strTo, strToArray, msg.as_string())

server.quit()

