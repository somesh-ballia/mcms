
MCMSINCD = ../McmIncld

COMMONINCPATH  = -I Libs/XmlPars \
		 -I Libs/ProcessBase \
		 -I Libs/Common \
		 -I Libs/BFCPLib \
		 -I Libs/SimLinux \
		 -I IncludeInternalMcms \
		 -I $(MCMSINCD)/Common \
		 -I $(MCMSINCD)/Common/CardMngrICE \
		 -I $(MCMSINCD)/MPL/Card/CardMngrBFCP \
		 -I $(MCMSINCD)/MPL/Shared/802_1x \
		 -I Libs/NDMLib \
		  $(XML2INC)

MPLAPIINC =	-I $(MCMSINCD)/MPL/Card/CardCommon \
		-I $(MCMSINCD)/MPL/Card/CardMngrICE \
		-I $(MCMSINCD)/MPL/Card/CardMngrTIP \
		-I $(MCMSINCD)/MPL/Card/CardMngrIpMedia \
		-I $(MCMSINCD)/MPL/Card/CardMngrIvrCntl \
		-I $(MCMSINCD)/MPL/Card/CardMngrMaintenance \
		-I $(MCMSINCD)/MPL/Card/CardMngrRecording \
		-I $(MCMSINCD)/MPL/Card/CardMngrTB \
		-I $(MCMSINCD)/MPL/Card/PhisycalPortNetISDN \
		-I $(MCMSINCD)/MPL/Card/PhisycalPortNetISDN \
		-I $(MCMSINCD)/MPL/Card/PhysicalPortART/Audio/ \
		-I $(MCMSINCD)/MPL/Card/PhysicalPortART/Common/ \
		-I $(MCMSINCD)/MPL/Card/PhysicalPortART/MUX/ \
		-I $(MCMSINCD)/MPL/Card/PhysicalPortART/RTP \
		-I $(MCMSINCD)/MPL/Card/PhysicalPortAudioCntl \
		-I $(MCMSINCD)/MPL/Card/PhysicalPortVideo \
		-I $(MCMSINCD)/MPL/Card/RtmIsdnMaintenance \
		-I $(MCMSINCD)/MPL/ShelfMngr \
		-I $(MCMSINCD)/MPL/Card/CardMngrBFCP \
		-I $(MCMSINCD)/MPL/Card/CardMngrTIP \
		-I $(MCMSINCD)/MPL/Card/PhysicalMrmp \
		-I $(MCMSINCD)/MPL/Shared/802_1x 


CSAPIINC = -I $(MCMSINCD)/CS/CsMaintenance \
	   -I $(MCMSINCD)/CS/CsSignaling










