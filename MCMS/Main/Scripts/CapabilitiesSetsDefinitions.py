#!/mcms/python/bin/python

##############################################################################
# capability sets definitions
#
# Date: 8/8/06
# By  : Eitan 
#
############################################################################

# capsets that are already defined in EP-SIM
# the first entry is h243 name that will the name of the party
FullCapSet = ["AUDIO_SIREN14","AUDIO_G7221","AUDIO_G7221","AUDIO_G722","AUDIO_G7231","AUDIO_G729","AUDIO_G728","AUDIO_G711","VIDEO_H264","VIDEO_H263","VIDEO_VP8"] ##N.A. DEBUG VP8
G711_H264_CapSet = ["AUDIO_G711","VIDEO_H264"]
G711_H263_CapSet = ["AUDIO_G711","VIDEO_H263"]
G711_H264_H263_CapSet = ["AUDIO_G711","VIDEO_H263","VIDEO_H264"]
WebRTC_CapSet = ["AUDIO_SIREN14","AUDIO_G7221","AUDIO_G7221","AUDIO_G722","AUDIO_G7231","AUDIO_G729","AUDIO_G728","AUDIO_G711","VIDEO_VP8"] ##N.A. DEBUG VP8

# dictionary that conains all defined capsets (capsetName:capset) 
DefinedCapSets = {"FULL CAPSET":FullCapSet, "G711+H264sd+Fecc":G711_H264_CapSet,"G711+H263+Fecc":G711_H263_CapSet,"G711+H264sd+H263+Fecc":G711_H264_H263_CapSet} ##N.A. DEBUG VP8

# manualy defined capsets - fill free to add any combination you have in mind... 

Fixed_H263_CapSet = ["AUDIO_SIREN14","AUDIO_G7221","AUDIO_G7221","AUDIO_G722","AUDIO_G7231","AUDIO_G729","AUDIO_G728","AUDIO_G711","VIDEO_H263"]
Fixed_H264_CapSet = ["AUDIO_SIREN14","AUDIO_G7221","AUDIO_G7221","AUDIO_G722","AUDIO_G7231","AUDIO_G729","AUDIO_G728","AUDIO_G711","VIDEO_H264"]

# dictionary that conains all man-defined capsets (capsetName:capset)
UndefinedCapSets = {"FIXED H263":Fixed_H263_CapSet,"FIXED H264":Fixed_H264_CapSet}
