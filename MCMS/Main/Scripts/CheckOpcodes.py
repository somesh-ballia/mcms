#!/mcms/python/bin/python

import re

dct = {}
dctDuplicate = {}
num = 0
st = ""
reg = re.compile('[0-9]')
fl = open("IncludeInternalMcms/OpcodesMcmsInternal.h")
for line in fl :       
    items = line.split()
    if items and len(items) > 2 and (items[0] == "const" or items[0] == "#define"):
        if len(items) > 4 and items[0] == "const" and items[1] == "OPCODE":
            num = ''.join(reg.findall(items[4]))            
            st = items[2]            
        elif items[0] == "#define":
            num = ''.join(reg.findall(items[2]))
            st = items[1]            
        if num in dct:
            dctDuplicate[num] = st                
        else:
            dct[num] = st       

fl.close()
for k,v in dctDuplicate.items():
    print "duplicate opcode in IncludeInternalMcms/OpcodesMcmsInternal.h", k
if len(dctDuplicate) > 0:
    exit(1)
else:
    exit(0)

