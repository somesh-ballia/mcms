INCPATHTimeConfig  = -I Libs/TimeConfig $(COMMONINCPATH)

INCPATHTimeConfig_RMX  = $(INCPATHTimeConfig)
INCPATHTimeConfig_SOFT  = $(INCPATHTimeConfig)

TARGETTimeConfig_RMX  = $(BIN_RMX)/libTimeConfig.so
TARGETTimeConfig_SOFT = $(BIN_SOFT)/libTimeConfig.so

TARGETS_RMX += $(TARGETTimeConfig_RMX)
TARGETS_SOFT += $(TARGETTimeConfig_SOFT)

CPPSOURCESTimeConfig := $(shell find Libs/TimeConfig |  grep "\.cpp$$")
SOURCESTimeConfig :=  $(CPPSOURCESTimeConfig)
OBJECTSTimeConfig_RMX := $(CSOURCESTimeConfig:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESTimeConfig:.cpp=.$(RMX_PLATFORM).o)
OBJECTSTimeConfig_SOFT := $(CSOURCESTimeConfig:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESTimeConfig:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSTimeConfig := $(CSOURCESTimeConfig:.c=.depend) $(CPPSOURCESTimeConfig:.cpp=.depend)
DEPENDS += $(DEPENDSTimeConfig)
-include Libs/TimeConfig/*depend 
endif

PREREQUIRED_SOURCES += Libs/TimeConfig/generate.flag

Libs/TimeConfig/generate.flag: Libs/TimeConfig/TimeConfigApi.xml $(XML2SOURCE_BIN)
	@echo Building $@
	$(XML2SOURCE_BIN) Libs/TimeConfig/TimeConfigApi.xml Libs/TimeConfig
	touch $@

Libs/TimeConfig/%.$(RMX_PLATFORM).o: Libs/TimeConfig/%.cpp Libs/TimeConfig/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHTimeConfig_RMX) -o $@ $<

Libs/TimeConfig/%.$(SOFT_PLATFORM).o: Libs/TimeConfig/%.cpp Libs/TimeConfig/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHTimeConfig_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/TimeConfig/%.depend: Libs/TimeConfig/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHTimeConfig_SOFT) $<
endif

$(TARGETTimeConfig_RMX):  $(OBJECTSTimeConfig_RMX) $(DEPENDSTimeConfig)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSTimeConfig_RMX) $(STDLIB_RMX) $(XML2LIB)  -lpthread -shared

$(TARGETTimeConfig_SOFT):  $(OBJECTSTimeConfig_SOFT) $(DEPENDSTimeConfig)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSTimeConfig_SOFT) $(STDLIB_SOFT) $(XML2LIB) -lpthread -shared
