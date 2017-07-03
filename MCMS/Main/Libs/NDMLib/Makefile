INCPATHNDMLib = \
	-I Libs/NDMLib \
	-I ../McmIncld/Common \
	-I IncludeInternalMcms

INCPATHNDMLib_RMX = $(INCPATHNDMLib)$(NDMINC_RMX) 
INCPATHNDMLib_SOFT = $(INCPATHNDMLib)$(NDMINC_SOFT)

TARGETNDMLib_RMX = $(BIN_RMX)/libNDM.so
TARGETNDMLib_SOFT = $(BIN_SOFT)/libNDM.so

SOURCESNDMLib := $(shell find Libs/NDMLib |  grep -v /Tests/ | grep "\.cpp$$")
OBJECTSNDMLib_RMX := $(SOURCESNDMLib:.cpp=.$(RMX_PLATFORM).o)
OBJECTSNDMLib_SOFT := $(SOURCESNDMLib:.cpp=.$(SOFT_PLATFORM).o)
DEPENDSNDMLib := $(SOURCESNDMLib:.cpp=.depend)

TARGETS_RMX += $(TARGETNDMLib_RMX)
TARGETS_SOFT += $(TARGETNDMLib_SOFT)

ifneq ($(MAKECMDGOALS) , clean)
-include $(DEPENDSNDMLib)
endif

Libs/NDMLib/%.$(RMX_PLATFORM).o: Libs/NDMLib/%.cpp Libs/NDMLib/%.depend
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHNDMLib_RMX) -o $@ $<

Libs/NDMLib/%.$(SOFT_PLATFORM).o: Libs/NDMLib/%.cpp Libs/NDMLib/%.depend
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHNDMLib_SOFT) -o $@ $<

Libs/NDMLib/%.depend: Libs/NDMLib/%.cpp
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHNDMLib_SOFT) $< 

$(TARGETNDMLib_RMX):  $(OBJECTSNDMLib_RMX)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSNDMLib_RMX) -lpthread -shared 


$(TARGETNDMLib_SOFT): $(OBJECTSNDMLib_SOFT)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSNDMLib_SOFT) -lpthread -shared 
