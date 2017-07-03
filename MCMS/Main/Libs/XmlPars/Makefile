INCPATHXmlPars  = \
	-I Libs/XmlPars \
	-I Libs/Common \
	-I Libs/ProcessBase \
	-I IncludeInternalMcms \
	-I ../McmIncld/Common

INCPATHXmlPars_RMX = $(INCPATHXmlPars) 	$(ZLIBINC_RMX)
INCPATHXmlPars_SOFT = $(INCPATHXmlPars) $(ZLIBINC_SOFT)

TARGETXmlPars_RMX   = $(BIN_RMX)/libXmlPars.so
TARGETXmlPars_SOFT   = $(BIN_SOFT)/libXmlPars.so

SOURCESXmlPars := $(shell find Libs/XmlPars | grep -v /Tests/ | grep "\.cpp$$")
OBJECTSXmlPars_RMX := $(SOURCESXmlPars:.cpp=.$(RMX_PLATFORM).o)
OBJECTSXmlPars_SOFT := $(SOURCESXmlPars:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSXmlPars := $(SOURCESXmlPars:.cpp=.depend)
endif

TARGETS_RMX += $(TARGETXmlPars_RMX)
TARGETS_SOFT += $(TARGETXmlPars_SOFT)

ifndef SKIP_DEPEND
DEPENDS += $(DEPENDSXmlPars)
-include Libs/XmlPars/*depend
endif

#ifneq ($(MAKECMDGOALS) , clean)
#-include $(DEPENDSXmlPars)
#endif

Libs/XmlPars/%.$(RMX_PLATFORM).o: Libs/XmlPars/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHXmlPars_RMX) -o $@ $<

Libs/XmlPars/%.$(SOFT_PLATFORM).o: Libs/XmlPars/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHXmlPars_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/XmlPars/%.depend: Libs/XmlPars/%.cpp
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHXmlPars_SOFT) $<
endif

$(TARGETXmlPars_RMX):  $(OBJECTSXmlPars_RMX) $(DEPENDSXmlPars)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSXmlPars_RMX) -lpthread -shared

$(TARGETXmlPars_SOFT):  $(OBJECTSXmlPars_SOFT) $(DEPENDSXmlPars)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSXmlPars_SOFT) -lpthread -shared
