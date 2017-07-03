INCPATHCdrPersist  = -I Libs/CdrPersist $(COMMONINCPATH)

INCPATHCdrPersist_RMX  = $(INCPATHCdrPersist)
INCPATHCdrPersist_SOFT  = $(INCPATHCdrPersist)

TARGETCdrPersist_RMX  = $(BIN_RMX)/libCdrPersist.so
TARGETCdrPersist_SOFT = $(BIN_SOFT)/libCdrPersist.so

TARGETS_RMX += $(TARGETCdrPersist_RMX)
TARGETS_SOFT += $(TARGETCdrPersist_SOFT)

CPPSOURCESCdrPersist := $(shell find Libs/CdrPersist |  grep "\.cpp$$")
SOURCESCdrPersist :=  $(CPPSOURCESCdrPersist)
OBJECTSCdrPersist_RMX := $(CSOURCESCdrPersist:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESCdrPersist:.cpp=.$(RMX_PLATFORM).o)
OBJECTSCdrPersist_SOFT := $(CSOURCESCdrPersist:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESCdrPersist:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSCdrPersist := $(CSOURCESCdrPersist:.c=.depend) $(CPPSOURCESCdrPersist:.cpp=.depend)
DEPENDS += $(DEPENDSCdrPersist)
-include Libs/CdrPersist/*depend 
endif

PREREQUIRED_SOURCES += Libs/CdrPersist/generate.flag

Libs/CdrPersist/generate.flag: Libs/CdrPersist/CdrPersistApi.xml $(XML2SOURCE_BIN)
	@echo Building $@
	$(XML2SOURCE_BIN) Libs/CdrPersist/CdrPersistApi.xml Libs/CdrPersist
	touch $@

Libs/CdrPersist/%.$(RMX_PLATFORM).o: Libs/CdrPersist/%.cpp Libs/CdrPersist/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHCdrPersist_RMX) -o $@ $<

Libs/CdrPersist/%.$(SOFT_PLATFORM).o: Libs/CdrPersist/%.cpp Libs/CdrPersist/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHCdrPersist_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/CdrPersist/%.depend: Libs/CdrPersist/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHCdrPersist_SOFT) $<
endif

$(TARGETCdrPersist_RMX):  $(OBJECTSCdrPersist_RMX) $(DEPENDSCdrPersist)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSCdrPersist_RMX) $(STDLIB_RMX) $(XML2LIB)  -lpthread -shared

$(TARGETCdrPersist_SOFT):  $(OBJECTSCdrPersist_SOFT) $(DEPENDSCdrPersist)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSCdrPersist_SOFT) $(STDLIB_SOFT) $(XML2LIB) -lpthread -shared
