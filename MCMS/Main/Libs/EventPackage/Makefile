INCPATHEventPackage  = -I Libs/EventPackage $(COMMONINCPATH)

INCPATHEventPackage_RMX  = $(INCPATHEventPackage)  $(XML2INC_RMX)
INCPATHEventPackage_SOFT  = $(INCPATHEventPackage) $(XML2INC_SOFT)

TARGETEventPackage_RMX  = $(BIN_RMX)/libEventPackage.so
TARGETEventPackage_SOFT = $(BIN_SOFT)/libEventPackage.so

TARGETS_RMX += $(TARGETEventPackage_RMX)
TARGETS_SOFT += $(TARGETEventPackage_SOFT)

CPPSOURCESEventPackage := $(shell find Libs/EventPackage |  grep "\.cpp$$")
SOURCESEventPackage :=  $(CPPSOURCESEventPackage)
OBJECTSEventPackage_RMX := $(CSOURCESEventPackage:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESEventPackage:.cpp=.$(RMX_PLATFORM).o)
OBJECTSEventPackage_SOFT := $(CSOURCESEventPackage:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESEventPackage:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSEventPackage := $(CSOURCESEventPackage:.c=.depend) $(CPPSOURCESEventPackage:.cpp=.depend)
DEPENDS += $(DEPENDSEventPackage)
-include Libs/EventPackage/*depend 
endif

PREREQUIRED_SOURCES += Libs/EventPackage/generate.flag

Libs/EventPackage/generate.flag: Libs/EventPackage/EventPackage.xml $(XML2SOURCE_BIN)
	@echo Building $@
	$(XML2SOURCE_BIN) Libs/EventPackage/EventPackage.xml Libs/EventPackage
	touch $@

Libs/EventPackage/%.$(RMX_PLATFORM).o: Libs/EventPackage/%.cpp Libs/EventPackage/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHEventPackage_RMX) -o $@ $<

Libs/EventPackage/%.$(SOFT_PLATFORM).o: Libs/EventPackage/%.cpp Libs/EventPackage/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHEventPackage_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/EventPackage/%.depend: Libs/EventPackage/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHEventPackage_SOFT) $<
endif

$(TARGETEventPackage_RMX):  $(OBJECTSEventPackage_RMX) $(DEPENDSEventPackage)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSEventPackage_RMX) $(STDLIB_RMX) $(XML2LIB_RMX)  -lpthread -shared

$(TARGETEventPackage_SOFT):  $(OBJECTSEventPackage_SOFT) $(DEPENDSEventPackage)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSEventPackage_SOFT) $(STDLIB_SOFT) $(XML2LIB_SOFT) -lpthread -shared
