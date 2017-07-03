INCPATHIceProcesssor = \
	-I Lib/IceProcessor \
	-I Lib/Common \
	-I IncludeInternalMcms \
	-I ../McmIncld/MPL/Card/CardMngrICE \
	-I ../McmIncld/Common

TARGETIceProcesssor_RMX   = Bin.$(RMX_PLATFORM)/libIceProcessor.so
TARGETIceProcesssor_SOFT   = Bin.$(SOFT_PLATFORM)/libIceProcessor.so

TARGETS_RMX += $(TARGETIceProcesssor_RMX)
TARGETS_SOFT += $(TARGETIceProcesssor_SOFT)

SOURCESIceProcesssor := $(shell find Libs/IceProcessor |  grep -v /Tests/ | grep "\.cpp$$")
OBJECTSIceProcesssor_RMX := $(SOURCESIceProcesssor:.cpp=.$(RMX_PLATFORM).o)
OBJECTSIceProcesssor_SOFT := $(SOURCESIceProcesssor:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSIceProcesssor := $(SOURCESIceProcesssor:.cpp=.depend)

DEPENDS += $(DEPENDSIceProcesssor)

-include Libs/IceProcessor/*depend
endif

Libs/IceProcessor/%.$(RMX_PLATFORM).o: Libs/IceProcessor/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHIceProcesssor) -o $@ $<

Libs/IceProcessor/%.$(SOFT_PLATFORM).o: Libs/IceProcessor/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHIceProcesssor) -o $@ $<

ifndef SKIP_DEPEND
Libs/IceProcessor/%.depend: Libs/IceProcessor/%.cpp
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHIceProcesssor) $<
endif

$(TARGETIceProcesssor_RMX):  $(OBJECTSIceProcesssor_RMX) $(DEPENDSIceProcesssor)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSIceProcesssor_RMX) -shared

$(TARGETIceProcesssor_SOFT):  $(OBJECTSIceProcesssor_SOFT) $(DEPENDSIceProcesssor)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSIceProcesssor_SOFT) -shared
