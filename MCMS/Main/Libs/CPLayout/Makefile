INCPATHCPLayout  = -I Libs/CPLayout $(COMMONINCPATH)

INCPATHCPLayout_RMX  = $(INCPATHCPLayout)  $(XML2INC_RMX)
INCPATHCPLayout_SOFT  = $(INCPATHCPLayout) $(XML2INC_SOFT)

TARGETCPLayout_RMX  = $(BIN_RMX)/libCPLayout.so
TARGETCPLayout_SOFT = $(BIN_SOFT)/libCPLayout.so

TARGETS_RMX += $(TARGETCPLayout_RMX)
TARGETS_SOFT += $(TARGETCPLayout_SOFT)

CPPSOURCESCPLayout := $(shell find Libs/CPLayout |  grep "\.cpp$$")
SOURCESCPLayout :=  $(CPPSOURCESCPLayout)
OBJECTSCPLayout_RMX := $(CSOURCESCPLayout:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESCPLayout:.cpp=.$(RMX_PLATFORM).o)
OBJECTSCPLayout_SOFT := $(CSOURCESCPLayout:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESCPLayout:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSCPLayout := $(CSOURCESCPLayout:.c=.depend) $(CPPSOURCESCPLayout:.cpp=.depend)
DEPENDS += $(DEPENDSCPLayout)
-include Libs/CPLayout/*depend 
endif

PREREQUIRED_SOURCES += Libs/CPLayout/generate.flag

Libs/CPLayout/generate.flag: Libs/CPLayout/CPLayout.xml $(XML2SOURCE_BIN)
	@echo Building $@
	$(XML2SOURCE_BIN) Libs/CPLayout/CPLayout.xml Libs/CPLayout
	touch $@

Libs/CPLayout/%.$(RMX_PLATFORM).o: Libs/CPLayout/%.cpp Libs/CPLayout/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHCPLayout_RMX) -o $@ $<

Libs/CPLayout/%.$(SOFT_PLATFORM).o: Libs/CPLayout/%.cpp Libs/CPLayout/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHCPLayout_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/CPLayout/%.depend: Libs/CPLayout/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHCPLayout_SOFT) $<
endif

$(TARGETCPLayout_RMX):  $(OBJECTSCPLayout_RMX) $(DEPENDSCPLayout)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSCPLayout_RMX) $(STDLIB_RMX) $(XML2LIB_RMX)  -lpthread -shared

$(TARGETCPLayout_SOFT):  $(OBJECTSCPLayout_SOFT) $(DEPENDSCPLayout)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSCPLayout_SOFT) $(STDLIB_SOFT) $(XML2LIB_SOFT) -lpthread -shared
