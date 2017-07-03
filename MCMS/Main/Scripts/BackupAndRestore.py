#!/mcms/python/bin/python


from McmsConnection import *
import os

c = McmsConnection()
c.Connect()

print "Verifying no Backup file exists ..."
backup = os.listdir("Backup/")
if len(backup) > 0:
    ScriptAbort("Backup file exists although backup request was not sent yet.")
else:
    print "OK."
    
    
print "Request Backup ..."
c.SendXmlFile("Scripts/BackupRestore/BackupConfigStart.xml")

c.Sleep(10)

backup = os.listdir("Backup/")
if len(backup) == 1:
    if backup[0].find(".bck"):
        print "Found Backupfile!"
    else:
        ScriptAbort("Backup file was created with unexpected format.")
else:
    ScriptAbort("Backup file was not created.")

c.SendXmlFile("Scripts/BackupRestore/BackupConfigFinish.xml")
c.Sleep(10)

backup = os.listdir("Backup/")
if len(backup) == 0:
    print "Verifying backup file was removed .... OK!"
else:
    ScriptAbort("Backup file was not removed.")
