INCPATHCdrMccf  = -I Libs/CdrMccf $(COMMONINCPATH)

INCPATHCdrMccf_RMX  = $(INCPATHCdrMccf)
INCPATHCdrMccf_SOFT  = $(INCPATHCdrMccf)


TARGETCdrMccf_RMX  = $(BIN_RMX)/libCdrMccf.so
TARGETCdrMccf_SOFT = $(BIN_SOFT)/libCdrMccf.so

TARGETS_RMX += $(TARGETCdrMccf_RMX)
TARGETS_SOFT += $(TARGETCdrMccf_SOFT)

CPPSOURCESCdrMccf := $(shell find Libs/CdrMccf |  grep "\.cpp$$")
SOURCESCdrMccf :=  $(CPPSOURCESCdrMccf)
OBJECTSCdrMccf_RMX := $(CSOURCESCdrMccf:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESCdrMccf:.cpp=.$(RMX_PLATFORM).o)
OBJECTSCdrMccf_SOFT := $(CPPSOURCESCdrMccf:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSCdrMccf := $(CPPSOURCESCdrMccf:.cpp=.depend)
DEPENDS += $(DEPENDSCdrMccf)
-include Libs/CdrMccf/*depend
endif

PREREQUIRED_SOURCES += Libs/CdrMccf/generate.flag

Libs/CdrMccf/generate.flag: Libs/CdrMccf/CdrCommonApi.xml $(XML2SOURCE_BIN)
	@echo Building $@
	$(XML2SOURCE_BIN) Libs/CdrMccf/CdrCommonApi.xml Libs/CdrMccf
	touch $@
	#export LD_LIBRARY_PATH=$(BIN_SOFT);$(BIN_SOFT)/Xml2Source Libs/CdrMccf/CdrCommonApi.xml Libs/CdrMccf

Libs/CdrMccf/%.$(RMX_PLATFORM).o: Libs/CdrMccf/%.cpp Libs/CdrMccf/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHCdrMccf_RMX) -o $@ $<

Libs/CdrMccf/%.$(SOFT_PLATFORM).o: Libs/CdrMccf/%.cpp Libs/CdrMccf/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHCdrMccf_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/CdrMccf/%.depend: Libs/CdrMccf/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHCdrMccf_SOFT) $<
endif

$(TARGETCdrMccf_RMX): $(OBJECTSCdrMccf_RMX) $(DEPENDSCdrMccf)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSCdrMccf_RMX) $(STDLIB_RMX) $(XML2LIB)  -lpthread -shared

$(TARGETCdrMccf_SOFT): $(OBJECTSCdrMccf_SOFT) $(DEPENDSCdrMccf) 
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSCdrMccf_SOFT) $(STDLIB_SOFT) $(XML2LIB) -lpthread -shared
