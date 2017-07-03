INCPATHMocks  = \
	-I Libs/Mocks \
	-I Libs/XmlPars \
	-I Libs/ProcessBase \
	-I Libs/Common \
	-I IncludeInternalMcms \
	-I ../McmIncld/CS/CsSignaling \
	-I ../McmIncld/Common \
	-I ../McmIncld/MPL/Card/CardCommon \
	-I ../McmIncld/MPL/Card/CardMngrIpMedia/ \
	-I ../McmIncld/MPL/Card/PhysicalPortART/RTP \
	-I ../McmIncld/MPL/Card/CardMngrTB \
	-I ../McmIncld/MPL/Card/CardMngrICE \
	-I ../McmIncld/MPL/Card/PhisycalPortNetISDN \
	-I ../McmIncld/MPL/Card/PhysicalPortVideo \
	-I ../McmIncld/MPL/Card/CardMngrTIP

INCPATHMocks_RMX = $(INCPATHMocks) 	$(CPPUNITINC_RMX)
INCPATHMocks_SOFT = $(INCPATHMocks) $(CPPUNITINC_SOFT)

TARGETMocks_RMX   = $(BIN_RMX)/libMocks.so
TARGETMocks_SOFT  = $(BIN_SOFT)/libMocks.so

SOURCESMocks := $(shell find Libs/Mocks | grep -v /Tests/ | grep "\.cpp$$")
OBJECTSMocks_RMX := $(SOURCESMocks:.cpp=.$(RMX_PLATFORM).o)
OBJECTSMocks_SOFT := $(SOURCESMocks:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSMocks := $(SOURCESMocks:.cpp=.depend)
endif

TARGETS_RMX  += $(TARGETMocks_RMX)
TARGETS_SOFT += $(TARGETMocks_SOFT)

DEPENDS += $(DEPENDSMocks)


ifndef SKIP_DEPEND
-include Libs/Mocks/*depend
endif

Libs/Mocks/%.$(RMX_PLATFORM).o: Libs/Mocks/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHMocks_RMX) -o $@ $<

Libs/Mocks/%.$(SOFT_PLATFORM).o: Libs/Mocks/%.cpp 
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHMocks_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/Mocks/%.depend: Libs/Mocks/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHMocks_SOFT) $<
endif

$(TARGETMocks_RMX):  $(OBJECTSMocks_RMX) $(DEPENDSMocks)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSMocks_RMX) -lpthread -shared


$(TARGETMocks_SOFT):  $(OBJECTSMocks_SOFT) $(DEPENDSMocks)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSMocks_SOFT) -lpthread -shared
