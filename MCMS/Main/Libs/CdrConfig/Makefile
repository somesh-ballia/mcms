INCPATHCdrConfig  = -I Libs/CdrConfig $(COMMONINCPATH)

INCPATHCdrConfig_RMX  = $(INCPATHCdrConfig)
INCPATHCdrConfig_SOFT  = $(INCPATHCdrConfig)

TARGETCdrConfig_RMX  = $(BIN_RMX)/libCdrConfig.so
TARGETCdrConfig_SOFT = $(BIN_SOFT)/libCdrConfig.so

TARGETS_RMX += $(TARGETCdrConfig_RMX)
TARGETS_SOFT += $(TARGETCdrConfig_SOFT)

CPPSOURCESCdrConfig := $(shell find Libs/CdrConfig |  grep "\.cpp$$")
SOURCESCdrConfig :=  $(CPPSOURCESCdrConfig)
OBJECTSCdrConfig_RMX := $(CSOURCESCdrConfig:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESCdrConfig:.cpp=.$(RMX_PLATFORM).o)
OBJECTSCdrConfig_SOFT := $(CSOURCESCdrConfig:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESCdrConfig:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSCdrConfig := $(CSOURCESCdrConfig:.c=.depend) $(CPPSOURCESCdrConfig:.cpp=.depend)
DEPENDS += $(DEPENDSCdrConfig)
-include Libs/CdrConfig/*depend 
endif

PREREQUIRED_SOURCES += Libs/CdrConfig/generate.flag

Libs/CdrConfig/generate.flag: Libs/CdrConfig/CdrConfigApi.xml $(XML2SOURCE_BIN)
	@echo Building $@
	$(XML2SOURCE_BIN) Libs/CdrConfig/CdrConfigApi.xml Libs/CdrConfig
	touch $@

Libs/CdrConfig/%.$(RMX_PLATFORM).o: Libs/CdrConfig/%.cpp Libs/CdrConfig/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHCdrConfig_RMX) -o $@ $<

Libs/CdrConfig/%.$(SOFT_PLATFORM).o: Libs/CdrConfig/%.cpp Libs/CdrConfig/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHCdrConfig_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/CdrConfig/%.depend: Libs/CdrConfig/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHCdrConfig_SOFT) $<
endif

$(TARGETCdrConfig_RMX):  $(OBJECTSCdrConfig_RMX) $(DEPENDSCdrConfig)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSCdrConfig_RMX) $(STDLIB_RMX) $(XML2LIB)  -lpthread -shared

$(TARGETCdrConfig_SOFT):  $(OBJECTSCdrConfig_SOFT) $(DEPENDSCdrConfig)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSCdrConfig_SOFT) $(STDLIB_SOFT) $(XML2LIB) -lpthread -shared
