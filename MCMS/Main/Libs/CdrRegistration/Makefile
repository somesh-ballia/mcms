INCPATHCdrRegistration  = -I Libs/CdrRegistration $(COMMONINCPATH)

INCPATHCdrRegistration_RMX  = $(INCPATHCdrRegistration)
INCPATHCdrRegistration_SOFT  = $(INCPATHCdrRegistration)

TARGETCdrRegistration_RMX  = $(BIN_RMX)/libCdrRegistration.so
TARGETCdrRegistration_SOFT = $(BIN_SOFT)/libCdrRegistration.so

TARGETS_RMX += $(TARGETCdrRegistration_RMX)
TARGETS_SOFT += $(TARGETCdrRegistration_SOFT)

CPPSOURCESCdrRegistration := $(shell find Libs/CdrRegistration |  grep "\.cpp$$")
SOURCESCdrRegistration :=  $(CPPSOURCESCdrRegistration)
OBJECTSCdrRegistration_RMX := $(CSOURCESCdrRegistration:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESCdrRegistration:.cpp=.$(RMX_PLATFORM).o)
OBJECTSCdrRegistration_SOFT := $(CSOURCESCdrRegistration:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESCdrRegistration:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSCdrRegistration := $(CSOURCESCdrRegistration:.c=.depend) $(CPPSOURCESCdrRegistration:.cpp=.depend)
DEPENDS += $(DEPENDSCdrRegistration)
-include Libs/CdrRegistration/*depend 
endif

PREREQUIRED_SOURCES += Libs/CdrRegistration/generate.flag

Libs/CdrRegistration/generate.flag: Libs/CdrRegistration/CdrRegistrationApi.xml $(XML2SOURCE_BIN)
	@echo Building $@
	$(XML2SOURCE_BIN) Libs/CdrRegistration/CdrRegistrationApi.xml Libs/CdrRegistration
	touch $@

Libs/CdrRegistration/%.$(RMX_PLATFORM).o: Libs/CdrRegistration/%.cpp Libs/CdrRegistration/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHCdrRegistration_RMX) -o $@ $<

Libs/CdrRegistration/%.$(SOFT_PLATFORM).o: Libs/CdrRegistration/%.cpp Libs/CdrRegistration/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHCdrRegistration_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/CdrRegistration/%.depend: Libs/CdrRegistration/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHCdrRegistration_SOFT) $<
endif

$(TARGETCdrRegistration_RMX):  $(OBJECTSCdrRegistration_RMX) $(DEPENDSCdrRegistration)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSCdrRegistration_RMX) $(STDLIB_RMX) $(XML2LIB)  -lpthread -shared

$(TARGETCdrRegistration_SOFT):  $(OBJECTSCdrRegistration_SOFT) $(DEPENDSCdrRegistration)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSCdrRegistration_SOFT) $(STDLIB_SOFT) $(XML2LIB) -lpthread -shared
