#!/mcms/python/bin/python

import sys

_setOpcodes = set()

def ParseLine(line):
    i = line.find("//")
    if (i >= 0):
        line = line[:i]
    items = line.split()
    if (len(items) > 2):
        if (len(items) >= 5):
            if (items[4].endswith(";")):
                items[4] = items[4][:-1]
                if (items[4] in _setOpcodes):
                    print "Duplicated opcode: ", items[4]
                else:
                    _setOpcodes.add(items[4])
        else:
            if (items[0] == "#define"):
                if (items[2] in _setOpcodes):
                    print "Duplicated opcode: ", items[2]
                else:
                    _setOpcodes.add(items[2])
#    print items


f = open("../Processes/ConfParty/ConfPartyLib/ConfPartyOpcodes.h")
for line in f:
    s = line.strip().replace("\t", " ")
    s = s.replace("=", " = ")
    words = s.split()
#    if (len(words) > 2 and words[1] == "OPCODE"):
    ParseLine(s)

