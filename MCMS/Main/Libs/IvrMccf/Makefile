INCPATHIvrMccf  = -I Libs/IvrMccf $(COMMONINCPATH)

INCPATHIvrMccf_RMX  = $(INCPATHIvrMccf)
INCPATHIvrMccf_SOFT  = $(INCPATHIvrMccf)

TARGETIvrMccf_RMX  = $(BIN_RMX)/libIvrMccf.so
TARGETIvrMccf_SOFT = $(BIN_SOFT)/libIvrMccf.so

TARGETS_RMX += $(TARGETIvrMccf_RMX)
TARGETS_SOFT += $(TARGETIvrMccf_SOFT)

CPPSOURCESIvrMccf := $(shell find Libs/IvrMccf |  grep "\.cpp$$")
SOURCESIvrMccf :=  $(CPPSOURCESIvrMccf)

OBJECTSIvrMccf_RMX := $(CPPSOURCESIvrMccf:.cpp=.$(RMX_PLATFORM).o)
OBJECTSIvrMccf_SOFT := $(CPPSOURCESIvrMccf:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSIvrMccf := $(CPPSOURCESIvrMccf:.cpp=.depend)
DEPENDS += $(DEPENDSIvrMccf)
-include Libs/IvrMccf/*depend 
endif

PREREQUIRED_SOURCES += Libs/IvrMccf/generate.flag

Libs/IvrMccf/generate.flag: Libs/IvrMccf/IvrCommonApi.xml $(XML2SOURCE_BIN)
	@echo Building $@
	$(XML2SOURCE_BIN) Libs/IvrMccf/IvrCommonApi.xml Libs/IvrMccf
	touch $@

Libs/IvrMccf/%.$(RMX_PLATFORM).o: Libs/IvrMccf/%.cpp Libs/IvrMccf/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHIvrMccf_RMX) -o $@ $<

Libs/IvrMccf/%.$(SOFT_PLATFORM).o: Libs/IvrMccf/%.cpp Libs/IvrMccf/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHIvrMccf_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/IvrMccf/%.depend: Libs/IvrMccf/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHIvrMccf_SOFT) $<
endif

$(TARGETIvrMccf_RMX): $(OBJECTSIvrMccf_RMX) $(DEPENDSIvrMccf)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSIvrMccf_RMX) $(STDLIB_RMX) $(XML2LIB)  -lpthread -shared

$(TARGETIvrMccf_SOFT): $(OBJECTSIvrMccf_SOFT) $(DEPENDSIvrMccf)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSIvrMccf_SOFT) $(STDLIB_SOFT) $(XML2LIB) -lpthread -shared


