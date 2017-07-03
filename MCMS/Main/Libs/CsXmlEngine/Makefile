INCPATHCsXmlEngine = \
	-I Libs/CsXmlEngine \
	-I Libs/XmlPars \
	-I Libs/ProcessBase \
	-I IncludeInternalMcms \
	-I ../McmIncd/CS/CsMaintenance \
	-I ../McmIncld/Common \
	-I ../McmIncld/CS/CsSignaling \
	-I ../McmIncld/CS/CsMaintenance\
	-I ../McmIncld/MPL/Card/CardMngrICE \
	-I ../McmIncld/MPL/Card/PhysicalPortVideo

INCPATHCsXmlEngine_RMX  = $(INCPATHCsXmlEngine) $(ZLIBINC_RMX)
INCPATHCsXmlEngine_SOFT  = $(INCPATHCsXmlEngine) $(ZLIBINC_SOFT)

TARGETCsXmlEngine_RMX   = Libs/CsXmlEngine/CsXmlLib.$(RMX_PLATFORM).a
TARGETCsXmlEngine_SOFT  = Libs/CsXmlEngine/CsXmlLib.$(SOFT_PLATFORM).a

SOURCESCsXmlEngine := $(shell find Libs/CsXmlEngine |  grep -v /Tests/ | grep "\.cpp$$")
OBJECTSCsXmlEngine_RMX := $(SOURCESCsXmlEngine:.cpp=.$(RMX_PLATFORM).o)
OBJECTSCsXmlEngine_SOFT := $(SOURCESCsXmlEngine:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSCsXmlEngine := $(SOURCESCsXmlEngine:.cpp=.depend)
endif

TARGETS_RMX += $(TARGETCsXmlEngine_RMX) 
TARGETS_SOFT += $(TARGETCsXmlEngine_SOFT)

ifndef SKIP_DEPEND
DEPENDS += $(DEPENDSCsXmlEngine)

-include Libs/CsXmlEngine/*depend
ifndef SKIP_TESTS
-include Libs/CsXmlEngine/Tests/*depend
endif
endif

Libs/CsXmlEngine/%.$(RMX_PLATFORM).o: Libs/CsXmlEngine/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) -O0  $(INCPATHCsXmlEngine_RMX) -o $@ $<

Libs/CsXmlEngine/%.$(SOFT_PLATFORM).o: Libs/CsXmlEngine/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) -O0  $(INCPATHCsXmlEngine_SOFT) -o $@ $<
	
# <<<< Added for suppressing the -Wformat warnings in XmlBuilder.h	
Libs/CsXmlEngine/XmlBuilder.$(RMX_PLATFORM).o: Libs/CsXmlEngine/XmlBuilder.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) -O0 -Wno-format  $(INCPATHCsXmlEngine_RMX) -o $@ $<

Libs/CsXmlEngine/XmlBuilder.$(SOFT_PLATFORM).o: Libs/CsXmlEngine/XmlBuilder.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) -O0 -Wno-format $(INCPATHCsXmlEngine_SOFT) -o $@ $<
# >>>> Added for suppressing the -Wformat warnings in XmlBuilder.h

ifndef SKIP_DEPEND
Libs/CsXmlEngine/%.depend: Libs/CsXmlEngine/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHCsXmlEngine_SOFT) $<
endif

$(TARGETCsXmlEngine_RMX):  $(OBJECTSCsXmlEngine_RMX) $(DEPENDSCsXmlEngine)
	@echo Building library $@
	$(Q)$(AR_RMX) $@ $(OBJECTSCsXmlEngine_RMX)

$(TARGETCsXmlEngine_SOFT):  $(OBJECTSCsXmlEngine_SOFT) $(DEPENDSCsXmlEngine)
	@echo Building library $@
	$(Q)$(AR_SOFT) $@ $(OBJECTSCsXmlEngine_SOFT)
