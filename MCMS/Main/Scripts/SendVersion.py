#!/mcms/python/bin/python

from McmsConnection import *
source=" /tmp/build_install/*.bin"
dest="/data/new_version/new_version.bin"
user="POLYCOM"

c = McmsConnection()
c.Connect()

c.SendXmlFile("./Scripts/SendVersion/BeginReceiving.xml");
print "Sending version using POLYCOM default user"
os.system("scp  "+source+" "+user+"@"+c.ip+":"+dest);
sleep(10)
c.SendXmlFile("./Scripts/SendVersion/EndReceiving.xml");
print "Reset the MCU now!"

c.Disconnect()

