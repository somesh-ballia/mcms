INCPATHBFCPLib = \
	-I Libs/BFCPLib \
	-I Libs/ProcessBase \
	-I Libs/XmlPars \
	-I Libs/Common \
	-I IncludeInternalMcms \
	-I ../McmIncld/Common \
	-I ../McmIncld/CS/CsSignaling \
	-I ../McmIncld/MPL/Card/CardCommon \
	-I ../McmIncld/MPL/Card/CardMngrMaintenance \
	-I ../McmIncld/MPL/Card/CardMngrRecording \
	-I ../McmIncld/MPL/Card/CardMngrTB \
	-I ../McmIncld/MPL/Card/CardMngrIvrCntl \
	-I ../McmIncld/MPL/Card/PhysicalPortART/Audio \
	-I ../McmIncld/MPL/Card/PhysicalPortAudioCntl \
	-I ../McmIncld/MPL/Card/PhysicalPortVideo \
	-I ../McmIncld/MPL/Card/CardMngrIpMedia \
	-I ../McmIncld/MPL/ShelfMngr \
	-I ../McmIncld/MPL/Card/CardMngrICE \
 	-I Processes/ConfParty/ConfPartyLib

TARGETBFCPLib_RMX   = Bin.$(RMX_PLATFORM)/libBFCP.so
TARGETBFCPLib_SOFT   = Bin.$(SOFT_PLATFORM)/libBFCP.so

CSOURCESBFCPLib := $(shell find Libs/BFCPLib | grep -v /Tests/ | grep "\.c$$")
SOURCESBFCPLib := $(CSOURCESBFCPLib) 
OBJECTSBFCPLib_RMX := $(CSOURCESBFCPLib:.c=.$(RMX_PLATFORM).o)
OBJECTSBFCPLib_SOFT := $(CSOURCESBFCPLib:.c=.$(SOFT_PLATFORM).o) 

ifndef SKIP_DEPEND
DEPENDSBFCPLib := $(CSOURCESBFCPLib:.c=.depend) 
DEPENDS += $(DEPENDSBFCPLib)
-include Libs/BFCPLib/*depend
endif

TARGETS += $(TARGETBFCPLib_RMX) $(TARGETBFCPLib_SOFT) 


Libs/BFCPLib/%.$(RMX_PLATFORM).o: Libs/BFCPLib/%.c
	@echo Building $@
	$(Q)$(DISTCC_RMX) -c $(CFLAGS_RMX) $(INCPATHBFCPLib) -o $@ $<

Libs/BFCPLib/%.$(SOFT_PLATFORM).o: Libs/BFCPLib/%.c
	@echo Building $@
	$(Q)$(DISTCC_SOFT) -c $(CFLAGS_SOFT) $(INCPATHBFCPLib) -o $@ $<

ifndef SKIP_DEPEND
Libs/BFCPLib/%.depend: Libs/BFCPLib/%.c 
	@echo Building $@
	$(Q)$(CC_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CFLAGS_SOFT) $(INCPATHBFCPLib) $<
endif

$(TARGETBFCPLib_RMX):  $(OBJECTSBFCPLib_RMX) $(DEPENDSBFCPLib)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSBFCPLib_RMX) $(STDLIB_RMX) -lpthread -shared 

$(TARGETBFCPLib_SOFT):  $(OBJECTSBFCPLib_SOFT) $(DEPENDSBFCPLib)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSBFCPLib_SOFT) $(STDLIB_SOFT) -lpthread -shared 
