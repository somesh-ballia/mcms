INCPATHProcessBase  = -I Libs/ProcessBase \
					-I Libs/Common \
				    -I Libs/XmlPars \
					-I IncludeInternalMcms \
					-I ../McmIncld/CS/CsSignaling \
					-I ../McmIncld/MPL/Card/CardCommon \
					-I Libs/CdrPersist \
					-I ../McmIncld/Common \
					-I ../McmIncld/MPL/Card/CardMngrMaintenance \
					-I ../McmIncld/MPL/Card/CardMngrRecording \
					-I ../McmIncld/MPL/Card/CardMngrTB \
					-I ../McmIncld/MPL/Card/CardMngrIvrCntl \
					-I ../McmIncld/MPL/Card/PhysicalPortART/Audio \
					-I ../McmIncld/MPL/Card/PhysicalPortART/MUX \
					-I ../McmIncld/MPL/Card/PhysicalPortAudioCntl \
					-I ../McmIncld/MPL/Card/PhysicalPortVideo \
					-I ../McmIncld/MPL/Card/CardMngrIpMedia \
					-I ../McmIncld/MPL/Card/PhisycalPortNetISDN \
					-I ../McmIncld/MPL/Card/RtmIsdnMaintenance \
					-I ../McmIncld/MPL/ShelfMngr \
					-I ../McmIncld/MPL/Card/CardMngrICE \
					-I ../McmIncld/MPL/Card/CardMngrTIP \
					-I ../McmIncld/MPL/Card/CardMngrBFCP \
					-I ../McmIncld/MPL/Card/PhysicalMrmp \
					-I ../McmIncld/MPL/Shared/802_1x \
					-I ../Common \
					$(XML2INC)

INCPATHProcessBase_RMX = $(INCPATHProcessBase) $(ZLIBINC_RMX)
INCPATHProcessBase_SOFT = $(INCPATHProcessBase) $(ZLIBINC_SOFT)

TARGETProcessBase_RMX   = $(BIN_RMX)/libProcessBase.so
TARGETProcessBase_SOFT  = $(BIN_SOFT)/libProcessBase.so

SOURCESProcessBase := $(shell find Libs/ProcessBase | grep "\.cpp$$")
OBJECTSProcessBase_RMX := $(SOURCESProcessBase:.cpp=.$(RMX_PLATFORM).o)
OBJECTSProcessBase_SOFT := $(SOURCESProcessBase:.cpp=.$(SOFT_PLATFORM).o)

TARGETS_RMX += $(TARGETProcessBase_RMX)
TARGETS_SOFT += $(TARGETProcessBase_SOFT)

ifndef SKIP_DEPEND
	DEPENDSProcessBase := $(SOURCESProcessBase:.cpp=.depend)
	DEPENDS += $(DEPENDSProcessBase)
	-include Libs/ProcessBase/*depend
endif

Libs/ProcessBase/%.$(RMX_PLATFORM).o: Libs/ProcessBase/%.cpp
	@echo Building $@ 
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX)  $(INCPATHProcessBase_RMX) -o $@ $<

Libs/ProcessBase/%.$(SOFT_PLATFORM).o: Libs/ProcessBase/%.cpp
	@echo Building $@ 
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT)  $(INCPATHProcessBase_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/ProcessBase/%.depend: Libs/ProcessBase/%.cpp
	@echo Building $@ 
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHProcessBase_SOFT) $<
endif

$(TARGETProcessBase_RMX):  $(OBJECTSProcessBase_RMX)  $(DEPENDSProcessBase)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSProcessBase_RMX) $(XML2LIB) -lpthread -shared


$(TARGETProcessBase_SOFT):  $(OBJECTSProcessBase_SOFT)  $(DEPENDSProcessBase)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSProcessBase_SOFT) $(XML2LIB) -lpthread -shared

