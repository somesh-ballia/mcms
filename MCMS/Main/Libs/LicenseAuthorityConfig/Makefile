INCPATHLicenseAuthorityConfig  = -I Libs/LicenseAuthorityConfig $(COMMONINCPATH)

INCPATHLicenseAuthorityConfig_RMX  = $(INCPATHLicenseAuthorityConfig)  $(XML2INC_RMX)
INCPATHLicenseAuthorityConfig_SOFT  = $(INCPATHLicenseAuthorityConfig) $(XML2INC_SOFT)

TARGETLicenseAuthorityConfig_RMX  = $(BIN_RMX)/libLicenseAuthorityConfig.so
TARGETLicenseAuthorityConfig_SOFT = $(BIN_SOFT)/libLicenseAuthorityConfig.so

TARGETS_RMX += $(TARGETLicenseAuthorityConfig_RMX)
TARGETS_SOFT += $(TARGETLicenseAuthorityConfig_SOFT)

CPPSOURCESLicenseAuthorityConfig := $(shell find Libs/LicenseAuthorityConfig |  grep "\.cpp$$")
SOURCESLicenseAuthorityConfig :=  $(CPPSOURCESLicenseAuthorityConfig)
OBJECTSLicenseAuthorityConfig_RMX := $(CSOURCESLicenseAuthorityConfig:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESLicenseAuthorityConfig:.cpp=.$(RMX_PLATFORM).o)
OBJECTSLicenseAuthorityConfig_SOFT := $(CSOURCESLicenseAuthorityConfig:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESLicenseAuthorityConfig:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSLicenseAuthorityConfig := $(CSOURCESLicenseAuthorityConfig:.c=.depend) $(CPPSOURCESLicenseAuthorityConfig:.cpp=.depend)
DEPENDS += $(DEPENDSLicenseAuthorityConfig)
-include Libs/LicenseAuthorityConfig/*depend 
endif

PREREQUIRED_SOURCES += Libs/LicenseAuthorityConfig/generate.flag

Libs/LicenseAuthorityConfig/generate.flag: Libs/LicenseAuthorityConfig/LicenseAuthorityConfigApi.xml $(XML2SOURCE_BIN)
	@echo Building $@
	$(XML2SOURCE_BIN) Libs/LicenseAuthorityConfig/LicenseAuthorityConfigApi.xml Libs/LicenseAuthorityConfig
	touch $@

Libs/LicenseAuthorityConfig/%.$(RMX_PLATFORM).o: Libs/LicenseAuthorityConfig/%.cpp Libs/LicenseAuthorityConfig/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHLicenseAuthorityConfig_RMX) -o $@ $<

Libs/LicenseAuthorityConfig/%.$(SOFT_PLATFORM).o: Libs/LicenseAuthorityConfig/%.cpp Libs/LicenseAuthorityConfig/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHLicenseAuthorityConfig_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/LicenseAuthorityConfig/%.depend: Libs/LicenseAuthorityConfig/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHLicenseAuthorityConfig_SOFT) $<
endif

$(TARGETLicenseAuthorityConfig_RMX):  $(OBJECTSLicenseAuthorityConfig_RMX) $(DEPENDSLicenseAuthorityConfig)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSLicenseAuthorityConfig_RMX) $(STDLIB_RMX) $(XML2LIB_RMX)  -lpthread -shared

$(TARGETLicenseAuthorityConfig_SOFT):  $(OBJECTSLicenseAuthorityConfig_SOFT) $(DEPENDSLicenseAuthorityConfig)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSLicenseAuthorityConfig_SOFT) $(STDLIB_SOFT) $(XML2LIB_SOFT) -lpthread -shared
